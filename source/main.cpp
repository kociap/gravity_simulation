#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/math/transform.hpp>
#include <anton/string.hpp>
#include <build.hpp>
#include <entity.hpp>
#include <input.hpp>
#include <mesh.hpp>
#include <physics.hpp>
#include <point_mass.hpp>
#include <rendering.hpp>
#include <shader.hpp>
#include <transform.hpp>
#include <world.hpp>

#include <mimas/mimas.h>
#include <mimas/mimas_gl.h>

#include <glad/glad.h>

static String read_file(String const& path) {
    fs::Input_File_Stream stream(path);
    ANTON_FAIL(stream, "could not open file for reading");
    stream.seek(Seek_Dir::end, 0);
    i64 size = stream.tell();
    stream.seek(Seek_Dir::beg, 0);
    String result{reserve, size};
    result.force_size(size);
    stream.read(result.data(), size);
    return result;
}

static anton::Array<math::Vec3> generate_circle(math::Vec3 const& origin, math::Vec3 const& normal, f32 const radius, i32 const vert_count) {
    f32 const angle = math::two_pi / static_cast<f32>(vert_count);
    math::Quat const rotation_quat = math::Quat::from_axis_angle(normal, angle);
    // Find a point in the plane n = normal, d = 0
    math::Vec3 vertex = math::perpendicular(normal);
    // Rescale the circle
    vertex *= radius;
    // Generate a circle in the plane n = normal, d = 0 and center it at origin
    math::Quat rotated_vec{vertex.x, vertex.y, vertex.z, 0.0f};
    anton::Array<math::Vec3> circle_points{anton::reserve, vert_count};
    for(i64 i = 0; i <= vert_count; ++i) {
        rotated_vec = rotation_quat * rotated_vec * conjugate(rotation_quat);
        circle_points.emplace_back(rotated_vec.x + origin.x, rotated_vec.y + origin.y, rotated_vec.z + origin.z);
    }
    return circle_points;
}

static Array<math::Vec3> generate_filled_circle(i32 const vertex_count) {
    math::Vec3 const origin{0.0f, 0.0f, 0.0f};
    Array<math::Vec3> circle = generate_circle(origin, math::Vec3{0.0f, 0.0f, -1.0f}, 1.0f, vertex_count);
    Array<math::Vec3> result{reserve, 3 * vertex_count};
    for(i64 i = 0; i < vertex_count; ++i) {
        math::Vec3 const& v2 = circle[i];
        math::Vec3 const& v3 = circle[(i + 1) % vertex_count];
        result.emplace_back(origin);
        result.emplace_back(v3);
        result.emplace_back(v2);
    }
    return result;
}

struct Application_Context {
    f32 zoom = 64.0f;
    f32 simulation_speed = 1.0f;
    f32 object_scale = 1.0f;
    bool single_step = true;
    bool debug_printing = false;

    Vec2 camera_position;
    Vec2 camera_position_prev;
    bool lmb_down = false;
    bool lmb_up_down_transitioned = false;
    i32 cursor_pos_x = 0;
    i32 cursor_pos_y = 0;
};

static void scroll_callback(Mimas_Window* window, f32 dx, f32 dy, void* user_data) {
    Application_Context& ctx = *(Application_Context*)user_data;
    if(dy < 0) {
        ctx.zoom *= 2.0f;
    } else if(dy > 0) {
        // Min zoom is 0.5
        if(ctx.zoom >= 1.0f) {
            ctx.zoom *= 0.5f;
        }
    }
}

static void mouse_button_callback(Mimas_Window* window, Mimas_Key button, Mimas_Mouse_Button_Action action, void* user_data) {
    Application_Context& ctx = *(Application_Context*)user_data;
    if(button == MIMAS_MOUSE_LEFT_BUTTON) {
        bool new_state = action == MIMAS_MOUSE_BUTTON_PRESS;
        ctx.lmb_up_down_transitioned = new_state != ctx.lmb_down;
        ctx.lmb_down = new_state;
    }

    if(ctx.lmb_down && ctx.lmb_up_down_transitioned) {
        i32 cursor_x, cursor_y;
        mimas_get_cursor_pos(&cursor_x, &cursor_y);
        i32 window_pos_x, window_pos_y;
        mimas_get_window_content_pos(window, &window_pos_x, &window_pos_y);
        ctx.cursor_pos_x = cursor_x - window_pos_x;
        ctx.cursor_pos_y = cursor_y - window_pos_y;
    }

    if(!ctx.lmb_down && ctx.lmb_up_down_transitioned) {
        ctx.camera_position_prev = ctx.camera_position;
    }
}

static Vec2 get_window_content_size(Mimas_Window* window) {
    i32 x, y;
    mimas_get_window_content_size(window, &x, &y);
    return Vec2(x, y);
}

static void cursor_pos_callback(Mimas_Window* window, mimas_i32 x, mimas_i32 y, void* user_data) {
    Application_Context& ctx = *(Application_Context*)user_data;
    if(ctx.lmb_down) {
        Vec2 const position_delta(x - ctx.cursor_pos_x, y - ctx.cursor_pos_y);
        Vec2 const vp_size = get_window_content_size(window);
        Vec2 scale{1.0f, 1.0f};
        // inverse the direction
        scale *= -1.0f;
        scale *= 2.0f / vp_size.y;
        scale *= ctx.zoom;
        ctx.camera_position = scale * position_delta + ctx.camera_position_prev;
    }
}

static void key_callback(Mimas_Window* window, Mimas_Key key, Mimas_Key_Action action, void* user_data) {
    add_key_event(key, action);
}

static void load_sim_data_from_file(World& world, String const& path) {
    String contents = read_file(path);

    auto parse_csv_file = [](String const& contents) -> Array<Point_Mass> {
        auto find_line_end = [](auto begin, auto end) {
            for(; begin != end && *begin != '\n'; ++begin) {}
            return begin;
        };

        auto read_float = [](auto& begin, auto end) {
            i64 pos = find_substring(String_View{begin, end}, ",");
            auto first = begin;
            // Skip spaces
            while(*first == ' ') {
                ++first;
            }

            auto last = begin;
            if(pos != npos) {
                last += pos;
            } else {
                last = end;
            }

            begin = last;
            if(begin != end) {
                ++begin;
            }

            return str_to_f32(String{first, last});
        };

        Array<Point_Mass> point_masses;
        auto begin = contents.bytes_begin();
        auto end = contents.bytes_end();
        while(begin != end) {
            auto line_end = find_line_end(begin, end);
            f32 const pos_x = read_float(begin, line_end);
            f32 const pos_y = read_float(begin, line_end);
            f32 const vel_x = read_float(begin, line_end);
            f32 const vel_y = read_float(begin, line_end);
            f32 const mass = read_float(begin, line_end);
            point_masses.emplace_back(Point_Mass{Vec2{pos_x, pos_y}, Vec2{vel_x, vel_y}, mass});

            begin = line_end;
            // begin points either to '\n' or end.
            // move to the next line if not equal to end
            if(begin != end) {
                ++begin;
            }
        }

        return point_masses;
    };

    for(Point_Mass const& point_mass: parse_csv_file(contents)) {
        Entity e = world.create();
        world.add_component(e, point_mass);
        world.add_component(e, Transform{});
    }
}

int main(int argc, char** argv) {
    String const executable_path{fs::normalize_path(argv[0])};
    String const executable_directory{fs::get_directory_name(executable_path)};

    Application_Context application_context;

    Mimas_Init_Options init_options;
    init_options.capture_input_when_application_is_out_of_focus = false;
    if(!mimas_init_with_gl(&init_options)) {
        return -1;
    }

    Mimas_Display* primary_display = mimas_get_primary_display();
    Mimas_Display_Settings display_settings = mimas_get_display_settings(primary_display);

    Mimas_Window_Create_Info window_info;
    window_info.title = (mimas_char8 const*)u8"Gravity Simulation";
    window_info.width = 1280;
    window_info.height = 720;
    window_info.initial_pos_x = (display_settings.width - window_info.width) / 2;
    window_info.initial_pos_y = (display_settings.height - window_info.height) / 2;
    window_info.decorated = true;
    Mimas_Window* window = mimas_create_window(window_info);
    if(!window) {
        mimas_terminate();
        return -1;
    }

    mimas_set_window_scroll_callback(window, scroll_callback, &application_context);
    mimas_set_window_mouse_button_callback(window, mouse_button_callback, &application_context);
    mimas_set_window_cursor_pos_callback(window, cursor_pos_callback, &application_context);
    mimas_set_window_key_callback(window, key_callback, &application_context);

    Mimas_GL_Context* context = mimas_create_gl_context(4, 5, MIMAS_GL_CORE_PROFILE);
    mimas_make_context_current(window, context);

    gladLoadGL();

    init_rendering();

    Handle<Shader> mesh_shader;
    {
        String source_vertex = read_file(executable_directory + "/mesh.vert");
        Handle<Shader_Stage> handle_vertex = compile_shader_source("mesh_vertex", Shader_Stage_Type::vertex, source_vertex);
        String source_fragment = read_file(executable_directory + "/mesh.frag");
        Handle<Shader_Stage> handle_fragment = compile_shader_source("mesh_fragment", Shader_Stage_Type::fragment, source_fragment);
        mesh_shader = create_shader("mesh", handle_vertex, handle_fragment);
    }

    Handle<Shader> isolines_shader;
    {
        String source_vertex = read_file(executable_directory + "/isolines.vert");
        Handle<Shader_Stage> handle_vertex = compile_shader_source("isolines_vertex", Shader_Stage_Type::vertex, source_vertex);
        String source_fragment = read_file(executable_directory + "/isolines.frag");
        Handle<Shader_Stage> handle_fragment = compile_shader_source("isolines_fragment", Shader_Stage_Type::fragment, source_fragment);
        isolines_shader = create_shader("isolines", handle_vertex, handle_fragment);
    }

    Handle<Mesh> circle_mesh;
    {
        Array<Vec3> circle = generate_filled_circle(128);
        Mesh mesh;
        for(Vec3 v: circle) {
            mesh.vertices.emplace_back(Vertex{v, Vec4{0.698f, 0.29f, 1.0f, 1.0f}});
        }
        circle_mesh = add_mesh(ANTON_MOV(mesh));
    }

    Handle<Mesh> square_mesh;
    {
        Mesh mesh;
        math::Vec3 const v1{1.0f, 1.0f, 0.9f};
        math::Vec3 const v2{1.0f, -1.0f, 0.9f};
        math::Vec3 const v3{-1.0f, 1.0f, 0.9f};
        math::Vec3 const v4{-1.0f, -1.0f, 0.9f};
        mesh.vertices.emplace_back(v1, Vec4{0.0f});
        mesh.vertices.emplace_back(v3, Vec4{0.0f});
        mesh.vertices.emplace_back(v2, Vec4{0.0f});
        mesh.vertices.emplace_back(v3, Vec4{0.0f});
        mesh.vertices.emplace_back(v4, Vec4{0.0f});
        mesh.vertices.emplace_back(v2, Vec4{0.0f});
        square_mesh = add_mesh(ANTON_MOV(mesh));
    }

    World world;
    world.register_type<Point_Mass>();
    world.register_type<Mesh_Renderer>();
    world.register_type<Transform>();
    world.register_type<Isolines>();

    Entity const isolines_entity = world.create();
    world.add_component(isolines_entity, Isolines{square_mesh, isolines_shader});
    Isolines& isolines = world.get_component<Isolines>(isolines_entity);
    isolines.enabled = true;
    isolines.mode = Isolines::Render_Mode::contour_inverted;

    load_sim_data_from_file(world, executable_directory + "/sim.txt");
    for(Entity const e: world.entities<Point_Mass>()) {
        world.add_component(e, Mesh_Renderer{circle_mesh, mesh_shader});
    }

    Physics_World* physics_world = create_physics_world();

    mimas_show_window(window);

    Console_Output cout;
    f32 delta_time = 1.0f / 60.0f;
    f32 time = mimas_get_time();
    while(true) {
        {
            f32 const new_time = mimas_get_time();
            delta_time = new_time - time;
            time = new_time;
        }

        application_context.lmb_up_down_transitioned = false;

        update_input();
        mimas_poll_events();

        if(mimas_close_requested(window)) {
            break;
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_Q); key_released(key)) {
            if(application_context.simulation_speed >= 0.001f) {
                application_context.simulation_speed *= 0.5f;
            }
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_W); key_released(key)) {
            if(application_context.simulation_speed <= 1000.0f) {
                application_context.simulation_speed *= 2.0f;
            }
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_Z); key_released(key)) {
            if(application_context.object_scale > 1.0f) {
                application_context.object_scale *= 0.5f;
            }
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_X); key_released(key)) {
            if(application_context.object_scale <= 100000.0f) {
                application_context.object_scale *= 2.0f;
            }
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_R); key_released(key)) {
            application_context.single_step = !application_context.single_step;
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_D); key_released(key)) {
            application_context.debug_printing = !application_context.debug_printing;
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_T); key_released(key)) {
            isolines.enabled = !isolines.enabled;
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_1); key_released(key)) {
            isolines.mode = Isolines::Render_Mode::lines;
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_2); key_released(key)) {
            isolines.mode = Isolines::Render_Mode::contour;
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_3); key_released(key)) {
            isolines.mode = Isolines::Render_Mode::contour_inverted;
        }

        if(Key_State const key = get_key_state(MIMAS_KEY_4); key_released(key)) {
            isolines.mode = Isolines::Render_Mode::smooth;
        }

        if(application_context.single_step) {
            if(Key_State const key = get_key_state(MIMAS_KEY_S); key_released(key)) {
                run_physics(*physics_world, world, application_context.simulation_speed / 60.0f);
                if(application_context.debug_printing) {
                    for(Entity const entity: world.entities<Point_Mass>()) {
                        Point_Mass const& point_mass = world.get_component<Point_Mass>(entity);
                        cout.write(format(u8"{}: ({}, {}); ({}, {}); {}\n", entity.id, point_mass.position.x, point_mass.position.y, point_mass.velocity.x,
                                          point_mass.velocity.y, point_mass.mass));
                    }
                }
            }
        } else {
            run_physics(*physics_world, world, application_context.simulation_speed * delta_time);
            if(application_context.debug_printing) {
                for(Entity const entity: world.entities<Point_Mass>()) {
                    Point_Mass const& point_mass = world.get_component<Point_Mass>(entity);
                    cout.write(format(u8"{}: ({}, {}); ({}, {}); {}\n", entity.id, point_mass.position.x, point_mass.position.y, point_mass.velocity.x,
                                      point_mass.velocity.y, point_mass.mass));
                }
            }
        }

        for(Entity const entity: world.entities<Point_Mass>()) {
            Point_Mass& point_mass = world.get_component<Point_Mass>(entity);
            Transform& transform = world.get_component<Transform>(entity);
            transform.postion = Vec3{point_mass.position, 0.0f};
            f32 const scale_factor = application_context.object_scale * log2(point_mass.mass);
            transform.scale = Vec3{scale_factor};
        }

        i32 x, y;
        mimas_get_window_content_size(window, &x, &y);
        f32 const aspect_ratio = (f32)x / (f32)y;
        f32 const zoom = application_context.zoom;

        Mat4 const view = lookat_rh(Vec3{application_context.camera_position, 5.0f}, Vec3{application_context.camera_position, -1.0f}, Vec3{0.0f, 1.0f, 0.0f});
        Mat4 const proj = orthographic_rh(-aspect_ratio * zoom, aspect_ratio * zoom, -zoom, zoom, 0.0f, 10.0f);

        glViewport(0, 0, x, y);
        render(world, view, proj);
        mimas_swap_buffers(window);
    }

    destory_physics_world(physics_world);
    mimas_destroy_window(window);
    mimas_terminate();

    return 0;
}

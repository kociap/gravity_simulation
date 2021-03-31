#include <rendering.hpp>

#include <anton/console.hpp>
#include <anton/math/transform.hpp>
#include <anton/math/vec2.hpp>
#include <anton/string.hpp>
#include <mesh.hpp>
#include <point_mass.hpp>
#include <transform.hpp>

#include <glad/glad.h>

static void debug_callback(GLenum const source, GLenum const type, GLuint, GLenum const severity, GLsizei, GLchar const* const message, void const*) {
    auto stringify_source = [](GLenum const source) {
        switch(source) {
            case GL_DEBUG_SOURCE_API:
                return u8"API";
            case GL_DEBUG_SOURCE_APPLICATION:
                return u8"Application";
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                return u8"Shader Compiler";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                return u8"Window System";
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                return u8"Third Party";
            case GL_DEBUG_SOURCE_OTHER:
                return u8"Other";
            default:
                ANTON_UNREACHABLE();
        }
    };

    auto stringify_type = [](GLenum const type) {
        switch(type) {
            case GL_DEBUG_TYPE_ERROR:
                return u8"Error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                return u8"Deprecated Behavior";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                return u8"Undefined Behavior";
            case GL_DEBUG_TYPE_PORTABILITY:
                return u8"Portability";
            case GL_DEBUG_TYPE_PERFORMANCE:
                return u8"Performance";
            case GL_DEBUG_TYPE_MARKER:
                return u8"Marker";
            case GL_DEBUG_TYPE_PUSH_GROUP:
                return u8"Push Group";
            case GL_DEBUG_TYPE_POP_GROUP:
                return u8"Pop Group";
            case GL_DEBUG_TYPE_OTHER:
                return u8"Other";
            default:
                ANTON_UNREACHABLE();
        }
    };

    auto stringify_severity = [](GLenum const severity) {
        switch(severity) {
            case GL_DEBUG_SEVERITY_HIGH:
                return u8"Fatal Error";
            case GL_DEBUG_SEVERITY_MEDIUM:
                return u8"Error";
            case GL_DEBUG_SEVERITY_LOW:
                return u8"Warning";
            case GL_DEBUG_SEVERITY_NOTIFICATION:
                return u8"Note";
            default:
                ANTON_UNREACHABLE();
        }
    };

    String stringified_message = String(stringify_severity(severity)) + " " + stringify_source(source) + " (" + stringify_type(type) + "): " + message + "\n";
    Console_Output cout;
    cout.write(stringified_message);
    if(severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM) {
        ANTON_FAIL(false, stringified_message.data());
    }
}

static void install_debug_callback() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_callback, nullptr);
}

struct Buffer {
    u32 handle;
    void* mapped;
};

static Buffer vbo;
static Buffer point_mass_objects_buffer;
static u32 vao;

struct Point_Mass_Object {
    alignas(8) Vec2 position;
    alignas(8) f32 mass;
};

void init_rendering() {
    install_debug_callback();

    glDisable(GL_FRAMEBUFFER_SRGB);
    glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Vertex, position));
    glVertexAttribBinding(0, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Vertex, color));
    glVertexAttribBinding(1, 0);

    glCreateBuffers(1, &vbo.handle);
    // Allocate 0.5MB of memory for the vertices. More than we will ever need.
    glNamedBufferStorage(vbo.handle, 32768 * sizeof(Vertex), nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    vbo.mapped = glMapNamedBufferRange(vbo.handle, 0, 32768 * sizeof(Vertex), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBindVertexBuffer(0, vbo.handle, 0, sizeof(Vertex));

    glCreateBuffers(1, &point_mass_objects_buffer.handle);
    // Allocate space for 32768 mass objects.
    glNamedBufferStorage(point_mass_objects_buffer.handle, 32768 * sizeof(Point_Mass_Object), nullptr,
                         GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    point_mass_objects_buffer.mapped = glMapNamedBufferRange(point_mass_objects_buffer.handle, 0, 32768 * sizeof(Point_Mass_Object),
                                                             GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, point_mass_objects_buffer.handle, 0, 32768 * sizeof(Point_Mass_Object));
}

void render(World& world, Mat4 const& view, Mat4 const& proj) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Vertex* const vertex_buffer_begin = (Vertex*)vbo.mapped;
    Vertex* vertex_buffer = (Vertex*)vbo.mapped;
    Mat4 const vp = proj * view;
    for(Entity const entity: world.entities<Mesh_Renderer>()) {
        Mesh_Renderer& mesh_renderer = world.get_component<Mesh_Renderer>(entity);
        Transform& transform = world.get_component<Transform>(entity);
        Mat4 const model = math::translate(transform.postion) * math::rotate(transform.orientation) * math::scale(transform.scale);
        Mat4 const mvp = vp * model;
        bind_shader(mesh_renderer.shader);
        set_uniform_mat4(mesh_renderer.shader, String{"mvp"}, mvp);
        Mesh& mesh = get_mesh(mesh_renderer.mesh);
        copy(mesh.vertices.begin(), mesh.vertices.end(), vertex_buffer);
        glDrawArrays(GL_TRIANGLES, vertex_buffer - vertex_buffer_begin, mesh.vertices.size());
        vertex_buffer += mesh.vertices.size();
    }

    Slice<Entity> isolines = world.entities<Isolines>();
    ANTON_FAIL(isolines.size() <= 1, "too many isolines");
    if(isolines.size() == 1) {
        Entity const entity = isolines[0];
        Isolines& isolines = world.get_component<Isolines>(entity);
        if(isolines.enabled) {
            constexpr f32 gravitational_constant = 6.67408e-11f;
            Slice<Point_Mass> point_masses = world.components<Point_Mass>();
            Point_Mass_Object* point_mass_objects = (Point_Mass_Object*)point_mass_objects_buffer.mapped;
            f32 max_field_value = 0.0f;
            for(Point_Mass const& v: point_masses) {
                point_mass_objects->position = v.position;
                point_mass_objects->mass = v.mass;
                point_mass_objects += 1;
                // at distance 1.0 from the mass
                max_field_value = math::max(max_field_value, gravitational_constant * v.mass);
            }

            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, point_mass_objects_buffer.handle, 0, point_masses.size() * sizeof(Point_Mass_Object));

            Mesh& mesh = get_mesh(isolines.mesh);
            copy(mesh.vertices.begin(), mesh.vertices.end(), vertex_buffer);
            bind_shader(isolines.shader);
            set_uniform_mat4(isolines.shader, String{"vp"}, vp);
            set_uniform_f32(isolines.shader, String{"max_field"}, max_field_value);
            set_uniform_i32(isolines.shader, String{"render_mode"}, (i32)isolines.mode);
            glDrawArrays(GL_TRIANGLES, vertex_buffer - vertex_buffer_begin, mesh.vertices.size());
            vertex_buffer += mesh.vertices.size();
        }
    }
}

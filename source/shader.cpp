#include <shader.hpp>

#include <anton/algorithm.hpp>
#include <anton/array.hpp>
#include <anton/assert.hpp>
#include <anton/console.hpp>
#include <anton/string.hpp>

#include <glad/glad.h>

struct Shader_Stage_Resource {
    Handle<Shader_Stage> handle;
    u32 gl_handle = 0;
};

struct Shader_Resource {
    Handle<Shader> handle;
    u32 gl_handle;
};

static Array<Shader_Stage_Resource> shader_stage_resources;
static u64 shader_stage_handle_index_counter = 0;
static Array<Shader_Resource> shader_resources;
static u64 shader_handle_index_counter = 0;

Handle<Shader_Stage> compile_shader_source(String_View name, Shader_Stage_Type type, String_View source) {
    u32 gl_handle = 0;
    switch(type) {
        case Shader_Stage_Type::vertex: {
            gl_handle = glCreateShader(GL_VERTEX_SHADER);
        } break;
        case Shader_Stage_Type::fragment: {
            gl_handle = glCreateShader(GL_FRAGMENT_SHADER);
        } break;
    }

    char const* src = source.data();
    glShaderSource(gl_handle, 1, &src, nullptr);

    // compile shader stage
    glCompileShader(gl_handle);
    GLint compilation_status;
    glGetShaderiv(gl_handle, GL_COMPILE_STATUS, &compilation_status);
    if(compilation_status == GL_FALSE) {
        GLint log_length;
        glGetShaderiv(gl_handle, GL_INFO_LOG_LENGTH, &log_length);
        i64 const total_size = log_length + name.size_bytes() + 35;
        String log{reserve, total_size};
        log.append(u8"Shader stage compilation failed (");
        log.append(name);
        log.append(")\n");
        glGetShaderInfoLog(gl_handle, log_length, &log_length, log.data() + log.size_bytes());
        log.force_size(total_size);
        Console_Output cout;
        cout.write(log);
        ANTON_FAIL(false, log.data());
    }

    Handle<Shader_Stage> handle{shader_stage_handle_index_counter++};
    shader_stage_resources.emplace_back(handle, gl_handle);
    return handle;
}

Handle<Shader> create_shader(String_View name, Handle<Shader_Stage> const& vertex, Handle<Shader_Stage> const& fragment) {
    u32 gl_handle = glCreateProgram();

    ANTON_FAIL(vertex, "invalid handle");
    auto r_vertex = find_if(shader_stage_resources.begin(), shader_stage_resources.end(),
                            [&vertex](Shader_Stage_Resource const& resource) { return resource.handle == vertex; });
    ANTON_FAIL(r_vertex != shader_stage_resources.end(), "handle to non-existent resource");
    glAttachShader(gl_handle, r_vertex->gl_handle);
    ANTON_FAIL(vertex, "invalid handle");

    auto r_fragment = find_if(shader_stage_resources.begin(), shader_stage_resources.end(),
                              [&fragment](Shader_Stage_Resource const& resource) { return resource.handle == fragment; });
    ANTON_FAIL(r_fragment != shader_stage_resources.end(), "handle to non-existent resource");
    glAttachShader(gl_handle, r_fragment->gl_handle);

    glLinkProgram(gl_handle);
    GLint link_status;
    glGetProgramiv(gl_handle, GL_LINK_STATUS, &link_status);
    if(link_status == GL_FALSE) {
        GLint log_length;
        glGetProgramiv(gl_handle, GL_INFO_LOG_LENGTH, &log_length);
        i64 const total_size = log_length + name.size_bytes() + 29;
        String log{reserve, total_size};
        log.append(u8"Shader compilation failed (");
        log.append(name);
        log.append(")\n");
        glGetProgramInfoLog(gl_handle, log_length, &log_length, log.data() + log.size_bytes());
        log.force_size(total_size);
        Console_Output cout;
        cout.write(log);
        ANTON_FAIL(false, log.data());
    }

    Handle<Shader> handle{shader_handle_index_counter++};
    shader_resources.emplace_back(handle, gl_handle);
    return handle;
}

void bind_shader(Handle<Shader> const& handle) {
    ANTON_FAIL(handle, "invalid handle");
    auto r = find_if(shader_resources.begin(), shader_resources.end(), [&handle](Shader_Resource const& resource) { return resource.handle == handle; });
    ANTON_FAIL(r != shader_resources.end(), "handle to non-existent resource");
    glUseProgram(r->gl_handle);
}

void set_uniform_i32(Handle<Shader> const& handle, String const& name, i32 v) {
    ANTON_FAIL(handle, "invalid handle");
    auto r = find_if(shader_resources.begin(), shader_resources.end(), [&handle](Shader_Resource const& resource) { return resource.handle == handle; });
    ANTON_FAIL(r != shader_resources.end(), "handle to non-existent resource");
    i32 const location = glGetUniformLocation(r->gl_handle, name.data());
    if(location != -1) {
        glProgramUniform1i(r->gl_handle, location, v);
    }
}

void set_uniform_f32(Handle<Shader> const& handle, String const& name, f32 v) {
    ANTON_FAIL(handle, "invalid handle");
    auto r = find_if(shader_resources.begin(), shader_resources.end(), [&handle](Shader_Resource const& resource) { return resource.handle == handle; });
    ANTON_FAIL(r != shader_resources.end(), "handle to non-existent resource");
    i32 const location = glGetUniformLocation(r->gl_handle, name.data());
    if(location != -1) {
        glProgramUniform1f(r->gl_handle, location, v);
    }
}

void set_uniform_mat4(Handle<Shader> const& handle, String const& name, Mat4 const& v) {
    ANTON_FAIL(handle, "invalid handle");
    auto r = find_if(shader_resources.begin(), shader_resources.end(), [&handle](Shader_Resource const& resource) { return resource.handle == handle; });
    ANTON_FAIL(r != shader_resources.end(), "handle to non-existent resource");
    i32 const location = glGetUniformLocation(r->gl_handle, name.data());
    if(location != -1) {
        glProgramUniformMatrix4fv(r->gl_handle, location, 1, GL_FALSE, v.data());
    }
}
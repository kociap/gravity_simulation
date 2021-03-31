#pragma once

#include <anton/math/mat4.hpp>
#include <anton/string_view.hpp>
#include <build.hpp>
#include <handle.hpp>

enum struct Shader_Stage_Type { vertex, fragment };

struct Shader_Stage;

Handle<Shader_Stage> compile_shader_source(String_View name, Shader_Stage_Type type, String_View source);

struct Shader;

Handle<Shader> create_shader(String_View name, Handle<Shader_Stage> const& vertex, Handle<Shader_Stage> const& fragment);

// bind_shader
// Binds shader for use during the following draw operations.
//
void bind_shader(Handle<Shader> const& handle);

void set_uniform_i32(Handle<Shader> const& handle, String const& name, i32 v);
void set_uniform_f32(Handle<Shader> const& handle, String const& name, f32 v);
void set_uniform_mat4(Handle<Shader> const& handle, String const& name, Mat4 const& v);

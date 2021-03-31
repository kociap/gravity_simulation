#pragma once

#include <anton/array.hpp>
#include <anton/math/vec3.hpp>
#include <anton/math/vec4.hpp>
#include <build.hpp>
#include <handle.hpp>
#include <shader.hpp>

struct Vertex {
    alignas(16) Vec3 position;
    alignas(16) Vec4 color;
};

struct Mesh {
    Array<Vertex> vertices;
};

struct Mesh_Renderer {
    Handle<Mesh> mesh;
    Handle<Shader> shader;
};

struct Isolines {
    enum struct Render_Mode {
        lines = 1,
        contour,
        contour_inverted,
        smooth,
    };

    Handle<Mesh> mesh;
    Handle<Shader> shader;
    Render_Mode mode = Render_Mode::lines;
    bool enabled = false;
};

Handle<Mesh> add_mesh(Mesh&& mesh);

// get_mesh
//
// Parameters:
// handle - handle to the mesh. Must be a valid handle returned by add_mesh.
//
Mesh& get_mesh(Handle<Mesh> const& handle);

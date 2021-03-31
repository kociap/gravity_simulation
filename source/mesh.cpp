#include <mesh.hpp>

#include <anton/algorithm.hpp>
#include <anton/assert.hpp>

struct Mesh_Resource {
    Handle<Mesh> handle;
    Mesh mesh;
};

static Array<Mesh_Resource> resources;
static u64 handle_index_counter = 0;

Handle<Mesh> add_mesh(Mesh&& mesh) {
    Handle<Mesh> handle{handle_index_counter++};
    resources.emplace_back(handle, ANTON_MOV(mesh));
    return handle;
}

Mesh& get_mesh(Handle<Mesh> const& handle) {
    ANTON_FAIL(handle, "invalid handle");
    auto r = find_if(resources.begin(), resources.end(), [&handle](Mesh_Resource const& resource) { return resource.handle == handle; });
    ANTON_FAIL(r != resources.end(), "handle to non-existent resource");
    return r->mesh;
}

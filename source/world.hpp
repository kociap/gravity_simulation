#pragma once

#include <anton/array.hpp>
#include <anton/intrinsics.hpp>
#include <anton/slice.hpp>
#include <anton/typeid.hpp>
#include <build.hpp>
#include <entity.hpp>

struct World {
private:
    struct Container_Base {
    public:
        Container_Base(u64 id): id(id) {}

        [[nodiscard]] u64 get_id() const {
            return id;
        }

    private:
        u64 id;
    };

    template<typename T>
    struct Container: Container_Base {
    public:
        Container(): Container_Base(type_identifier<T>()) {}

        Slice<Entity> get_entities() {
            return entities;
        }

        Slice<T> get_components() {
            return components;
        }

        void add(Entity const entity, T const& component) {
            if(entity_index.size() < (i64)entity.id + 1) {
                entity_index.resize(entity.id + 1, -1);
            }
            entity_index[entity.id] = components.size();
            entities.emplace_back(entity);
            components.emplace_back(component);
        }

        T& get(Entity const entity) {
            ANTON_FAIL((i64)entity.id < entity_index.size() && entity_index[entity.id] != -1, "component doesn't exist");
            i64 const index = entity_index[entity.id];
            return components[index];
        }

    private:
        Array<T> components;
        Array<Entity> entities;
        Array<i64> entity_index;
    };

    template<typename T>
    Container<T>* get_container() {
        u64 const id = type_identifier<T>();
        for(Container_Base* container: containers) {
            if(container->get_id() == id) {
                return (Container<T>*)container;
            }
        }

        // Containers must be created a priori.
        ANTON_FAIL(false, "container for type doesn't exist");
    }

    Array<Container_Base*> containers;
    u64 entity_index_counter = 0;

public:
    template<typename T>
    void register_type() {
        // No duplicate checking cause yolo
        Container<T>* container = new Container<T>();
        containers.emplace_back(container);
    }

    template<typename T>
    [[nodiscard]] Slice<Entity> entities() {
        Container<T>* container = get_container<T>();
        return container->get_entities();
    }

    template<typename T>
    [[nodiscard]] Slice<T> components() {
        Container<T>* container = get_container<T>();
        return container->get_components();
    }

    [[nodiscard]] Entity create() {
        return {entity_index_counter++};
    }

    template<typename T>
    void add_component(Entity const entity, T const& component) {
        Container<T>* container = get_container<T>();
        container->add(entity, component);
    }

    template<typename T>
    T& get_component(Entity const entity) {
        Container<T>* container = get_container<T>();
        return container->get(entity);
    }
};

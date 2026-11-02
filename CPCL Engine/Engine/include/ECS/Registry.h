#pragma once
#include "ECS/Entity.h"
#include "ECS/ComponentPool.h"
#include <memory>
#include <vector>

class Registry {
    Entity _next = 0;
    std::vector<std::unique_ptr<IPool>> _pools;   

    static size_t nextTypeId() { static size_t id = 0; return id++; }
    template <typename T> static size_t typeId() { static size_t id = nextTypeId(); return id; }

    template <typename T>
    ComponentPool<T>& pool() {
        size_t id = typeId<T>();
        if (id >= _pools.size()) _pools.resize(id + 1);
        if (!_pools[id]) _pools[id] = std::make_unique<ComponentPool<T>>();
        return static_cast<ComponentPool<T>&>(*_pools[id]);
    }

public:
    Entity create() { return _next++; }          
    template <typename T, typename... Args>
    T& add(Entity e, Args&&... args) { return pool<T>().add(e, T{ std::forward<Args>(args)... }); }

    template <typename T> bool has(Entity e) { return pool<T>().has(e); }
    template <typename T> T& get(Entity e) { return pool<T>().get(e); }

    // Appelle fn(entity, A&, B&...) pour chaque entité qui a TOUS les composants
    template <typename A, typename... Rest, typename Fn>
    void each(Fn&& fn) {
        auto& pa = pool<A>();
        for (Entity e : pa.owners()) {             // on itère sur le pool de A
            if ((pool<Rest>().has(e) && ...))      // et on vérifie les autres
                fn(e, pa.get(e), pool<Rest>().get(e)...);
        }
    }
};
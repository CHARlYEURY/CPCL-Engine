// ============================================================
//  Registry.h
// ------------------------------------------------------------
//  le Registry est le "centre" de l'ECS. il fait 3 choses :
//     - create()     : donne un nouvel id d'entité
//     - add/has/get  : attache / teste / recupere un composant d'une entité
//     - each<...>()  : parcourt toutes les entités qui ont un ensemble de composants
//
//     entité   = un id (Entity.h)
//     composant = de la donnée pure, sans logique (Transform, MeshRenderer, Spin)
//     système  = une fonction qui parcourt les entités avec each<> et agit sur leurs
//                composants (SpinSystem, RenderSystem)
//
//  le Registry range chaque type de composant dans son propre ComponentPool.
//  utilisé par : main (creation des entités) et les systèmes (each)
// ============================================================
#pragma once
#include "ECS/Entity.h"
#include "ECS/ComponentPool.h"
#include <memory>
#include <vector>

class Registry {
    Entity _next = 0;                              // prochain id d'entité a donner (on incremente, jamais recyclé)
    std::vector<std::unique_ptr<IPool>> _pools;    // un pool par type de composant, rangé selon l'id du type (voir typeId)

    // ------------------------------------------------------------
    //  typeId<T>() : donne un numero UNIQUE a chaque type de composant (0, 1, 2...) sert d'index dans _pools
    //  la variable "id" dans typeId<T> est static, donc initialisée UNE seule fois
    //  par type T, la premiere fois qu'on appelle la fonction. a ce moment on lui donne le
    //  prochain numero libre grace a nextTypeId() (compteur static partagé).
    //  ex : le 1er type demandé (Transform) reçoit 0, le 2eme (MeshRenderer) reçoit 1, etc.
    // ------------------------------------------------------------
    static size_t nextTypeId() { static size_t id = 0; return id++; }
    template <typename T> static size_t typeId() { static size_t id = nextTypeId(); return id; }

    // ------------------------------------------------------------
    //  pool<T>() : renvoie le ComponentPool<T>, et le crée s'il existe pas encore
    //  1) on recupere l'id du type T
    //  2) si le tableau _pools est trop petit on l'agrandit (nouvelles cases = nullptr)
    //  3) si la case est vide on cree le pool (make_unique)
    //  4) _pools stocke des IPool*, donc on caste vers ComponentPool<T> (static_cast sans danger
    //  car on sait que la case de l'id de T contient forcement un ComponentPool<T>)
    //  appeler has<T> ou each<T> sur un type jamais utilisé crée un pool vide
    // ------------------------------------------------------------
    template <typename T>
    ComponentPool<T>& pool() {
        size_t id = typeId<T>();
        if (id >= _pools.size()) _pools.resize(id + 1);
        if (!_pools[id]) _pools[id] = std::make_unique<ComponentPool<T>>();
        return static_cast<ComponentPool<T>&>(*_pools[id]);
    }

public:
    // cree une entité : juste un nouveau numero, elle n'a aucun composant au depart
    Entity create() { return _next++; }

    // ------------------------------------------------------------
    //  add<T>(e, args...) : attache un composant de type T a l'entité e
    //  les args sont transmis (forward) a la construction de T avec des accolades T{ ... },
    //  c'est pour ca que mes composants sont des structs simples (aggregate) :
    //     reg.add<Spin>(e)                                  -> valeurs par defaut
    //     reg.add<MeshRenderer>(e, &mesh, &mat, &normals)   -> remplit les champs dans l'ordre
    //  renvoie une reference sur le composant pour pouvoir le regler tout de suite
    // ------------------------------------------------------------
    template <typename T, typename... Args>
    T& add(Entity e, Args&&... args) { return pool<T>().add(e, T{ std::forward<Args>(args)... }); }

    // l'entité e a t'elle un composant de type T ?
    template <typename T> bool has(Entity e) { return pool<T>().has(e); }
    // recupere le composant T de l'entité e (verifier avec has avant si pas sur)
    template <typename T> T& get(Entity e) { return pool<T>().get(e); }

    // ------------------------------------------------------------
    //  each<A, Rest...>(fn) : appelle fn pour chaque entité qui possede TOUS les composants
    //  A, Rest1, Rest2... fn recoit l'entité puis une reference vers chacun des composants,
    //  dans l'ordre des parametres template.
    //  ex : each<Transform, Spin>([](Entity e, Transform& t, Spin& s) {...})
    //
    //  on parcourt uniquement les proprietaires du pool de A, et pour chacun
    //  on verifie qu'il a aussi tous les autres composants. la verification utilise
    //  une "fold expression" : (pool<Rest>().has(e) && ...) est depliée par le compilateur
    //  en  has<R1> && has<R2> && ...
    //  conseil : mettre en A le composant le plus RARE, comme ca on parcourt moins d'entités
    //  attention : ne pas ajouter / retirer de composants de type A dans fn, ca modifierait
    //  le tableau qu'on est en train de parcourir
    //  utilisé par : SpinSystem et RenderSystem
    // ------------------------------------------------------------
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
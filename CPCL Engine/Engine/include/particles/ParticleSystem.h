// ============================================================
//  ParticleSystem.h
// ------------------------------------------------------------
//  le systeme de particules gere TOUTES les particules vivantes :
//    - l'Emitter les fait naitre (position, vitesse, duree de vie, couleur...)
//    - update() les fait vivre : vieillissement, gravité, deplacement, mort
//    - le Renderer les dessine (Renderer::draw_particles)
//
//  une particule = une petite struct Particle (voir Emitter.h), pas un objet de l'ECS,
//  ca serait trop lourd pour des milliers de particules pour l'instant, je verrais pour une opti plus tard
//
//  appelé dans main a chaque frame : ps.update(dt), puis le ParticleSystem est
//  passé a RenderSystem qui le donne a Renderer::draw_particles.
// ============================================================
#pragma once
#include "particles/Emitter.h"     // Emitter + struct Particle

class ParticleSystem
{
private:
    size_t _capacity;      // nombre MAX de particules en meme temps (le pool ne depasse jamais ca)

public:
    // capacity = taille max du pool (10000 par defaut)
    explicit ParticleSystem(size_t capacity = 10000) : _capacity(capacity)
    {
        // reserve = on alloue la memoire UNE fois au depart, comme l'emitter ne depasse jamais
        // _capacity le vector n'a plus jamais besoin de se reallouer (pas de lag en pleine partie)
        particles.reserve(capacity);           // aucune allocation ensuite
    }

    Emitter emitter;                                          // le "robinet" a particules, public pour regler ses parametres depuis main
    Maths::Vec3_SSE gravity = Maths::Vec3_SSE(0, -3.0f, 0);   // acceleration appliquée a toutes les particules (vers le bas, en unités monde / s²)
    std::vector<Particle> particles;                          // le pool : toutes les particules vivantes, rangées a la suite

    // avance la simulation de dt secondes : emission des nouvelles, puis mise a jour / suppression des existantes
    void update(float dt);


};
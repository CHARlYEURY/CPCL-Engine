// ============================================================
//  Emitter.h
// ------------------------------------------------------------
//  ce fichier contient 2 choses :
//    - Particle : les données d'UNE particule (position, vitesse, age, couleur...)
//    - Emitter  : la "source" qui fait naitre les particules, avec tous ses reglages
//
//  l'emitter ne fait QUE creer des particules. leur vie (gravité, deplacement, mort)
//  est geré par ParticleSystem::update, et leur affichage par Renderer::draw_particles.
//
//  utilisé par : ParticleSystem (qui possede un Emitter et lui donne son pool de particules)
// ============================================================
#pragma once
#include <vector>
#include <cstdint>
#include "Vec3_SSE.h"

// ------------------------------------------------------------
//  Particle : une particule, juste des données
//  elles sont stockées a la suite dans ParticleSystem::particles
// ------------------------------------------------------------
struct Particle {
    Maths::Vec3_SSE pos, vel;           // position et vitesse en espace monde
    float age, lifetime, size;          // age = temps vécu (secondes), lifetime = durée de vie totale, size = taille en unités monde
    uint32_t colorStart, colorEnd;      // couleur a la naissance et a la mort, le rendu fait le degradé entre les deux (0xAARRGGBB)
};

// ------------------------------------------------------------
//  Emitter : tous les parametres de generation sont publics pour pouvoir
//  les regler facilement depuis main (ex : emitter.rate = 500; emitter.position = ...)
//  chaque valeur "Min / Max" = a chaque naissance on tire un nombre au hasard entre les deux
// ------------------------------------------------------------

class Emitter
{
public:
    Maths::Vec3_SSE position = Maths::Vec3_SSE(0, 0, 0);   // endroit ou les particules naissent
    float rate = 300.0f;                       // particules / seconde
    float speedMin = 2.0f, speedMax = 3.5f;    // vitesse de depart (unités monde / seconde)
    float lifeMin = 1.0f, lifeMax = 2.0f;      // durée de vie en secondes
    float sizeMin = 0.03f, sizeMax = 0.08f;    // taille en unités monde
    float spread = 0.4f;                       // 0 = jet droit, 1 = très ouvert
    uint32_t colorStart = 0xFFFFC040;          // 0xAARRGGBB  (orange clair, comme une flamme qui vient de naitre)
    uint32_t colorEnd = 0xFFFF2000;            // rouge fonçé, couleur en fin de vie

    // fait naitre les particules qui doivent apparaitre pendant dt secondes et les ajoute dans "pool"
    // pool = le tableau de ParticleSystem, capacity = nombre max de particules autorisé
    // appelée par ParticleSystem::update a chaque frame
    void emit(std::vector<Particle>& pool, size_t capacity, float dt);

private:
    float _accum = 0.0f;                       // "reste" de particules pas encore emises (partie decimale), garde entre deux frames
    uint32_t _rng = 12345u;                    // etat du generateur aleatoire (graine de depart, ne doit jamais etre 0)
    float rnd();                               // [0,1[
    // tire un nombre au hasard entre a et b (a + (b-a) * un nombre entre 0 et 1)
    float range(float a, float b) { return a + (b - a) * rnd(); }
};
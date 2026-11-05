// ============================================================
//  ParticleSystem.cpp
// ------------------------------------------------------------
//  une seule fonction : update, qui fait avancer toute la simulation d'une frame.
// ============================================================
#include "particles/ParticleSystem.h"

// ------------------------------------------------------------
//  update : simulation des particules pour dt secondes
//  etapes :
//    1) l'emitter ajoute les nouvelles particules dans le pool (selon son debit "rate")
//    2) pour chaque particule : on la vieillit, si elle est trop vieille on la supprime,
//       sinon on applique la gravité et on la deplace
//  dt = temps ecoulé depuis la derniere frame, en secondes (comme ca la simulation va a la meme vitesse
//  quelque soit le nombre de fps)
//  appelée une fois par frame depuis main, avant le rendu
// ------------------------------------------------------------
void ParticleSystem::update(float dt)
{
    // 1) emission : emit regarde combien de particules doivent naitre pendant dt
    // et les ajoute dans "particles" sans jamais depasser _capacity
    emitter.emit(particles, _capacity, dt);

    // 2) mise a jour. boucle avec un index et PAS de i++ dans le for, car on n'avance
    // que si on n'a pas supprimé la particule (voir plus bas)
    for (size_t i = 0; i < particles.size(); ) {
        Particle& p = particles[i];
        p.age += dt;                                 // elle vieillit
        if (p.age >= p.lifetime) {
            // elle est morte : suppression "swap-remove" = on ecrase la particule par la derniere
            // du tableau puis on retire la derniere. beaucoup plus rapide que erase() qui
            // decalerait tout le tableau. par contre l'ordre des particules change, mais ca
            // n'a pas d'importance car elles sont dessinées en additif (l'ordre ne change rien au rendu)
            p = particles.back();              // swap-remove, on ne ré-incrémente pas i
            particles.pop_back();
            continue;     // on ne fait pas ++i : la particule qui vient d'arriver a la place i n'a pas encore été traitée
        }
        // physique (Euler semi-implicite) : on met a jour la vitesse avec la gravité d'abord,
        // puis la position avec la NOUVELLE vitesse. ca reste stable meme avec dt qui varie un peu
        p.vel = p.vel + gravity * dt;
        p.pos = p.pos + p.vel * dt;
        ++i;      // celle la est vivante, on passe a la suivante
    }
}
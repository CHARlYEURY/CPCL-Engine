// ============================================================
//  Emitter.cpp
// ------------------------------------------------------------
//  2 fonctions :
//    - rnd  : le generateur de nombres aleatoires (xorshift32)
//    - emit : la creation des particules a chaque frame
// ============================================================
#include "particles/Emitter.h"

// ------------------------------------------------------------
//  rnd : renvoie un float aleatoire dans [0, 1[ (0 inclus, 1 exclu)
//  algo xorshift32 : on mélange les bits de l'etat avec des decalages et des xor.
//  c'est tres rapide (3 decalages, 3 xor) et suffisant pour des particules, pas besoin
//  du rand() de la std. avantage : avec la meme graine on obtient toujours la meme suite,
//  donc le rendu est reproductible (pratique pour debugger).
//  appelée par range(), jamais directement de l'exterieur
// ------------------------------------------------------------
float Emitter::rnd()
{
    _rng ^= _rng << 13; _rng ^= _rng >> 17; _rng ^= _rng << 5;   // xorshift32
    // on garde les 24 bits du bas puis on divise par 2^24 (16777216) -> resultat dans [0,1[
    // 24 bits car c'est la precision d'un float, au dela ca servirait a rien
    return (_rng & 0xFFFFFF) / 16777216.0f;
}

// ------------------------------------------------------------
//  emit : fait naitre les particules de cette frame
//  1) combien en faire naitre ? rate * dt, mais c'est rarement un nombre entier
//     (ex : 300/s a 60fps = 5 par frame, mais 250/s = 4.16). donc on utilise un accumulateur :
//     on ajoute rate*dt a _accum, chaque particule creée retire 1, et le reste (la partie decimale)
//     est gardé pour la frame suivante. comme ca le debit moyen est exact quelque soit le fps
//  2) pour chaque particule : direction au hasard, vitesse, duree de vie, taille tirées au hasard
//     entre les min et max, puis ajout dans le pool
//  pool = tableau de ParticleSystem, capacity = limite max du pool
//  appelée par ParticleSystem::update, tout au debut
// ------------------------------------------------------------
void Emitter::emit(std::vector<Particle>& pool, size_t capacity, float dt)
{
    _accum += rate * dt;
    while (_accum >= 1.0f) {
        _accum -= 1.0f;     // on "consomme" une particule du compteur
        // pool plein : on ne spawn plus. on a quand meme retiré 1 de _accum juste avant,
        // sinon l'accumulateur grossirait sans fin et il y aurait une rafale de particules des qu'une place se libere
        if (pool.size() >= capacity) continue;   // pool plein : on ne spawn plus

        // direction : un vecteur qui pointe vers le haut (y = 1) avec un petit decalage
        // aleatoire en x et z, puis normalisé pour avoir une longueur de 1.
        // spread = taille du decalage : 0 -> tout droit, grand -> jet qui s'ouvre en cone.
        // le y vaut 1 donc le vecteur n'est jamais nul (Normalize est donc sans danger)
        Maths::Vec3_SSE dir = Maths::Vec3_SSE(range(-spread, spread), 1.0f,
            range(-spread, spread)).Normalize();

        // on remplit TOUS les champs de la particule (la struct n'a pas de valeurs par defaut)
        Particle p;
        p.pos = position;                               // toutes naissent au meme point
        p.vel = dir * range(speedMin, speedMax);        // direction * vitesse au hasard
        p.age = 0.0f;                                   // elle vient de naitre
        p.lifetime = range(lifeMin, lifeMax);           // chaque particule a sa propre durée de vie
        p.size = range(sizeMin, sizeMax);
        p.colorStart = colorStart;                      // on copie les couleurs de l'emitter : si on les change
        p.colorEnd = colorEnd;                          // apres, les particules deja en vie gardent les anciennes
        pool.push_back(p);
    }
}
// ============================================================
//  clip.h
// ------------------------------------------------------------
//  clipping des triangles contre le plan NEAR (le plan le plus proche de la camera).
//
//  un sommet qui est derriere la camera (ou pile dessus) a un
//  w <= 0 en clip space, et la division par w (projection perspective) donnerait des
//  coordonnées fausses ou une division par zero. on doit donc couper les triangles qui
//  traversent le plan near AVANT de les projeter a l'ecran.
//
//  le travail se fait en clip space (apres la multiplication par la mvp, AVANT la division par w) :
//  la c'est encore linéaire, donc on peut interpoler les attributs (position, normale, uv)
//  simplement avec un lerp.
//
//  utilisé par : Renderer::draw_mesh (une fois par triangle, juste apres la mvp)
//  les autres plans (gauche, droite, haut, bas, far) ne sont PAS clippés ici.
// ============================================================
#pragma once
#include <xmmintrin.h>      
#include "Vec3_SSE.h"
#include "Vec4_SSE.h"
#include "asset/Mesh.h"  

// ------------------------------------------------------------
//  clip_vertex : un sommet en cours de traitement, avec tout ce qu'il faut garder
//  pendant le clipping : quand on cree un nouveau sommet sur le plan near il faut
//  calculer aussi sa normale et son uv, pas seulement sa position
// ------------------------------------------------------------
struct clip_vertex {
    Maths::Vec4_SSE clip;     // position clip space (avant division par w)
    Maths::Vec3_SSE normal;   // normale monde
    Maths::UV uv;
};

// ------------------------------------------------------------
//  clip_lerp : interpolation lineaire entre deux sommets
//  r = a + t * (b - a), pour tous les attributs
//  t = 0 -> a, t = 1 -> b, t = 0.5 -> le milieu.
//  avec SSE on fait les 4 composantes d'un vecteur en une seule instruction :
//    _mm_set1_ps(t)  : met t dans les 4 cases du registre
//    _mm_sub_ps      : b - a  (4 floats d'un coup)
//    _mm_mul_ps      : t * (b - a)
//    _mm_add_ps      : a + ...
//  les uv (2 floats) sont faits a la main.
//  utilisée seulement par clip_triangle_near, pour creer les sommets sur le plan near
// ------------------------------------------------------------
inline clip_vertex clip_lerp(const clip_vertex& a, const clip_vertex& b, float t)
{
    const __m128 vt = _mm_set1_ps(t);
    clip_vertex r;
    r.clip.reg = _mm_add_ps(a.clip.reg, _mm_mul_ps(vt, _mm_sub_ps(b.clip.reg, a.clip.reg)));
    r.normal.reg = _mm_add_ps(a.normal.reg, _mm_mul_ps(vt, _mm_sub_ps(b.normal.reg, a.normal.reg)));
    r.uv.x = a.uv.x + t * (b.uv.x - a.uv.x);
    r.uv.y = a.uv.y + t * (b.uv.y - a.uv.y);
    return r;
}

// ------------------------------------------------------------
//  clip_near_dist : distance "signée" d'un sommet au plan near
//  Distance signée au plan near : >= 0 = devant le plan (visible).
//  Convention OpenGL : -w <= z  <=>  z + w >= 0
//  en clip space un point est visible si -w <= z <= w. le plan near c'est z = -w,
//  donc "z + w" vaut 0 pile sur le plan, positif devant, negatif derriere.
//  m128_f32[2] = z et m128_f32[3] = w (m128_f32 = acces aux 4 floats du registre sous msvc)
// ------------------------------------------------------------
inline float clip_near_dist(const clip_vertex& v)
{
    return v.clip.reg.m128_f32[2] + v.clip.reg.m128_f32[3];
}

// ------------------------------------------------------------
//  clip_triangle_near : coupe un triangle contre le plan near
//  Clippe un triangle contre le plan near.
//  out doit pouvoir contenir 6 sommets (2 triangles). *out_n_tris vaut 0, 1 ou 2.
//
//  algo = Sutherland-Hodgman pour un seul plan. on parcourt les 3 aretes (a -> b)
//  du triangle et pour chacune on regarde quels bouts sont devant le plan :
//
//     a devant, b devant     -> on garde b
//     a devant, b derriere   -> on garde le point d'intersection avec le plan
//     a derriere, b devant   -> on garde l'intersection PUIS b
//     a derriere, b derriere -> on ne garde rien
//
//  (a chaque arete on ajoute que la fin "b", comme ca chaque sommet est ajouté une seule fois)
//  le resultat est un polygone de 0, 3 ou 4 sommets :
//     0 sommet  -> triangle completement derriere la camera, on le jette
//     3 sommets -> un triangle (soit rien a couper, soit un seul sommet est resté devant)
//     4 sommets -> un quadrilatere (un sommet derriere, deux devant) qu'on redécoupe en 2 triangles
//  l'ordre des sommets est conservé, donc le sens de rotation (winding) du triangle ne change pas
//  et le back-face culling du Renderer marche toujours apres.
//
//  v0, v1, v2 = le triangle d'entrée / out = tableau de sortie (6 sommets max) / out_n_tris = nb de triangles produits
//  appelée par Renderer::draw_mesh
// ------------------------------------------------------------
inline void clip_triangle_near(const clip_vertex& v0, const clip_vertex& v1, const clip_vertex& v2,
    clip_vertex* out, int* out_n_tris)
{
    const clip_vertex in[3] = { v0, v1, v2 };    // copie dans un tableau pour pouvoir boucler dessus
    clip_vertex poly[4];     // polygone resultat : 4 max (un triangle coupé par un plan donne au plus 4 sommets)
    int n = 0;               // nombre de sommets deja dans poly

    for (int e = 0; e < 3; e++) {
        const clip_vertex& a = in[e];              // debut de l'arete
        const clip_vertex& b = in[(e + 1) % 3];    // fin de l'arete (le % 3 reboucle : la derniere arete va de v2 a v0)
        const float da = clip_near_dist(a);
        const float db = clip_near_dist(b);
        const bool a_in = da >= 0.0f;      // true = devant le plan near (visible)
        const bool b_in = db >= 0.0f;

        if (a_in && b_in) {
            poly[n++] = b;
        }
        else if (a_in && !b_in) {
            // t = ou est l'intersection sur le segment a->b (entre 0 et 1).
            // on cherche t tel que la distance vaille 0 : da + t * (db - da) = 0  ->  t = da / (da - db)
            poly[n++] = clip_lerp(a, b, da / (da - db));
        }
        else if (!a_in && b_in) {
            poly[n++] = clip_lerp(a, b, da / (da - db));    // meme formule pour l'intersection
            poly[n++] = b;
        }
        // si les deux sont derriere on fait rien
    }

    if (n == 3) {
        // un seul triangle : on le recopie tel quel
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        *out_n_tris = 1;
    }
    else if (n == 4) {
        // quadrilatere 0-1-2-3 coupé en 2 triangles qui partagent la diagonale 0-2 : (0,1,2) et (0,2,3)
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        out[3] = poly[0]; out[4] = poly[2]; out[5] = poly[3];
        *out_n_tris = 2;
    }
    else {
        // n = 0 : tout est derriere la camera
        *out_n_tris = 0;
    }
}
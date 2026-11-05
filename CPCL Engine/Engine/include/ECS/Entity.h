// ============================================================
//  Entity.h
// ------------------------------------------------------------
//  c'est juste un numero (un id) qui sert de "clé"
//  ce sont les composants (Transform, MeshRenderer, Spin...) qui portent les données,
//  rattachés a cette clé dans les ComponentPool.
//
//  utilisé par : Registry (qui distribue les id) et ComponentPool (qui s'en sert d'index)
// ============================================================
#pragma once
#include <cstdint>


// une entité = un entier 32 bits non signé
using Entity = uint32_t;
// valeur speciale "pas d'entité" (le plus grand uint32 possible, donc jamais un vrai id
// vu que Registry::create commence a 0 et incremente)
constexpr Entity NULL_ENTITY = UINT32_MAX;
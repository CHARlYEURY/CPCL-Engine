// ============================================================
//  Spin.h  (composant)
// ------------------------------------------------------------
//  marque une entité comme "doit tourner sur elle meme" et donne la vitesse.
//  juste des données : c'est SpinSystem qui applique la rotation au Transform.
//  une entité sans Spin ne tourne pas.
// ============================================================
#pragma once 

// vitesse de rotation autour de X et de Y, en radians par seconde
struct Spin { float speedX = 0.5f, speedY = 0.8f; };
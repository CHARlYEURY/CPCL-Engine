#include "asset/Terrain.h"

//  Génération PROCÉDURALE d'un terrain : on ne charge pas de fichier,
//  construit avec un algorithme. result
// 
//  Principe en 3 temps :
//    1) hash         : un "fake rand" reproductible
//    2) value_noise  : du bruit lisse construit à partir du hash
//    3) fBm          : on superpose plusieurs bruits de tailles différentes pour avoir un relief naturel
//  Puis generate_Terrain place une grille de points, hauteur avec fBm.


//  hash : transforme 2 entiers (ix, iy) en un nombre "random" entre 0 et 1
float hash(int ix, int iy) {
	int n = ix * 1869 + iy * 6965;	// + chiffre elever + gros melange 
	n = (n << 13) ^ n;				//  On mélange les bits avec des multiplications et des décalages (<<, ^),
	int hased = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff; // "& 0x7fffffff" enlève le bit de signe pour avoir un nombre positif

	return (float)hased / (float)INT32_MAX; // divise par le max d'un int pour ramener le résultat entre 0 et 1
}

//  Interpolation linéaire simple
//  "static" = only this file 
static float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

//  value_noise : bruit "lisse" en 2D (x, z) -> valeur entre 0 et 1
//
//  Idée : on a une grille dont chaque coin d'une case entière a une
//  valeur "aléatoire" (hash). Pour un point (x, z) quelconque :
//  Sans l'étape de lissage on verrait des "marches"
float value_noise(float x, float z) {
	// la case qui le contient (coins ix0/ix1 et iz0/iz1)
	int ix0 = std::floor(x);
	int iz0 = std::floor(z);
	int ix1 = ix0 + 1;
	int iz1 = iz0 + 1;

	// on calcule où il est DANS la case (fx, fz entre 0 et 1)
	float fx = x - std::floor(x);
	float fz = z - std::floor(z);

	// smoothstep : t*t*(3 - 2t). Courbe en S qui "adoucit" le passage d'un
	// coin à l'autre (pente nulle aux bords de la case -> pas de cassure visible)
	float sx = fx * fx * (3.0f - 2.0f * fx);
	float sz = fz * fz * (3.0f - 2.0f * fz);

	// valeur "aléatoire" de chacun des 4 coins
	float v00 = hash(ix0, iz0);
	float v10 = hash(ix1, iz0);
	float v01 = hash(ix0, iz1);
	float v11 = hash(ix1, iz1);

	// On mélange les 4 coins selon cette position (interpolation bilinéaire)
	// 1er mélange : en x
	float nx0 = lerp(v00, v10, sx);
	float nx1 = lerp(v01, v11, sx);
	//puis en z
	return lerp(nx0, nx1, sz);
}

//  fBm (fractional Brownian motion) : 
//  Les grosses collines viennent de la 1re octave, la rugosité des suivantes.
//  fréquenc et amplitude à régler pour changer le relief.
//  Note : max_value est calculé mais pas utilisé, on renvoie "total" tel quel
//  (la hauteur max possible est donc la somme des amplitudes, ~2.8 pour 4 octaves).
//  Si on voulait une hauteur entre 0 et 1 il faudrait renvoyer total / max_value.
float fBm(float x, float z, int octaves) {
	float total = 0.0f;
	float frequency = 0.3f;
	float amplitude = 1.5f;
	float max_value = 0.0f;

	for (int i = 0; i < octaves; i++) {
		total += value_noise(x * frequency, z * frequency) * amplitude;
		max_value += amplitude;

		frequency *= 2.0f;
		amplitude *= 0.5f;
	}

	return total;
}

//  generate_Terrain : remplit "mesh"
//
//  Étape 1 : une grille de n_points x n_points sommets
//    - posX et posZ répartis de -taille/2 à +taille/2
//    - posY (la hauteur) = fBm(posX, posZ)
//    - un UV par sommet, pour plaquer la texture
//  Étape 2 : chaque case de la grille (un carré a-b-c-d) est coupée en 2 triangles
//
//        a --- b          a = (x, z)      b = (x+1, z)
//        |   / |          c = (x, z+1)    d = (x+1, z+1)
//        |  /  |
//        c --- d          triangles : (a, c, b) et (b, c, d)
//
//  L'ordre des sommets (winding) est choisi pour que les normales
//  pointent vers le HAUT, sinon le terrain serait vu "de dos" et supprimé.
void generate_Terrain(Mesh& mesh) {
	mesh.verts.clear();
	mesh.uvs.clear();
	mesh.faces.clear();
	mesh.uvFaces.clear();

	// 1 les sommets
	for (int z = 0; z < n_points; z++) {
		for (int x = 0; x < n_points; x++) {

			float posX = -taille / 2 + x * step;
			float posZ = +taille / 2 + z * step;

			float posY = fBm(posX, posZ, 4);

			mesh.verts.push_back(Maths::Vec3_SSE(posX, posY, posZ));

			// UV : va de 0 à "taille" sur la grille
			float u = taille * x / (n_points - 1);
			float v = taille * z / (n_points - 1);

			Maths::UV uv_coord;
			uv_coord.x = u;
			uv_coord.y = v;
			mesh.uvs.push_back(uv_coord);
		}
	}
		
	// 2 les triangles
	// on s'arrête à n_points - 1 car une case a besoin de son voisin (x+1, z+1)
	for (int z = 0; z < n_points - 1; z++) {
		for (int x = 0; x < n_points - 1; x++) {
			int a = z * n_points + x;
			int b = z * n_points + (x + 1);
			int c = (z + 1) * n_points + x;
			int d = (z + 1) * n_points + (x + 1);

			mesh.faces.push_back(a);
			mesh.faces.push_back(c);
			mesh.faces.push_back(b);

			mesh.uvFaces.push_back(a);
			mesh.uvFaces.push_back(c);
			mesh.uvFaces.push_back(b);

			mesh.faces.push_back(b);
			mesh.faces.push_back(c);
			mesh.faces.push_back(d);

			mesh.uvFaces.push_back(b);
			mesh.uvFaces.push_back(c);
			mesh.uvFaces.push_back(d);
		}
	}

}
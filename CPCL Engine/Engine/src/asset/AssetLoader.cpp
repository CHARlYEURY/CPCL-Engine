#include "asset/AssetLoader.h"
#include <stdio.h>
#include <stdlib.h>


//  Chargeur de fichiers .obj (Wavefront). Il transforme un fichier texte
//  en un Mesh (liste de sommets + liste de triangles + UV).
//
//  Fonctions :
//    - ObjReadFaceToken : lit UN "mot" d'une ligne f (ex : "12/5/3")
//    - mesh_load        : lit tout le fichier et remplit le Mesh


//  ObjReadFaceToken : lit un "mot" d'une ligne de face et avance le pointeur p
//
//  Un mot peut avoir 4 formes :   "v"   "v/vt"   "v/vt/vn"   "v//vn"
//    v  = indice du sommet, vt = indice de l'UV, vn = indice de la normale
//  On ne garde que v et vt (les normales, on les recalcule nous-mêmes dans Mesh.cpp).
//
//  Paramètres :
//    p          : position de lecture dans la ligne
//    vertCount  : nombre de sommets déjà lus (sert pour les indices négatifs)
//    uvCount    : nombre d'UV déjà lus (same)
//    outV, outVt: résultats (indices en base 0). outVt = -1 s'il n'y a pas d'UV

static bool ObjReadFaceToken(const char*& p, int vertCount, int uvCount, int& outV, int& outVt) {
    //skip space and tab
     while (*p == ' ' || *p == '\t') ++p;
     //  Retourne false s'il n'y a plus de mot à lire sur la ligne.
     if (*p == '\0' || *p == '\n' || *p == '\r') return false;

     // strtol lit un entier et nous dit où il s'est arrêté (end)
     char* end;
     long v = strtol(p, &end, 10);
     if (end == p) return false;  // pas d'entier -> ligne invalide
     p = end;
     outV = (v < 0) ? vertCount + (int)v : (int)v - 1; // v-1 car nous on veut start à 0
     outVt = -1;
     // Un indice NÉGATIF veut dire "en partant de la fin" (-1 = dernier sommet lu)


     if (*p == '/') {
         ++p;
         if (*p != '/') {                       // il y a un vt (sinon c'est la forme "v//vn")
             long t = strtol(p, &end, 10);
             if (end != p) {
                 outVt = (t < 0) ? uvCount + (int)t : (int)t - 1;
                 p = end;
             }
         }
     }
	 // on saute le reste (/vn) jusqu'au prochain space/tab ou fin de ligne
     while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') ++p;
     return true;
 }

// mesh_load : charge un fichier.obj dans "out"
//  Retourne true si le fichier a pu être ouvert ET contient au moins
//  un sommet et une face, false sinon.
//
//  Principe : on lit le fichier ligne par ligne et on regarde les premiers
//  caractères pour savoir quel type de ligne c'est.
bool mesh_load(const char* path, Mesh& out) {
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "could not open %s\n", path); return false; }

    out.verts.clear();
    out.faces.clear();
	out.uvs.clear();
	out.uvFaces.clear();


    char line[256];
    while (fgets(line, sizeof line, f)) {
        // ligne v : sommet
        if (line[0] == 'v' && line[1] == ' ')
        {
            float x, y, z;
            if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3) // line + 2 = on saute le "v " ; sscanf renvoie le nombre de valeurs lues
            {
                out.verts.push_back(Maths::Vec3_SSE(x, y, z));
            }
        }

        // ligne "vt u v" : coord texture
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            // a texture coordinate: two floats
            Maths::UV t;
            if (sscanf(line + 3, "%f %f", &t.x, &t.y) >= 2) { // ">= 2" car certains .obj écrivent un 3e nombre (w) qu'on ignore
                out.uvs.push_back(t);
            }
        }
		// ligne "f ..." : face
        else if (line[0] == 'f' && line[1] == ' ')
        {
            const char* p = line + 2;      // on se place juste après "f "
            const int n = out.vertCount(); // nombre de sommets lus jusque là
            const int nt = out.uvCount();  // nombre de coordonnées UV lues jusque là
            int vFirst, tFirst, vPrev, tPrev, vCur, tCur;

            // une face a au moins 3 sommets : on lit les 2 premiers d'avance
            // (si on n'y arrive pas, la ligne est invalide -> on l'ignore
            if (!ObjReadFaceToken(p, n, nt, vFirst, tFirst) ||
                !ObjReadFaceToken(p, n, nt, vPrev, tPrev)) continue;

            // Triangulation "en éventail" (fan) : pour une face à N sommets,
            // chaque nouveau sommet forme un triangle avec le 1er et le précédent.

            while (ObjReadFaceToken(p, n, nt, vCur, tCur)) {
                // on vérifie que les indices de sommets existent bien (sinon fichier corrompu)
                if (vFirst >= 0 && vFirst < n && vPrev >= 0 && vPrev < n && vCur >= 0 && vCur < n) {
                    out.faces.push_back(vFirst);
                    out.faces.push_back(vPrev);
                    out.faces.push_back(vCur);

                    out.uvFaces.push_back(tFirst);
                    out.uvFaces.push_back(tPrev);
                    out.uvFaces.push_back(tCur);
                }
                // le "courant" devient le "précédent" pour le triangle suivant
                vPrev = vCur;
                tPrev = tCur;
            }
        }
    }

    fclose(f);
    // succès = on a trouvé des sommets ET des faces
    return !out.verts.empty() && !out.faces.empty();;
}
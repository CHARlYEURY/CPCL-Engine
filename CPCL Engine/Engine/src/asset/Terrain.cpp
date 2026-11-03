#include "asset/Terrain.h"

void generate_Terrain(Mesh& mesh) {
	mesh.verts.clear();
	mesh.uvs.clear();

	for (int x = 0; x < n_points; x++) {
		for (int z = 0; z < n_points; z++) {
			/*x = z * n_points + x;
			z  = z * n_points + x;*/
			float posX = -taille / 2 + x * step;
			float posY = -taille / 2 + z * step;
			mesh.verts.push_back(Maths::Vec3_SSE(posX, 1, posY));

			float u = (float)taille / (n_points - 1);
			float v = (float)taille / (n_points - 1);

			Maths::UV uv_coord;
			uv_coord.x = u;
			uv_coord.y = v;
			mesh.uvs.push_back(uv_coord);
		}
	}

	for (int x = 0; x < n_points-1; x++) {
		for (int z = 0; z < n_points - 1; z++) {
			int a = z * n_points + x;
			int b = z * n_points + (x + 1);
			int c = (z + 1) * n_points + x;
			int d = (z + 1) * n_points + (x + 1);

			mesh.faces.push_back(a);
			mesh.faces.push_back(b);
			mesh.faces.push_back(c);

			mesh.uvFaces.push_back(a);
			mesh.uvFaces.push_back(b);
			mesh.uvFaces.push_back(c);

			mesh.faces.push_back(b);
			mesh.faces.push_back(d);
			mesh.faces.push_back(c);

			mesh.uvFaces.push_back(b);
			mesh.uvFaces.push_back(d);
			mesh.uvFaces.push_back(c);
		}
	}

}
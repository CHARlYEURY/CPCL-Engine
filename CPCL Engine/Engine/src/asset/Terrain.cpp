#include "asset/Terrain.h"


float hash(int ix, int iy) {
	int n = ix * 1869 + iy * 6965;
	n = (n << 13) ^ n;
	int hased = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;

	return (float)hased	/ (float)INT32_MAX;
}

// Fonction d'interpolation linéaire simple
static float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

float value_noise(float x, float z) {
	int ix0 = std::floor(x);
	int iz0 = std::floor(z);
	int ix1 = ix0 + 1;
	int iz1 = iz0 + 1;

	float fx = x - std::floor(x);
	float fz = z - std::floor(z);

	float sx = fx * fx * (3.0f - 2.0f * fx);
	float sz = fz * fz * (3.0f - 2.0f * fz);

	float v00 = hash(ix0, iz0);
	float v10 = hash(ix1, iz0);
	float v01 = hash(ix0, iz1);
	float v11 = hash(ix1, iz1);

	float nx0 = lerp(v00, v10, sx);
	float nx1 = lerp(v01, v11, sx);

	return lerp(nx0, nx1, sz);
}

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

void generate_Terrain(Mesh& mesh) {
	mesh.verts.clear();
	mesh.uvs.clear();
	mesh.faces.clear();
	mesh.uvFaces.clear();

	for (int z = 0; z < n_points; z++) {
		for (int x = 0; x < n_points; x++) {

			float posX = -taille / 2 + x * step;
			float posZ = -taille / 2 + z * step;

			float posY = fBm(posX, posZ, 4);

			mesh.verts.push_back(Maths::Vec3_SSE(posX, posY, posZ));

			float u = taille * x / (n_points - 1);
			float v = taille * z / (n_points - 1);

			Maths::UV uv_coord;
			uv_coord.x = u;
			uv_coord.y = v;
			mesh.uvs.push_back(uv_coord);
		}
	}

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
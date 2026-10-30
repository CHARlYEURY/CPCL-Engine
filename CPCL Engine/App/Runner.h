#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include "renderer/Renderer.h"
#include "Vec3_SSE.h"
#include "Matrix4x4_SSE.h"
#include <numbers>
#include "asset/AssetLoader.h"
#include "asset/textureLoader.h"
#include "asset/Font.h"
#include "asset/TextRenderer.h"
#include "platform/Window.h"
#include "particles/ParticleSystem.h"
#include "particles/Emitter.h"
#include "camera/camera.h"
#include <memory>


class RunApp {
private:
	//Window
	const int SCALE = 1;
private:
	//Normales
	std::vector<Maths::Vec3_SSE> _face_normals;
	std::vector<Maths::Vec3_SSE> _vertex_normals;
	std::vector<Maths::Vec3_SSE> _face_normals2;
	std::vector<Maths::Vec3_SSE> _vertex_normals2;
private:
	//Lumière / la caméra
	const Maths::Vec3_SSE _light = Maths::Vec3_SSE(0.4f, 0.8f, 0.45f).Normalize();
	const Maths::Vec3_SSE _view_dir = Maths::Vec3_SSE(0.0f, 0.0f, 1.0f);
	const Maths::Vec3_SSE _half_vec = (_light + _view_dir).Normalize();
private:
	const Maths::Vec3_SSE _eye = Maths::Vec3_SSE(0, 0, 3);
	const Maths::Vec3_SSE _target = Maths::Vec3_SSE(0, 0, 0);
	const Maths::Vec3_SSE _up = Maths::Vec3_SSE(0, 1, 0);
private:
	std::vector<drawItem> _scene;
private:
	//FPS
	const Uint64 freq = SDL_GetPerformanceFrequency();
	const Uint64 start = SDL_GetPerformanceCounter();
	Uint64 last = start;
	double fps_accum = 0.0;
	int    fps_count = 0;
	char   title[1600];

public:

	RunApp();

	void run(int width, int height);

};
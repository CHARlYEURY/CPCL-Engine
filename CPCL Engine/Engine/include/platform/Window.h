#pragma once
#include <SDL3/SDL.h>
#include "renderer/Framebuffer.h"

// Regroupe le bloc SDL qui était dans main.cpp ("pas faire gaffe") :
// SDL_Window + SDL_Renderer + SDL_Texture qui reçoit les pixels du framebuffer.
class Window
{
private:
	SDL_Window* _window = nullptr;
	SDL_Renderer* _renderer = nullptr;
	SDL_Texture* _texture = nullptr;

public:
	// winW/winH = taille de la fenêtre, texW/texH = taille du framebuffer
	Window(const char* title, int winW, int winH, int texW, int texH);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool IsValid() const { return _window && _renderer && _texture; }
	void SetTitle(const char* title);

	// SDL_UpdateTexture + RenderClear + RenderTexture + RenderPresent
	void Present(const FrameBuffer& fb);

	// À appeler AVANT SDL_Quit() (le destructeur le rappelle sans danger)
	void Destroy();
};

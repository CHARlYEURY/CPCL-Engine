#include "platform/Window.h"

Window::Window(const char* title, int winW, int winH, int texW, int texH)
{
    _window = SDL_CreateWindow(title, winW, winH, SDL_WINDOW_TRANSPARENT);
    if (_window)
        _renderer = SDL_CreateRenderer(_window, nullptr);

    if (_renderer)
    {
        // Texture qui reçoit les pixels du framebuffer
        _texture = SDL_CreateTexture(_renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            texW, texH);

        // Pixels nets (pas de lissage) quand on agrandit
        if (_texture)
            SDL_SetTextureScaleMode(_texture, SDL_SCALEMODE_NEAREST);
    }
}

Window::~Window()
{
    Destroy();
}

void Window::SetTitle(const char* title)
{
    SDL_SetWindowTitle(_window, title);
}

void Window::Present(const FrameBuffer& fb)
{
    SDL_UpdateTexture(_texture, nullptr, fb.Data(), fb.getWidth() * (int)sizeof(uint32_t));
    SDL_RenderClear(_renderer);
    SDL_RenderTexture(_renderer, _texture, nullptr, nullptr);
    SDL_RenderPresent(_renderer);
}

void Window::Destroy()
{
    if (_texture)  { SDL_DestroyTexture(_texture);   _texture = nullptr; }
    if (_renderer) { SDL_DestroyRenderer(_renderer); _renderer = nullptr; }
    if (_window)   { SDL_DestroyWindow(_window);     _window = nullptr; }
}

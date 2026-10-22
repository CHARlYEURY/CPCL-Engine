#include "asset/Font.h"
#include <iostream>

bool FontTag::contains(const std::string& str) const {
    return _name.find(str) != std::string::npos;
}

Font::Font(const FontTag& tag, const FilePath& fileName) : 
    _tag(tag), _fileName(fileName) {}


TTF_Font* Font::getFont(uint16_t fontSize) {
    auto it = _fonts.find(fontSize);
    if (it != _fonts.end() && it->second)
        return it->second;

    TTF_Font* font = TTF_OpenFont(_fileName.getString().c_str(), static_cast<float>(fontSize));
    if (!font) {
        std::cerr << "Failed to load font: " << _fileName.getString()
            << " size " << fontSize << " (" << SDL_GetError() << ")\n";
        return nullptr;
    }
    _fonts[fontSize] = font;
    return font;
}

Font::~Font() {
    for (auto& p : _fonts)
        if (p.second)
            TTF_CloseFont(p.second);

	_fonts.clear();
}

std::pair<int, int> Font::getTextSize(const std::string& text, uint16_t fontSize) {
    TTF_Font* font = getFont(fontSize);
    if (!font) return {0, 0};

    int width = 0;
    int height = 0;

    if (!TTF_GetStringSize(font, text.c_str(), 0, &width, &height)) {   // SDL3 : renvoie true si OK
        std::cerr << "Failed to get text size: " << SDL_GetError() << "\n";
        return { 0, 0 };
    }
    return {width, height};
}

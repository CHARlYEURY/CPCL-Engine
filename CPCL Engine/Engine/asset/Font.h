#pragma once

#ifndef UTILS_FONT_H

#define UTILS_FONT_H

#include <string>
#include <map>

//#include "SDL_ttf.h"

class Font
{
private:

	const FilePath _fileName;
	std::map<uint16_t, TTF_Font*> _fonts;

public:

	Font() {}

	TTF_Font* getFont(uint16_t fontSize = 14);

	[[nodiscard]] std::pair<int, int> getTextSize(const std::string& text, uint16_t fontSize = 14);

	~Font();
};

#endif UTILS_FONT_H
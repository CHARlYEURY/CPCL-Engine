#pragma once

#ifndef UTILS_FONT_H

#define UTILS_FONT_H

#include <string>
#include <map>
#include "core/FilePath.h"
#include <SDL3_ttf/SDL_ttf.h>

class FontTag {
private:
	const std::string _name;

public:
	explicit FontTag(const std::string& name = "") : _name(name) {}
	/**
	 * @brief Copy constructor.
	 * @param other The other FontTag to copy from.
	 */
	FontTag(const FontTag& other) : _name(other._name) {}

	[[nodiscard]] const std::string& getTagString() const { return _name; }
	[[nodiscard]] bool empty() const { return _name.empty(); }

	// Comparison operators
	bool operator==(const FontTag& tag) const { return _name == tag._name; }
	bool operator!=(const FontTag& tag) const { return _name != tag._name; }
	bool operator<(const FontTag& tag) const { return _name < tag._name; }

	/**
     * @brief Checks if the tag contains a specified string.
     * @param str The string to check for containment.
     * @return True if the tag contains the string, false otherwise.
     */
    [[nodiscard]] bool contains(const std::string& str) const;
};




class Font
{
private:
	const FontTag _tag;			///< The tag identifying this font.
	const FilePath _fileName;	///< The file path to the font file.

	std::map<uint16_t, TTF_Font*> _fonts;

public:

	Font(const FontTag& tag, const FilePath& fileName);
	~Font();

	Font(const Font&) = delete;            // évite un double TTF_CloseFont
	Font& operator=(const Font&) = delete;

	TTF_Font* getFont(uint16_t fontSize = 14);
	[[nodiscard]] std::pair<int, int> getTextSize(const std::string& text, uint16_t fontSize = 14);

	
};

#endif // UTILS_FONT_H

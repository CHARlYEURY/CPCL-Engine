#include "asset/Font.h"
#include <iostream>

//  Gère les polices de caractères avec SDL3_ttf.
//  Une police ne se charge pas une seule fois. La classe Font garde donc un cache
//  (_fonts : taille -> TTF_Font*) pour ne charger chaque taille qu'une fois.
//
//  Contenu :
//    - FontTag::contains : petit outil de recherche dans le nom d'un tag
//    - Font::Font        : constructeur (ne charge rien tout de suite)
//    - Font::getFont     : donne le TTF_Font d'une taille (le charge si besoin)
//    - Font::~Font       : libère toutes les tailles chargées
//    - Font::getTextSize : mesure la taille en pixels d'un texte
//  (inspiré de la classe Font du moteur "exemple" pris sur internet)

//  true si le nom du tag contient la chaîne "str"
//  find renvoie npos quand il ne trouve rien, donc "!= npos" = trouvé
bool FontTag::contains(const std::string& str) const {
    return _name.find(str) != std::string::npos;
}

Font::Font(const FontTag& tag, const FilePath& fileName) : 
    _tag(tag), _fileName(fileName) {}


//  getFont : renvoie le TTF_Font de la taille demandée


//  Retourne nullptr si le chargement échoue (fichier introuvable...)

TTF_Font* Font::getFont(uint16_t fontSize) {
    auto it = _fonts.find(fontSize);    // 1) la taille est déjà dans le cache ? ->on la renvoie directement
    if (it != _fonts.end() && it->second)
        return it->second;

    //  2) sinon on ouvre le fichier .ttf à cette taille avec TTF_OpenFont
    TTF_Font* font = TTF_OpenFont(_fileName.getString().c_str(), static_cast<float>(fontSize));
    if (!font) {
        std::cerr << "Failed to load font: " << _fileName.getString()
            << " size " << fontSize << " (" << SDL_GetError() << ")\n";
        return nullptr;
    }

    // 3) on range le résultat dans le cache pour la prochaine fois
    _fonts[fontSize] = font;
    return font;
}

Font::~Font() {
    for (auto& p : _fonts)
        if (p.second)
            TTF_CloseFont(p.second);

	_fonts.clear();
}

//  getTextSize : largeur et hauteur (en pxls) qu'aura un texte avec cette taille de police. Retourne {0, 0} en cas d'erreur.
//  Le "0" passé à TTF_GetStringSize veut dire "chaîne terminée par \0"
//  (SDL3 : la fonction renvoie true si tout s'est bien passé).
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

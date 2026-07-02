// ============================================================================
//  tekst.hpp — wspoldzielone narzedzia na lancuchach znakow:
//  normalizacja, formatowanie kolumn oraz kodowanie/parsowanie pol CSV.
//  Bez zaleznosci od modelu — czyste operacje tekstowe.
// ============================================================================
#ifndef SERWIS_TEKST_HPP
#define SERWIS_TEKST_HPP

#include <string>
#include <string_view>
#include <vector>

namespace serwis::tekst {

// Zamiana na male litery (porownania bez rozroznienia wielkosci).
std::string naMale(std::string_view s);

// Skrocenie tekstu do 'maks' znakow z kropka na koncu (do tabeli).
std::string skroc(std::string_view s, std::size_t maks);

// Dopelnienie spacjami z prawej do szerokosci 'w' (wyrownanie do lewej).
std::string dopelnij(std::string_view s, std::size_t w);

// Ucieczka znakow specjalnych pola CSV: ';', '\n' oraz '\'.
std::string escape(std::string_view s);
std::string unescape(std::string_view s);

// Podzial wiersza po separatorze z poszanowaniem sekwencji ucieczki '\\x'.
std::vector<std::string> podziel(std::string_view s, char sep);

// Zlaczenie listy tekstow pojedynczym separatorem.
std::string zlacz(const std::vector<std::string>& v, char sep);

}  // namespace serwis::tekst

#endif  // SERWIS_TEKST_HPP

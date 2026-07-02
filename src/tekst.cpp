// ============================================================================
//  tekst.cpp — implementacja narzedzi tekstowych i kodowania CSV.
// ============================================================================
#include "tekst.hpp"

#include <cctype>

namespace serwis::tekst {

std::string naMale(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out += static_cast<char>(std::tolower(c));
    return out;
}

std::string skroc(std::string_view s, std::size_t maks) {
    if (s.size() <= maks) return std::string(s);
    std::size_t ile = maks > 1 ? maks - 1 : 1;
    return std::string(s.substr(0, ile)) + ".";
}

std::string dopelnij(std::string_view s, std::size_t w) {
    std::string out(s);
    if (out.size() < w) out.append(w - out.size(), ' ');
    return out;
}

std::string escape(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if      (c == ';')  out += "\\;";
        else if (c == '\n') out += "\\n";
        else if (c == '\\') out += "\\\\";
        else                out += c;
    }
    return out;
}

std::string unescape(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            out += (n == 'n') ? '\n' : n;
        } else {
            out += s[i];
        }
    }
    return out;
}

std::vector<std::string> podziel(std::string_view s, char sep) {
    std::vector<std::string> pola;
    std::string biezace;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {   // zachowaj sekwencje ucieczki
            biezace += s[i];
            biezace += s[++i];
        } else if (s[i] == sep) {
            pola.push_back(biezace);
            biezace.clear();
        } else {
            biezace += s[i];
        }
    }
    pola.push_back(biezace);
    return pola;
}

std::string zlacz(const std::vector<std::string>& v, char sep) {
    std::string out;
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) out += sep;
        out += v[i];
    }
    return out;
}

}  // namespace serwis::tekst

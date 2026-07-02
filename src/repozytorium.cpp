// ============================================================================
//  repozytorium.cpp — serializacja/deserializacja zlecen do formatu CSV.
//
//  Format wiersza (separator ';', pola uciekane przez tekst::escape):
//    id;klient;telefon;urzadzenie;opis;status;koszt;data;priorytet;historia
//  Historia = wpisy sklejone znakiem '|'. Numer kolejnego ID nie jest
//  zapisywany — wyprowadza go Serwis z najwyzszego wczytanego id.
// ============================================================================
#include "repozytorium.hpp"
#include "tekst.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace serwis {

namespace {
constexpr char SEP      = ';';   // separator pol
constexpr char SEP_HIST = '|';   // separator wpisow historii
constexpr std::size_t MIN_POL = 7;   // minimum pol, by uznac wiersz za rekord
}  // namespace

std::string CsvRepozytorium::doLinii(const Zlecenie& z) {
    std::vector<std::string> historiaEsc;
    historiaEsc.reserve(z.historia.size());
    for (const auto& e : z.historia) historiaEsc.push_back(tekst::escape(e));

    std::ostringstream out;
    out << z.id << SEP
        << tekst::escape(z.klient)     << SEP
        << tekst::escape(z.telefon)    << SEP
        << tekst::escape(z.urzadzenie) << SEP
        << tekst::escape(z.opis)       << SEP
        << statusNaTekst(z.status)     << SEP
        << std::fixed << std::setprecision(2) << z.koszt << SEP
        << z.data                      << SEP
        << priorytetNaTekst(z.priorytet) << SEP
        << tekst::zlacz(historiaEsc, SEP_HIST);
    return out.str();
}

std::optional<Zlecenie> CsvRepozytorium::zLinii(const std::string& linia) {
    if (linia.empty()) return std::nullopt;
    std::vector<std::string> p = tekst::podziel(linia, SEP);
    if (p.size() < MIN_POL) return std::nullopt;   // np. legacy wiersz-licznik

    Zlecenie z;
    try {
        z.id    = std::stoi(p[0]);
        z.koszt = std::stod(p[6]);
    } catch (const std::exception&) {
        return std::nullopt;                       // uszkodzony wiersz -> pomijamy
    }
    z.klient     = tekst::unescape(p[1]);
    z.telefon    = tekst::unescape(p[2]);
    z.urzadzenie = tekst::unescape(p[3]);
    z.opis       = tekst::unescape(p[4]);
    z.status     = tekstNaStatus(p[5]);
    z.data       = p.size() > 7 ? p[7] : dzisiajISO();
    z.priorytet  = p.size() > 8 ? tekstNaPriorytet(p[8]) : Priorytet::Normalny;

    if (p.size() > 9 && !p[9].empty())
        for (const auto& e : tekst::podziel(p[9], SEP_HIST))
            z.historia.push_back(tekst::unescape(e));
    if (z.historia.empty())
        z.historia.push_back(wpisHistorii(z.data, z.status));

    return z;
}

std::vector<Zlecenie> CsvRepozytorium::wczytaj(const std::string& sciezka) const {
    std::vector<Zlecenie> dane;
    std::ifstream in(sciezka);
    if (!in) return dane;                          // brak pliku = pusta lista

    std::string linia;
    while (std::getline(in, linia))
        if (auto z = zLinii(linia)) dane.push_back(std::move(*z));
    return dane;
}

bool CsvRepozytorium::zapisz(const std::string& sciezka,
                             const std::vector<Zlecenie>& dane) const {
    std::ofstream out(sciezka);
    if (!out) return false;
    for (const auto& z : dane) out << doLinii(z) << '\n';
    return static_cast<bool>(out);
}

}  // namespace serwis

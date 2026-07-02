// ============================================================================
//  serwis.cpp — implementacja logiki biznesowej.
// ============================================================================
#include "serwis.hpp"
#include "tekst.hpp"

#include <algorithm>
#include <numeric>

namespace serwis {

Serwis::Serwis(std::vector<Zlecenie> dane) : zlecenia_(std::move(dane)) {
    // Kolejne ID musi byc wieksze od najwyzszego wczytanego, aby uniknac kolizji.
    auto it = std::max_element(zlecenia_.begin(), zlecenia_.end(),
                               [](const Zlecenie& a, const Zlecenie& b) { return a.id < b.id; });
    nastepneId_ = (it == zlecenia_.end()) ? 1 : it->id + 1;
}

int Serwis::dodaj(const std::string& klient, const std::string& telefon,
                  const std::string& urzadzenie, const std::string& opis,
                  Priorytet priorytet) {
    Zlecenie z;
    z.id         = nastepneId_++;
    z.klient     = klient;
    z.telefon    = telefon;
    z.urzadzenie = urzadzenie;
    z.opis       = opis;
    z.priorytet  = priorytet;
    z.data       = dzisiajISO();
    z.historia.push_back(wpisHistorii(z.data, z.status));
    zlecenia_.push_back(std::move(z));
    return zlecenia_.back().id;
}

Zlecenie* Serwis::znajdzMut(int id) {
    auto it = std::find_if(zlecenia_.begin(), zlecenia_.end(),
                           [id](const Zlecenie& z) { return z.id == id; });
    return it == zlecenia_.end() ? nullptr : &*it;
}

const Zlecenie* Serwis::znajdz(int id) const {
    auto it = std::find_if(zlecenia_.begin(), zlecenia_.end(),
                           [id](const Zlecenie& z) { return z.id == id; });
    return it == zlecenia_.end() ? nullptr : &*it;
}

bool Serwis::zmienStatus(int id, Status s) {
    Zlecenie* z = znajdzMut(id);
    if (!z) return false;
    z->status = s;
    z->historia.push_back(wpisHistorii(dzisiajISO(), s));
    return true;
}

bool Serwis::ustawKoszt(int id, double koszt) {
    Zlecenie* z = znajdzMut(id);
    if (!z) return false;
    z->koszt = koszt;
    return true;
}

bool Serwis::usun(int id) {
    auto it = std::remove_if(zlecenia_.begin(), zlecenia_.end(),
                             [id](const Zlecenie& z) { return z.id == id; });
    if (it == zlecenia_.end()) return false;
    zlecenia_.erase(it, zlecenia_.end());
    return true;
}

std::vector<const Zlecenie*> Serwis::wedlugKosztu() const {
    std::vector<const Zlecenie*> widok;
    widok.reserve(zlecenia_.size());
    for (const auto& z : zlecenia_) widok.push_back(&z);
    std::sort(widok.begin(), widok.end(),
              [](const Zlecenie* a, const Zlecenie* b) { return a->koszt > b->koszt; });
    return widok;
}

std::vector<const Zlecenie*> Serwis::szukajKlient(std::string_view fraza) const {
    std::string szukane = tekst::naMale(fraza);
    std::vector<const Zlecenie*> wynik;
    for (const auto& z : zlecenia_)
        if (tekst::naMale(z.klient).find(szukane) != std::string::npos)
            wynik.push_back(&z);
    return wynik;
}

std::vector<const Zlecenie*> Serwis::gotoweDoOdbioru() const {
    std::vector<const Zlecenie*> wynik;
    for (const auto& z : zlecenia_)
        if (z.status == Status::Gotowe) wynik.push_back(&z);
    return wynik;
}

Statystyki Serwis::statystyki() const {
    Statystyki s;
    s.lacznie = zlecenia_.size();
    for (const auto& z : zlecenia_) s.wgStatusu[indeks(z.status)]++;
    s.pilne = static_cast<std::size_t>(std::count_if(zlecenia_.begin(), zlecenia_.end(),
              [](const Zlecenie& z) { return z.priorytet == Priorytet::Pilny; }));
    s.przychod = std::accumulate(zlecenia_.begin(), zlecenia_.end(), 0.0,
              [](double suma, const Zlecenie& z) {
                  return suma + (z.status == Status::Wydane ? z.koszt : 0.0);
              });
    return s;
}

}  // namespace serwis

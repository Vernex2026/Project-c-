// ============================================================================
//  serwis.hpp — logika biznesowa (operacje na kolekcji zlecen).
//  CZYSTA warstwa: nie drukuje na ekran i nie dotyka plikow. Metody zwracaja
//  dane lub status operacji; o prezentacji decyduje warstwa UI.
// ============================================================================
#ifndef SERWIS_SERWIS_HPP
#define SERWIS_SERWIS_HPP

#include "domena.hpp"

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

namespace serwis {

// Zagregowane statystyki (obliczane, zwracane do wyswietlenia przez UI).
struct Statystyki {
    std::size_t lacznie = 0;
    std::array<std::size_t, LICZBA_STATUSOW> wgStatusu{};  // indeksowane indeks(Status)
    std::size_t pilne = 0;
    double      przychod = 0.0;                            // suma kosztow zlecen wydanych
};

class Serwis {
public:
    Serwis() = default;
    // Budowa z danych wczytanych z repozytorium; nastepne ID = max(id)+1.
    explicit Serwis(std::vector<Zlecenie> dane);

    // --- Modyfikacje ----------------------------------------------------
    int  dodaj(const std::string& klient, const std::string& telefon,
               const std::string& urzadzenie, const std::string& opis,
               Priorytet priorytet);
    bool zmienStatus(int id, Status s);
    bool ustawKoszt(int id, double koszt);
    bool usun(int id);

    // --- Odczyt (zwraca dane, nie drukuje) ------------------------------
    [[nodiscard]] const std::vector<Zlecenie>& wszystkie() const noexcept { return zlecenia_; }
    [[nodiscard]] const Zlecenie* znajdz(int id) const;
    [[nodiscard]] std::vector<const Zlecenie*> wedlugKosztu() const;
    [[nodiscard]] std::vector<const Zlecenie*> szukajKlient(std::string_view fraza) const;
    [[nodiscard]] std::vector<const Zlecenie*> gotoweDoOdbioru() const;
    [[nodiscard]] Statystyki statystyki() const;
    [[nodiscard]] bool pusty() const noexcept { return zlecenia_.empty(); }

private:
    Zlecenie* znajdzMut(int id);   // wewnetrzny odpowiednik nie-const

    std::vector<Zlecenie> zlecenia_;
    int nastepneId_ = 1;
};

}  // namespace serwis

#endif  // SERWIS_SERWIS_HPP

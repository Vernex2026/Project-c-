// ============================================================================
//  konsola.hpp — warstwa prezentacji (UI).
//  Jedyne miejsce z std::cout / std::cin. Renderuje dane zwracane przez Serwis
//  i zbiera wejscie od uzytkownika. Klasa Aplikacja spina calosc (petla menu).
// ============================================================================
#ifndef SERWIS_KONSOLA_HPP
#define SERWIS_KONSOLA_HPP

#include "repozytorium.hpp"
#include "serwis.hpp"

#include <string>

namespace serwis {

// Aplikacja = "composition root" petli interaktywnej: laczy logike (Serwis)
// z trwaloscia (CsvRepozytorium) i obsluguje menu konsolowe.
class Aplikacja {
public:
    Aplikacja(Serwis& serwis, CsvRepozytorium& repo, std::string sciezka);
    void uruchom();

private:
    // Obsluga poszczegolnych pozycji menu (jedna metoda = jedna akcja).
    void obsluzDodaj();
    void obsluzListe(bool wedlugKosztu);
    void obsluzZmianeStatusu();
    void obsluzSzukajId();
    void obsluzSzukajKlient();
    void obsluzUstawKoszt();
    void obsluzGotowe();
    void obsluzUsun();
    void obsluzStatystyki();
    bool obsluzZapis();   // true = zapis udany

    Serwis&          serwis_;
    CsvRepozytorium& repo_;
    std::string      sciezka_;
};

}  // namespace serwis

#endif  // SERWIS_KONSOLA_HPP

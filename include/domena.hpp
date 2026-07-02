// ============================================================================
//  domena.hpp — model dziedziny: zlecenie serwisowe, jego status i priorytet.
//  Warstwa najnizsza: nie wie nic o UI, plikach ani logice aplikacji.
// ============================================================================
#ifndef SERWIS_DOMENA_HPP
#define SERWIS_DOMENA_HPP

#include <string>
#include <string_view>
#include <vector>

namespace serwis {

// Cykl zycia zlecenia w serwisie.
enum class Status { Przyjete, WTrakcie, Gotowe, Wydane };

// Waga zlecenia (kolejnosc obslugi).
enum class Priorytet { Niski, Normalny, Pilny };

// Liczba wartosci w Status — uzywana m.in. do tablic zliczajacych.
inline constexpr std::size_t LICZBA_STATUSOW = 4;

// Pojedyncze zlecenie serwisowe.
struct Zlecenie {
    int         id        = 0;
    std::string klient;
    std::string telefon;
    std::string urzadzenie;
    std::string opis;
    Status      status    = Status::Przyjete;
    Priorytet   priorytet = Priorytet::Normalny;
    double      koszt     = 0.0;
    std::string data;                     // data przyjecia (YYYY-MM-DD)
    std::vector<std::string> historia;    // log zmian: "data - status"
};

// --- Konwersje enum <-> tekst (jedno zrodlo prawdy dla nazw statusow) -------
std::string statusNaTekst(Status s);
Status      tekstNaStatus(std::string_view t);
std::string priorytetNaTekst(Priorytet p);
Priorytet   tekstNaPriorytet(std::string_view t);

// Kod koloru ANSI dla statusu (do wyroznienia w tabeli).
const char* kolorStatus(Status s);

// Indeks statusu (0..LICZBA_STATUSOW-1) — do tablic zliczajacych.
std::size_t indeks(Status s);

// Dzisiejsza data w formacie YYYY-MM-DD.
std::string dzisiajISO();

// Wpis do historii: "YYYY-MM-DD - <status>".
std::string wpisHistorii(std::string_view data, Status s);

}  // namespace serwis

#endif  // SERWIS_DOMENA_HPP

// ============================================================================
//  testy.cpp — lekkie testy jednostkowe bez zewnetrznych bibliotek.
//  Sprawdzaja logike (Serwis), trwalosc (CsvRepozytorium) i kodowanie (tekst).
//  Uruchomienie: make test
// ============================================================================
#include "domena.hpp"
#include "repozytorium.hpp"
#include "serwis.hpp"
#include "tekst.hpp"

#include <cstdio>
#include <iostream>
#include <string>

using namespace serwis;

// Prosty licznik + makro asercji z czytelnym komunikatem i miejscem bledu.
namespace {
int g_ile = 0;

void sprawdz(bool warunek, const char* opis, const char* plik, int linia) {
    ++g_ile;
    if (!warunek) {
        std::cerr << "  X BLAD: " << opis << "  (" << plik << ":" << linia << ")\n";
        std::exit(1);
    }
}
}  // namespace

#define SPRAWDZ(w) sprawdz((w), #w, __FILE__, __LINE__)

static void testDodawanieINumeracja() {
    Serwis s;
    int a = s.dodaj("Anna", "600", "Laptop", "nie dziala", Priorytet::Normalny);
    int b = s.dodaj("Bartek", "700", "Telefon", "ekran", Priorytet::Pilny);
    SPRAWDZ(a == 1);
    SPRAWDZ(b == 2);
    SPRAWDZ(s.wszystkie().size() == 2);
    SPRAWDZ(s.znajdz(1) != nullptr);
    SPRAWDZ(s.znajdz(999) == nullptr);
    // Nowo dodane zlecenie ma jeden wpis historii (status poczatkowy).
    SPRAWDZ(s.znajdz(1)->historia.size() == 1);
}

static void testNumeracjaZDanych() {
    // nastepneId musi byc wieksze od najwyzszego wczytanego id.
    std::vector<Zlecenie> dane;
    Zlecenie z; z.id = 41; z.klient = "X"; z.data = "2026-01-01"; dane.push_back(z);
    Serwis s(std::move(dane));
    int nowe = s.dodaj("Y", "1", "AGD", "opis", Priorytet::Niski);
    SPRAWDZ(nowe == 42);
}

static void testZmianyStatusuIHistorii() {
    Serwis s;
    s.dodaj("Anna", "600", "Laptop", "nie dziala", Priorytet::Normalny);
    SPRAWDZ(s.zmienStatus(1, Status::Gotowe) == true);
    SPRAWDZ(s.zmienStatus(123, Status::Gotowe) == false);   // brak takiego id
    SPRAWDZ(s.znajdz(1)->status == Status::Gotowe);
    SPRAWDZ(s.znajdz(1)->historia.size() == 2);              // przybyl wpis
}

static void testKosztIUsuwanie() {
    Serwis s;
    s.dodaj("Anna", "600", "Laptop", "x", Priorytet::Normalny);
    SPRAWDZ(s.ustawKoszt(1, 199.99) == true);
    SPRAWDZ(s.ustawKoszt(2, 50.0) == false);
    SPRAWDZ(s.znajdz(1)->koszt == 199.99);
    SPRAWDZ(s.usun(1) == true);
    SPRAWDZ(s.usun(1) == false);
    SPRAWDZ(s.pusty());
}

static void testPrzychodIStatystyki() {
    Serwis s;
    s.dodaj("A", "1", "L", "x", Priorytet::Pilny);
    s.dodaj("B", "2", "T", "y", Priorytet::Normalny);
    s.dodaj("C", "3", "D", "z", Priorytet::Pilny);
    s.ustawKoszt(1, 100.0);
    s.ustawKoszt(2, 200.0);
    s.zmienStatus(1, Status::Wydane);   // tylko to liczy sie do przychodu
    s.zmienStatus(2, Status::Gotowe);

    Statystyki st = s.statystyki();
    SPRAWDZ(st.lacznie == 3);
    SPRAWDZ(st.pilne == 2);
    SPRAWDZ(st.przychod == 100.0);      // 200 nie jest jeszcze "Wydane"
    SPRAWDZ(st.wgStatusu[indeks(Status::Wydane)] == 1);
    SPRAWDZ(st.wgStatusu[indeks(Status::Gotowe)] == 1);
    SPRAWDZ(st.wgStatusu[indeks(Status::Przyjete)] == 1);
}

static void testWyszukiwanieISortowanie() {
    Serwis s;
    s.dodaj("Jan Kowalski", "1", "L", "x", Priorytet::Normalny);
    s.dodaj("anna nowak",   "2", "T", "y", Priorytet::Normalny);
    s.ustawKoszt(1, 50.0);
    s.ustawKoszt(2, 300.0);

    SPRAWDZ(s.szukajKlient("KOWAL").size() == 1);   // bez rozroznienia wielkosci
    SPRAWDZ(s.szukajKlient("nowak").size() == 1);
    SPRAWDZ(s.szukajKlient("brak").empty());

    auto wg = s.wedlugKosztu();                     // malejaco po koszcie
    SPRAWDZ(wg.size() == 2);
    SPRAWDZ(wg[0]->koszt >= wg[1]->koszt);
    SPRAWDZ(wg[0]->id == 2);
}

static void testCsvRoundTrip() {
    const std::string plik = "test_zlecenia.csv";
    CsvRepozytorium repo;

    Serwis s;
    // Pole z ';' i '\' sprawdza kodowanie/ucieczke.
    s.dodaj("Nowak; Sp. z o.o.", "600\\100", "Laptop Dell", "haslo: a;b", Priorytet::Pilny);
    s.ustawKoszt(1, 320.50);
    s.zmienStatus(1, Status::Wydane);

    SPRAWDZ(repo.zapisz(plik, s.wszystkie()) == true);

    std::vector<Zlecenie> wczyt = repo.wczytaj(plik);
    SPRAWDZ(wczyt.size() == 1);
    const Zlecenie& z = wczyt[0];
    SPRAWDZ(z.id == 1);
    SPRAWDZ(z.klient == "Nowak; Sp. z o.o.");     // ';' odtworzony
    SPRAWDZ(z.telefon == "600\\100");             // '\' odtworzony
    SPRAWDZ(z.opis == "haslo: a;b");
    SPRAWDZ(z.status == Status::Wydane);
    SPRAWDZ(z.priorytet == Priorytet::Pilny);
    SPRAWDZ(z.koszt == 320.50);
    SPRAWDZ(z.historia.size() == 2);              // Przyjete + Wydane

    // Serwis zbudowany z wczytanych danych kontynuuje numeracje.
    Serwis s2(std::move(wczyt));
    SPRAWDZ(s2.dodaj("Kolejny", "1", "X", "y", Priorytet::Niski) == 2);

    std::remove(plik.c_str());
}

static void testWczytajBrakPliku() {
    CsvRepozytorium repo;
    auto dane = repo.wczytaj("nie_istnieje_12345.csv");
    SPRAWDZ(dane.empty());                         // brak pliku != blad
}

static void testTekstUcieczka() {
    using namespace serwis::tekst;
    std::string s = "a;b\\c\nd";
    SPRAWDZ(unescape(escape(s)) == s);             // round-trip
    auto p = podziel("a;b;c", ';');
    SPRAWDZ(p.size() == 3 && p[1] == "b");
    SPRAWDZ(podziel("a\\;b;c", ';').size() == 2);  // ucieczka chroni separator
    SPRAWDZ(naMale("AbC") == "abc");
    SPRAWDZ(dopelnij("xy", 4) == "xy  ");
    SPRAWDZ(skroc("abcdef", 4) == "abc.");
}

int main() {
    testDodawanieINumeracja();
    testNumeracjaZDanych();
    testZmianyStatusuIHistorii();
    testKosztIUsuwanie();
    testPrzychodIStatystyki();
    testWyszukiwanieISortowanie();
    testCsvRoundTrip();
    testWczytajBrakPliku();
    testTekstUcieczka();

    std::cout << "  Wszystkie testy przeszly (" << g_ile << " asercji).\n";
    return 0;
}

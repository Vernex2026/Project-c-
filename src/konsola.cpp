// ============================================================================
//  konsola.cpp — implementacja warstwy UI (render, wejscie, petla menu).
// ============================================================================
#include "konsola.hpp"
#include "tekst.hpp"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace serwis {
namespace {

constexpr const char* RESET = "\033[0m";

// Szerokosci kolumn tabeli (nazwane, zamiast "magicznych" liczb).
constexpr int W_ID = 8, W_KLIENT = 16, W_URZ = 14, W_PRIO = 9, W_STATUS = 11, W_KOSZT = 9;
constexpr int SZEROKOSC_LINII = W_ID + W_KLIENT + W_URZ + W_PRIO + W_STATUS + W_KOSZT + 1;

// --- Wejscie z walidacja ----------------------------------------------------

void wyczyscWejscie() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int wczytajInt(const std::string& monit) {
    int w;
    while (std::cout << monit, !(std::cin >> w)) {
        std::cout << "  !! Podaj liczbe calkowita.\n";
        wyczyscWejscie();
    }
    wyczyscWejscie();
    return w;
}

double wczytajDouble(const std::string& monit) {
    double w;
    while (std::cout << monit, !(std::cin >> w)) {
        std::cout << "  !! Podaj liczbe (np. 149.99).\n";
        wyczyscWejscie();
    }
    wyczyscWejscie();
    return w;
}

std::string wczytajTekst(const std::string& monit) {
    std::cout << monit;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

Priorytet wczytajPriorytet() {
    std::cout << "  Priorytet: 1=Niski  2=Normalny  3=Pilny\n";
    switch (wczytajInt("  Wybor: ")) {
        case 1:  return Priorytet::Niski;
        case 3:  return Priorytet::Pilny;
        default: return Priorytet::Normalny;
    }
}

// --- Render -----------------------------------------------------------------

void naglowekTabeli() {
    std::cout << "  " << std::left
              << std::setw(W_ID)     << "ID"
              << std::setw(W_KLIENT) << "Klient"
              << std::setw(W_URZ)    << "Urzadzenie"
              << std::setw(W_PRIO)   << "Prio"
              << std::setw(W_STATUS) << "Status"
              << std::right << std::setw(W_KOSZT + 1) << "Koszt" << "\n"
              << "  " << std::string(SZEROKOSC_LINII, '-') << "\n";
}

void wierszTabeli(const Zlecenie& z) {
    std::cout << "  " << std::left
              << std::setw(W_ID)     << ("SRV-" + std::to_string(z.id))
              << std::setw(W_KLIENT) << tekst::skroc(z.klient, W_KLIENT - 1)
              << std::setw(W_URZ)    << tekst::skroc(z.urzadzenie, W_URZ - 1)
              << std::setw(W_PRIO)   << priorytetNaTekst(z.priorytet)
              // Kolor liczy sie do szerokosci pola, wiec dopelniamy tekst recznie.
              << kolorStatus(z.status) << tekst::dopelnij(statusNaTekst(z.status), W_STATUS) << RESET
              << std::right << std::setw(W_KOSZT) << std::fixed << std::setprecision(2)
              << z.koszt << "\n";
}

// Render listy wskaznikow na zlecenia (wspolny dla listy/wyszukiwania).
void pokazTabele(const std::vector<const Zlecenie*>& widok) {
    if (widok.empty()) { std::cout << "  (brak zlecen)\n"; return; }
    naglowekTabeli();
    for (const Zlecenie* z : widok) wierszTabeli(*z);
}

void pokazStatystyki(const Statystyki& s) {
    std::cout << "  Zlecen lacznie : " << s.lacznie << "\n"
              << "  Przyjete       : " << s.wgStatusu[indeks(Status::Przyjete)] << "\n"
              << "  W trakcie      : " << s.wgStatusu[indeks(Status::WTrakcie)] << "\n"
              << "  Gotowe         : " << s.wgStatusu[indeks(Status::Gotowe)]   << "\n"
              << "  Wydane         : " << s.wgStatusu[indeks(Status::Wydane)]   << "\n"
              << "  Pilne          : " << s.pilne << "\n"
              << "  Przychod (wydane): " << std::fixed << std::setprecision(2)
              << s.przychod << " zl\n";
}

void banner() {
    std::cout << "\n  ============================================================\n"
              <<   "   SerwisPro Mini  |  konsolowy system zlecen serwisowych\n"
              <<   "   Pelna wersja: serwispro.vernex.pl\n"
              <<   "   Kod: github.com/Vernex2026/SerwisPRO\n"
              <<   "  ============================================================\n";
}

void menu() {
    std::cout << "\n  --- MENU ---------------------------------------------------\n"
              <<   "   1. Dodaj zlecenie\n"
              <<   "   2. Lista zlecen\n"
              <<   "   3. Lista wg kosztu (malejaco)\n"
              <<   "   4. Zmien status\n"
              <<   "   5. Szukaj po ID (z historia)\n"
              <<   "   6. Szukaj po kliencie\n"
              <<   "   7. Ustaw koszt naprawy\n"
              <<   "   8. Gotowe do odbioru\n"
              <<   "   9. Usun zlecenie\n"
              <<   "  10. Statystyki\n"
              <<   "  11. Zapisz do pliku (CSV)\n"
              <<   "   0. Wyjscie (autozapis)\n"
              <<   "  ------------------------------------------------------------\n";
}

// Wspolny komunikat "znaleziono / brak zlecenia".
void informujWynik(bool ok, int id, const std::string& coZrobiono) {
    if (ok) std::cout << "  OK: " << coZrobiono << " SRV-" << id << "\n";
    else    std::cout << "  !! Brak zlecenia SRV-" << id << "\n";
}

}  // namespace

// --- Aplikacja --------------------------------------------------------------

Aplikacja::Aplikacja(Serwis& serwis, CsvRepozytorium& repo, std::string sciezka)
    : serwis_(serwis), repo_(repo), sciezka_(std::move(sciezka)) {}

void Aplikacja::obsluzDodaj() {
    std::string klient = wczytajTekst("  Klient: ");
    std::string tel    = wczytajTekst("  Telefon: ");
    std::string urz    = wczytajTekst("  Urzadzenie: ");
    std::string opis   = wczytajTekst("  Opis usterki: ");
    Priorytet   prio   = wczytajPriorytet();
    int id = serwis_.dodaj(klient, tel, urz, opis, prio);
    std::cout << "  OK: utworzono zlecenie SRV-" << id << "\n";
}

void Aplikacja::obsluzListe(bool wedlugKosztu) {
    if (wedlugKosztu) {
        pokazTabele(serwis_.wedlugKosztu());
    } else {
        std::vector<const Zlecenie*> widok;
        for (const auto& z : serwis_.wszystkie()) widok.push_back(&z);
        pokazTabele(widok);
    }
}

void Aplikacja::obsluzZmianeStatusu() {
    int id = wczytajInt("  Podaj ID: ");
    std::cout << "  Status: 1=Przyjete  2=W trakcie  3=Gotowe  4=Wydane\n";
    Status s;
    switch (wczytajInt("  Wybor: ")) {
        case 1: s = Status::Przyjete; break;
        case 2: s = Status::WTrakcie; break;
        case 3: s = Status::Gotowe;   break;
        case 4: s = Status::Wydane;   break;
        default: std::cout << "  !! Nieznany status.\n"; return;
    }
    bool ok = serwis_.zmienStatus(id, s);
    if (ok) std::cout << "  OK: SRV-" << id << " -> " << statusNaTekst(s) << "\n";
    else    std::cout << "  !! Brak zlecenia SRV-" << id << "\n";
}

void Aplikacja::obsluzSzukajId() {
    int id = wczytajInt("  Podaj ID: ");
    const Zlecenie* z = serwis_.znajdz(id);
    if (!z) { std::cout << "  !! Brak zlecenia SRV-" << id << "\n"; return; }
    pokazTabele({z});
    std::cout << "  Historia:\n";
    for (const auto& h : z->historia) std::cout << "    - " << h << "\n";
}

void Aplikacja::obsluzSzukajKlient() {
    std::string fraza = wczytajTekst("  Klient (fragment): ");
    auto wynik = serwis_.szukajKlient(fraza);
    if (wynik.empty()) std::cout << "  (brak dopasowan dla: " << fraza << ")\n";
    else               pokazTabele(wynik);
}

void Aplikacja::obsluzUstawKoszt() {
    int id = wczytajInt("  Podaj ID: ");
    double k = wczytajDouble("  Koszt (zl): ");
    if (serwis_.ustawKoszt(id, k))
        std::cout << "  OK: koszt SRV-" << id << " = "
                  << std::fixed << std::setprecision(2) << k << " zl\n";
    else
        std::cout << "  !! Brak zlecenia SRV-" << id << "\n";
}

void Aplikacja::obsluzGotowe() {
    auto wynik = serwis_.gotoweDoOdbioru();
    if (wynik.empty()) { std::cout << "  (nic nie czeka na odbior)\n"; return; }
    std::cout << "  Gotowe do odbioru:\n";
    for (const Zlecenie* z : wynik)
        std::cout << "    SRV-" << z->id << "  " << z->klient
                  << "  tel: " << z->telefon << "  (" << z->urzadzenie << ")\n";
}

void Aplikacja::obsluzUsun() {
    int id = wczytajInt("  Podaj ID do usuniecia: ");
    informujWynik(serwis_.usun(id), id, "usunieto");
}

void Aplikacja::obsluzStatystyki() {
    pokazStatystyki(serwis_.statystyki());
}

bool Aplikacja::obsluzZapis() {
    return repo_.zapisz(sciezka_, serwis_.wszystkie());
}

void Aplikacja::uruchom() {
    banner();
    std::cout << "  (Wczytano dane z pliku " << sciezka_ << ", jesli istnial.)\n";

    for (bool dziala = true; dziala; ) {
        menu();
        int wybor = wczytajInt("  Twoj wybor: ");
        std::cout << "\n";
        switch (wybor) {
            case 1:  obsluzDodaj();            break;
            case 2:  obsluzListe(false);       break;
            case 3:  obsluzListe(true);        break;
            case 4:  obsluzZmianeStatusu();    break;
            case 5:  obsluzSzukajId();         break;
            case 6:  obsluzSzukajKlient();     break;
            case 7:  obsluzUstawKoszt();       break;
            case 8:  obsluzGotowe();           break;
            case 9:  obsluzUsun();             break;
            case 10: obsluzStatystyki();       break;
            case 11: std::cout << (obsluzZapis()
                        ? "  OK: zapisano do " + sciezka_ + "\n"
                        : "  !! Blad zapisu pliku.\n");
                     break;
            case 0:  dziala = false;           break;
            default: std::cout << "  !! Nieznana opcja.\n";
        }
    }

    std::cout << (obsluzZapis()
        ? "\n  Autozapis OK -> " + sciezka_ + ". Do zobaczenia!\n"
        : "\n  !! Autozapis nieudany.\n");
}

}  // namespace serwis

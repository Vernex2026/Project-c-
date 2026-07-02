// ============================================================================
//  SerwisPro Mini  —  konsolowy system zarzadzania zleceniami serwisowymi
// ----------------------------------------------------------------------------
//  Wersja edukacyjna (projekt na zajecia).
//  Pelna, produkcyjna wersja dziala pod adresem:
//      Strona : https://serwispro.vernex.pl
//      Kod    : https://github.com/Vernex2026/SerwisPRO
// ----------------------------------------------------------------------------
//  Kompilacja:  g++ -std=c++17 -Wall -O2 serwis.cpp -o serwis
//  Uruchomienie: ./serwis        (Windows: serwis.exe)
//
//  Pokazuje: struct + class, dwa enum class, std::vector (tez wektor w
//  zleceniu), STL (remove_if / sort / accumulate / count_if + lambdy),
//  obsluge plikow (fstream / stringstream), date (ctime), kolory ANSI,
//  walidacje wejscia i formatowanie (iomanip).
// ============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <limits>
#include <ctime>

// ---------------------------------------------------------------------------
//  Typy wyliczeniowe
// ---------------------------------------------------------------------------
enum class Status    { Przyjete, WTrakcie, Gotowe, Wydane };
enum class Priorytet { Niski, Normalny, Pilny };

// Kolory ANSI (dzialaja w Linux/macOS i nowym terminalu Windows).
constexpr const char* RESET = "\033[0m";

const char* kolorStatus(Status s) {
    switch (s) {
        case Status::Przyjete: return "\033[36m";  // cyan
        case Status::WTrakcie: return "\033[33m";  // zolty
        case Status::Gotowe:   return "\033[32m";  // zielony
        case Status::Wydane:   return "\033[90m";  // szary
    }
    return RESET;
}

std::string statusNaTekst(Status s) {
    switch (s) {
        case Status::Przyjete: return "Przyjete";
        case Status::WTrakcie: return "W trakcie";
        case Status::Gotowe:   return "Gotowe";
        case Status::Wydane:   return "Wydane";
    }
    return "?";
}
Status tekstNaStatus(const std::string& t) {
    if (t == "W trakcie") return Status::WTrakcie;
    if (t == "Gotowe")    return Status::Gotowe;
    if (t == "Wydane")    return Status::Wydane;
    return Status::Przyjete;
}

std::string priorytetNaTekst(Priorytet p) {
    switch (p) {
        case Priorytet::Niski:  return "Niski";
        case Priorytet::Pilny:  return "PILNY";
        default:                return "Normalny";
    }
}
Priorytet tekstNaPriorytet(const std::string& t) {
    if (t == "Niski") return Priorytet::Niski;
    if (t == "PILNY") return Priorytet::Pilny;
    return Priorytet::Normalny;
}

// Dzisiejsza data w formacie YYYY-MM-DD.
std::string dzisiaj() {
    std::time_t t = std::time(nullptr);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}

// ---------------------------------------------------------------------------
//  Pojedyncze zlecenie serwisowe
// ---------------------------------------------------------------------------
struct Zlecenie {
    int         id;
    std::string klient;
    std::string telefon;
    std::string urzadzenie;
    std::string opis;
    Status      status    = Status::Przyjete;
    Priorytet   priorytet = Priorytet::Normalny;
    double      koszt     = 0.0;
    std::string data;                       // data przyjecia
    std::vector<std::string> historia;      // log zmian ("data - status")
};

// ---------------------------------------------------------------------------
//  Male pomocnicze funkcje formatujace / tekstowe
// ---------------------------------------------------------------------------
namespace pom {
    std::string naMale(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return s;
    }
    std::string skroc(const std::string& s, std::size_t maks) {
        return s.size() <= maks ? s
             : s.substr(0, maks > 1 ? maks - 1 : 1) + ".";
    }
    std::string dopelnij(std::string s, std::size_t w) {   // wyrownanie do lewej
        if (s.size() < w) s.append(w - s.size(), ' ');
        return s;
    }
    // Ucieczka znakow specjalnych przy zapisie CSV (separator to ';').
    std::string escape(const std::string& s) {
        std::string out;
        for (char c : s) {
            if      (c == ';')  out += "\\;";
            else if (c == '\n') out += "\\n";
            else if (c == '\\') out += "\\\\";
            else                out += c;
        }
        return out;
    }
    std::string unescape(const std::string& s) {
        std::string out;
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                char n = s[++i];
                out += (n == 'n') ? '\n' : n;
            } else out += s[i];
        }
        return out;
    }
    // Podzial wiersza po separatorze z poszanowaniem ucieczek '\\x'.
    std::vector<std::string> podziel(const std::string& s, char sep) {
        std::vector<std::string> pola;
        std::string b;
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) { b += s[i]; b += s[++i]; }
            else if (s[i] == sep) { pola.push_back(b); b.clear(); }
            else                  b += s[i];
        }
        pola.push_back(b);
        return pola;
    }
    // Zlaczenie / rozbicie listy tekstow jednym znakiem (do historii w CSV).
    std::string zlacz(const std::vector<std::string>& v, char sep) {
        std::string out;
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (i) out += sep;
            out += v[i];
        }
        return out;
    }
}

// ---------------------------------------------------------------------------
//  Klasa zarzadzajaca kolekcja zlecen
// ---------------------------------------------------------------------------
class Serwis {
public:
    static constexpr const char* PLIK = "zlecenia.csv";

    // Dodanie nowego zlecenia; zwraca nadany numer ID.
    int dodaj(const std::string& klient, const std::string& telefon,
              const std::string& urzadzenie, const std::string& opis,
              Priorytet priorytet) {
        Zlecenie z;
        z.id         = nastepneId++;
        z.klient     = klient;
        z.telefon    = telefon;
        z.urzadzenie = urzadzenie;
        z.opis       = opis;
        z.priorytet  = priorytet;
        z.data       = dzisiaj();
        z.historia.push_back(z.data + " - " + statusNaTekst(z.status));
        zlecenia.push_back(z);
        return z.id;
    }

    // Lista wszystkich zlecen (opcjonalnie posortowana po koszcie malejaco).
    void lista(bool wgKosztu = false) const {
        if (zlecenia.empty()) { std::cout << "  (brak zlecen)\n"; return; }
        std::vector<Zlecenie> kopia = zlecenia;      // sortujemy kopie
        if (wgKosztu)
            std::sort(kopia.begin(), kopia.end(),
                      [](const Zlecenie& a, const Zlecenie& b) { return a.koszt > b.koszt; });
        naglowek();
        for (const auto& z : kopia) wiersz(z);
    }

    void zmienStatus(int id, Status s) {
        if (Zlecenie* z = znajdz(id)) {
            z->status = s;
            z->historia.push_back(dzisiaj() + " - " + statusNaTekst(s));
            std::cout << "  OK: SRV-" << id << " -> " << statusNaTekst(s) << "\n";
        } else brak(id);
    }

    void ustawKoszt(int id, double k) {
        if (Zlecenie* z = znajdz(id)) {
            z->koszt = k;
            std::cout << "  OK: koszt SRV-" << id << " = "
                      << std::fixed << std::setprecision(2) << k << " zl\n";
        } else brak(id);
    }

    void szukajPoId(int id) const {
        const Zlecenie* z = znajdz(id);
        if (!z) { brak(id); return; }
        naglowek();
        wiersz(*z);
        std::cout << "  Historia:\n";
        for (const auto& h : z->historia) std::cout << "    - " << h << "\n";
    }

    void szukajPoKliencie(const std::string& fraza) const {
        std::string szukane = pom::naMale(fraza);
        bool cos = false;
        for (const auto& z : zlecenia)
            if (pom::naMale(z.klient).find(szukane) != std::string::npos) {
                if (!cos) naglowek();
                wiersz(z);
                cos = true;
            }
        if (!cos) std::cout << "  (brak dopasowan dla: " << fraza << ")\n";
    }

    void usun(int id) {
        auto it = std::remove_if(zlecenia.begin(), zlecenia.end(),
                                 [id](const Zlecenie& z) { return z.id == id; });
        if (it == zlecenia.end()) { brak(id); return; }
        zlecenia.erase(it, zlecenia.end());
        std::cout << "  OK: usunieto SRV-" << id << "\n";
    }

    void gotoweDoOdbioru() const {
        bool cos = false;
        for (const auto& z : zlecenia)
            if (z.status == Status::Gotowe) {
                if (!cos) std::cout << "  Gotowe do odbioru:\n";
                std::cout << "    SRV-" << z.id << "  " << z.klient
                          << "  tel: " << z.telefon << "  (" << z.urzadzenie << ")\n";
                cos = true;
            }
        if (!cos) std::cout << "  (nic nie czeka na odbior)\n";
    }

    void statystyki() const {
        int licz[4] = {0, 0, 0, 0};
        for (const auto& z : zlecenia) licz[static_cast<int>(z.status)]++;
        long pilne = std::count_if(zlecenia.begin(), zlecenia.end(),
            [](const Zlecenie& z) { return z.priorytet == Priorytet::Pilny; });
        std::cout << "  Zlecen lacznie : " << zlecenia.size() << "\n"
                  << "  Przyjete       : " << licz[0] << "\n"
                  << "  W trakcie      : " << licz[1] << "\n"
                  << "  Gotowe         : " << licz[2] << "\n"
                  << "  Wydane         : " << licz[3] << "\n"
                  << "  Pilne          : " << pilne  << "\n"
                  << "  Przychod (wydane): " << std::fixed << std::setprecision(2)
                  << sumaPrzychodu() << " zl\n";
    }

    // Laczny przychod = suma kosztow zlecen wydanych.
    double sumaPrzychodu() const {
        return std::accumulate(zlecenia.begin(), zlecenia.end(), 0.0,
            [](double suma, const Zlecenie& z) {
                return suma + (z.status == Status::Wydane ? z.koszt : 0.0);
            });
    }

    // --- Trwalosc (CSV) -------------------------------------------------
    bool zapisz() const {
        std::ofstream out(PLIK);
        if (!out) return false;
        out << nastepneId << "\n";
        for (const auto& z : zlecenia) {
            std::vector<std::string> h;
            for (const auto& e : z.historia) h.push_back(pom::escape(e));
            out << z.id << ';' << pom::escape(z.klient) << ';' << pom::escape(z.telefon) << ';'
                << pom::escape(z.urzadzenie) << ';' << pom::escape(z.opis) << ';'
                << statusNaTekst(z.status) << ';'
                << std::fixed << std::setprecision(2) << z.koszt << ';'
                << z.data << ';' << priorytetNaTekst(z.priorytet) << ';'
                << pom::zlacz(h, '|') << "\n";
        }
        return true;
    }

    void wczytaj() {
        std::ifstream in(PLIK);
        if (!in) return;
        std::string linia;
        if (std::getline(in, linia)) {
            std::stringstream ss(linia);
            ss >> nastepneId;
            if (nastepneId < 1) nastepneId = 1;
        }
        zlecenia.clear();
        while (std::getline(in, linia)) {
            if (linia.empty()) continue;
            auto p = pom::podziel(linia, ';');
            if (p.size() < 7) continue;                 // za malo pol -> pomijamy
            Zlecenie z;
            z.id         = std::stoi(p[0]);
            z.klient     = pom::unescape(p[1]);
            z.telefon    = pom::unescape(p[2]);
            z.urzadzenie = pom::unescape(p[3]);
            z.opis       = pom::unescape(p[4]);
            z.status     = tekstNaStatus(p[5]);
            z.koszt      = std::stod(p[6]);
            z.data       = p.size() > 7 ? p[7] : dzisiaj();
            z.priorytet  = p.size() > 8 ? tekstNaPriorytet(p[8]) : Priorytet::Normalny;
            if (p.size() > 9 && !p[9].empty())
                for (const auto& e : pom::podziel(p[9], '|'))
                    z.historia.push_back(pom::unescape(e));
            if (z.historia.empty())
                z.historia.push_back(z.data + " - " + statusNaTekst(z.status));
            zlecenia.push_back(z);
        }
    }

private:
    std::vector<Zlecenie> zlecenia;
    int                   nastepneId = 1;

    Zlecenie* znajdz(int id) {
        for (auto& z : zlecenia) if (z.id == id) return &z;
        return nullptr;
    }
    const Zlecenie* znajdz(int id) const {
        for (const auto& z : zlecenia) if (z.id == id) return &z;
        return nullptr;
    }
    static void brak(int id) { std::cout << "  !! Brak zlecenia SRV-" << id << "\n"; }

    static void naglowek() {
        std::cout << "  " << std::left
                  << std::setw(8)  << "ID"
                  << std::setw(16) << "Klient"
                  << std::setw(14) << "Urzadzenie"
                  << std::setw(9)  << "Prio"
                  << std::setw(11) << "Status"
                  << std::right << std::setw(10) << "Koszt" << "\n"
                  << "  " << std::string(66, '-') << "\n";
    }
    static void wiersz(const Zlecenie& z) {
        std::cout << "  " << std::left
                  << std::setw(8)  << ("SRV-" + std::to_string(z.id))
                  << std::setw(16) << pom::skroc(z.klient, 15)
                  << std::setw(14) << pom::skroc(z.urzadzenie, 13)
                  << std::setw(9)  << priorytetNaTekst(z.priorytet)
                  << kolorStatus(z.status) << pom::dopelnij(statusNaTekst(z.status), 11) << RESET
                  << std::right << std::setw(9) << std::fixed << std::setprecision(2)
                  << z.koszt << "\n";
    }
};

// ===========================================================================
//  Warstwa interfejsu (menu konsolowe)
// ===========================================================================

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

void akcjaDodaj(Serwis& s) {
    std::string klient = wczytajTekst("  Klient: ");
    std::string tel    = wczytajTekst("  Telefon: ");
    std::string urz    = wczytajTekst("  Urzadzenie: ");
    std::string opis   = wczytajTekst("  Opis usterki: ");
    std::cout << "  Priorytet: 1=Niski  2=Normalny  3=Pilny\n";
    int p = wczytajInt("  Wybor: ");
    Priorytet prio = (p == 1) ? Priorytet::Niski
                   : (p == 3) ? Priorytet::Pilny
                              : Priorytet::Normalny;
    int id = s.dodaj(klient, tel, urz, opis, prio);
    std::cout << "  OK: utworzono zlecenie SRV-" << id << "\n";
}

void akcjaZmienStatus(Serwis& s) {
    int id = wczytajInt("  Podaj ID: ");
    std::cout << "  Status: 1=Przyjete  2=W trakcie  3=Gotowe  4=Wydane\n";
    switch (wczytajInt("  Wybor: ")) {
        case 1: s.zmienStatus(id, Status::Przyjete); break;
        case 2: s.zmienStatus(id, Status::WTrakcie); break;
        case 3: s.zmienStatus(id, Status::Gotowe);   break;
        case 4: s.zmienStatus(id, Status::Wydane);   break;
        default: std::cout << "  !! Nieznany status.\n";
    }
}

int main() {
    Serwis serwis;
    serwis.wczytaj();

    banner();
    std::cout << "  (Wczytano dane z pliku " << Serwis::PLIK << ", jesli istnial.)\n";

    for (bool dziala = true; dziala; ) {
        menu();
        int wybor = wczytajInt("  Twoj wybor: ");
        std::cout << "\n";
        switch (wybor) {
            case 1:  akcjaDodaj(serwis);       break;
            case 2:  serwis.lista();           break;
            case 3:  serwis.lista(true);       break;
            case 4:  akcjaZmienStatus(serwis); break;
            case 5:  serwis.szukajPoId(wczytajInt("  Podaj ID: ")); break;
            case 6:  serwis.szukajPoKliencie(wczytajTekst("  Klient (fragment): ")); break;
            case 7: {
                int id = wczytajInt("  Podaj ID: ");
                serwis.ustawKoszt(id, wczytajDouble("  Koszt (zl): "));
                break;
            }
            case 8:  serwis.gotoweDoOdbioru(); break;
            case 9:  serwis.usun(wczytajInt("  Podaj ID do usuniecia: ")); break;
            case 10: serwis.statystyki();      break;
            case 11: std::cout << (serwis.zapisz()
                        ? "  OK: zapisano do " + std::string(Serwis::PLIK) + "\n"
                        : "  !! Blad zapisu pliku.\n");
                     break;
            case 0:  dziala = false;           break;
            default: std::cout << "  !! Nieznana opcja.\n";
        }
    }

    std::cout << (serwis.zapisz()
        ? "\n  Autozapis OK -> " + std::string(Serwis::PLIK) + ". Do zobaczenia!\n"
        : "\n  !! Autozapis nieudany.\n");
    return 0;
}

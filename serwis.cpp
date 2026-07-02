// ============================================================================
//  SerwisPro Mini  —  konsolowy system zarzadzania zleceniami serwisowymi
// ----------------------------------------------------------------------------
//  Wersja edukacyjna (projekt na zajecia) autorstwa Vernex2026.
//  Pelna, produkcyjna wersja dziala pod adresem:
//      Strona : https://serwispro.vernex.pl
//      Kod    : https://github.com/Vernex2026/SerwisPRO
// ----------------------------------------------------------------------------
//  Kompilacja:  g++ -std=c++17 -Wall -O2 serwis.cpp -o serwis
//  Uruchomienie: ./serwis        (Windows: serwis.exe)
//
//  Co pokazuje technicznie: struct + class, enum class, std::vector, STL
//  (remove_if / sort / accumulate + lambdy), obsluga plikow (fstream /
//  stringstream), walidacja wejscia oraz formatowanie (iomanip).
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

// ---------------------------------------------------------------------------
//  Status zlecenia (enum class -> bezpieczny typ, brak niejawnych konwersji)
// ---------------------------------------------------------------------------
enum class Status { Przyjete, WTrakcie, Gotowe, Wydane };

// Zamiana statusu na czytelny tekst (do wyswietlania i zapisu CSV).
std::string statusNaTekst(Status s) {
    switch (s) {
        case Status::Przyjete: return "Przyjete";
        case Status::WTrakcie: return "W trakcie";
        case Status::Gotowe:   return "Gotowe";
        case Status::Wydane:   return "Wydane";
    }
    return "Nieznany";
}

// Odwrotna zamiana (przy wczytywaniu z pliku CSV).
Status tekstNaStatus(const std::string& t) {
    if (t == "W trakcie") return Status::WTrakcie;
    if (t == "Gotowe")    return Status::Gotowe;
    if (t == "Wydane")    return Status::Wydane;
    return Status::Przyjete;
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
    Status      status;
    double      koszt;
};

// ---------------------------------------------------------------------------
//  Klasa zarzadzajaca kolekcja zlecen
// ---------------------------------------------------------------------------
class Serwis {
public:
    static constexpr char PLIK[] = "zlecenia.csv";

    // Dodanie nowego zlecenia; zwraca nadany numer ID.
    int dodaj(const std::string& klient, const std::string& telefon,
              const std::string& urzadzenie, const std::string& opis) {
        Zlecenie z;
        z.id         = nastepneId++;
        z.klient     = klient;
        z.telefon    = telefon;
        z.urzadzenie = urzadzenie;
        z.opis       = opis;
        z.status     = Status::Przyjete;
        z.koszt      = 0.0;
        zlecenia.push_back(z);
        return z.id;
    }

    // Lista wszystkich zlecen (opcjonalnie posortowana po koszcie malejaco).
    void lista(bool wgKosztu = false) const {
        if (zlecenia.empty()) {
            std::cout << "  (brak zlecen)\n";
            return;
        }
        std::vector<Zlecenie> kopia = zlecenia;   // sortujemy kopie, nie oryginal
        if (wgKosztu) {
            std::sort(kopia.begin(), kopia.end(),
                      [](const Zlecenie& a, const Zlecenie& b) { return a.koszt > b.koszt; });
        }
        naglowekTabeli();
        for (const auto& z : kopia) wierszTabeli(z);
    }

    // Zmiana statusu zlecenia o danym ID.
    void zmienStatus(int id, Status s) {
        if (Zlecenie* z = znajdz(id)) {
            z->status = s;
            std::cout << "  OK: SRV-" << id << " -> " << statusNaTekst(s) << "\n";
        } else {
            std::cout << "  !! Brak zlecenia SRV-" << id << "\n";
        }
    }

    // Wyszukiwanie po ID.
    void szukajPoId(int id) const {
        if (const Zlecenie* z = znajdz(id)) {
            naglowekTabeli();
            wierszTabeli(*z);
        } else {
            std::cout << "  !! Brak zlecenia SRV-" << id << "\n";
        }
    }

    // Wyszukiwanie po (fragmencie) nazwiska klienta — bez rozroznienia wielkosci.
    void szukajPoKliencie(const std::string& fraza) const {
        std::string szukane = naMale(fraza);
        bool cos = false;
        for (const auto& z : zlecenia) {
            if (naMale(z.klient).find(szukane) != std::string::npos) {
                if (!cos) naglowekTabeli();
                wierszTabeli(z);
                cos = true;
            }
        }
        if (!cos) std::cout << "  (brak dopasowan dla: " << fraza << ")\n";
    }

    // Usuniecie zlecenia po ID (STL remove_if + lambda).
    void usun(int id) {
        auto it = std::remove_if(zlecenia.begin(), zlecenia.end(),
                                 [id](const Zlecenie& z) { return z.id == id; });
        if (it != zlecenia.end()) {
            zlecenia.erase(it, zlecenia.end());
            std::cout << "  OK: usunieto SRV-" << id << "\n";
        } else {
            std::cout << "  !! Brak zlecenia SRV-" << id << "\n";
        }
    }

    // Statystyki wg statusu.
    void statystyki() const {
        int licz[4] = {0, 0, 0, 0};
        for (const auto& z : zlecenia) licz[static_cast<int>(z.status)]++;
        std::cout << "  Zlecen lacznie : " << zlecenia.size() << "\n";
        std::cout << "  Przyjete       : " << licz[0] << "\n";
        std::cout << "  W trakcie      : " << licz[1] << "\n";
        std::cout << "  Gotowe         : " << licz[2] << "\n";
        std::cout << "  Wydane         : " << licz[3] << "\n";
        std::cout << "  Przychod (wydane): " << std::fixed << std::setprecision(2)
                  << sumaPrzychodu() << " zl\n";
    }

    // --- Akcenty (na wyzsza ocene) --------------------------------------

    // Ustawienie kosztu naprawy dla zlecenia o danym ID.
    void ustawKoszt(int id, double k) {
        if (Zlecenie* z = znajdz(id)) {
            z->koszt = k;
            std::cout << "  OK: koszt SRV-" << id << " = "
                      << std::fixed << std::setprecision(2) << k << " zl\n";
        } else {
            std::cout << "  !! Brak zlecenia SRV-" << id << "\n";
        }
    }

    // Lista tylko zlecen gotowych do odbioru (Status::Gotowe).
    void gotoweDoOdbioru() const {
        bool cos = false;
        for (const auto& z : zlecenia) {
            if (z.status == Status::Gotowe) {
                if (!cos) std::cout << "  Gotowe do odbioru:\n";
                std::cout << "    SRV-" << z.id << "  " << z.klient
                          << "  tel: " << z.telefon
                          << "  (" << z.urzadzenie << ")\n";
                cos = true;
            }
        }
        if (!cos) std::cout << "  (nic nie czeka na odbior)\n";
    }

    // Laczny przychod = suma kosztow zlecen juz wydanych (STL accumulate).
    double sumaPrzychodu() const {
        return std::accumulate(zlecenia.begin(), zlecenia.end(), 0.0,
            [](double suma, const Zlecenie& z) {
                return suma + (z.status == Status::Wydane ? z.koszt : 0.0);
            });
    }

    // --- Trwalosc (CSV) -------------------------------------------------

    // Zapis wszystkich zlecen do pliku CSV.
    bool zapisz() const {
        std::ofstream out(PLIK);
        if (!out) return false;
        out << nastepneId << "\n";                     // pierwszy wiersz: licznik ID
        for (const auto& z : zlecenia) {
            out << z.id << ';' << escape(z.klient) << ';' << escape(z.telefon) << ';'
                << escape(z.urzadzenie) << ';' << escape(z.opis) << ';'
                << statusNaTekst(z.status) << ';'
                << std::fixed << std::setprecision(2) << z.koszt << "\n";
        }
        return true;
    }

    // Wczytanie zlecen z pliku CSV (jesli istnieje).
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
            std::vector<std::string> pola = podziel(linia, ';');
            if (pola.size() < 7) continue;
            Zlecenie z;
            z.id         = std::stoi(pola[0]);
            z.klient     = unescape(pola[1]);
            z.telefon    = unescape(pola[2]);
            z.urzadzenie = unescape(pola[3]);
            z.opis       = unescape(pola[4]);
            z.status     = tekstNaStatus(pola[5]);
            z.koszt      = std::stod(pola[6]);
            zlecenia.push_back(z);
        }
    }

private:
    std::vector<Zlecenie> zlecenia;
    int                   nastepneId = 1;

    // Szukanie wskaznika na zlecenie o danym ID (wersja modyfikujaca i stala).
    Zlecenie* znajdz(int id) {
        for (auto& z : zlecenia) if (z.id == id) return &z;
        return nullptr;
    }
    const Zlecenie* znajdz(int id) const {
        for (const auto& z : zlecenia) if (z.id == id) return &z;
        return nullptr;
    }

    static void naglowekTabeli() {
        std::cout << "  " << std::left
                  << std::setw(8)  << "ID"
                  << std::setw(18) << "Klient"
                  << std::setw(16) << "Urzadzenie"
                  << std::setw(12) << "Status"
                  << std::right << std::setw(10) << "Koszt" << "\n";
        std::cout << "  " << std::string(62, '-') << "\n";
    }

    static void wierszTabeli(const Zlecenie& z) {
        std::cout << "  " << std::left
                  << std::setw(8)  << ("SRV-" + std::to_string(z.id))
                  << std::setw(18) << skroc(z.klient, 17)
                  << std::setw(16) << skroc(z.urzadzenie, 15)
                  << std::setw(12) << statusNaTekst(z.status)
                  << std::right << std::setw(9) << std::fixed << std::setprecision(2)
                  << z.koszt << " " << "\n";
    }

    // --- Pomocnicze -----------------------------------------------------

    static std::string naMale(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    static std::string skroc(const std::string& s, std::size_t maks) {
        if (s.size() <= maks) return s;
        return s.substr(0, maks > 1 ? maks - 1 : 1) + ".";
    }

    // Ucieczka separatora/nowej linii przy zapisie CSV.
    static std::string escape(const std::string& s) {
        std::string out;
        for (char c : s) {
            if (c == ';')      out += "\\;";
            else if (c == '\n') out += "\\n";
            else if (c == '\\') out += "\\\\";
            else                out += c;
        }
        return out;
    }

    static std::string unescape(const std::string& s) {
        std::string out;
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                char n = s[++i];
                if (n == 'n')      out += '\n';
                else               out += n;   // ';' lub '\\'
            } else {
                out += s[i];
            }
        }
        return out;
    }

    // Podzial wiersza CSV po separatorze, z poszanowaniem ucieczek '\;'.
    static std::vector<std::string> podziel(const std::string& s, char sep) {
        std::vector<std::string> pola;
        std::string biezace;
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                biezace += s[i];
                biezace += s[++i];             // zachowaj sekwencje ucieczki
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
};

// ---------------------------------------------------------------------------
//  Warstwa interfejsu (menu konsolowe)
// ---------------------------------------------------------------------------

void wyczyscWejscie() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Pobranie liczby calkowitej z walidacja.
int wczytajInt(const std::string& monit) {
    int wartosc;
    while (true) {
        std::cout << monit;
        if (std::cin >> wartosc) {
            wyczyscWejscie();
            return wartosc;
        }
        std::cout << "  !! Podaj liczbe calkowita.\n";
        wyczyscWejscie();
    }
}

// Pobranie liczby zmiennoprzecinkowej z walidacja.
double wczytajDouble(const std::string& monit) {
    double wartosc;
    while (true) {
        std::cout << monit;
        if (std::cin >> wartosc) {
            wyczyscWejscie();
            return wartosc;
        }
        std::cout << "  !! Podaj liczbe (np. 149.99).\n";
        wyczyscWejscie();
    }
}

// Pobranie linii tekstu.
std::string wczytajTekst(const std::string& monit) {
    std::cout << monit;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

void banner() {
    std::cout << "\n";
    std::cout << "  ============================================================\n";
    std::cout << "   SerwisPro Mini  |  konsolowy system zlecen serwisowych\n";
    std::cout << "   Pelna wersja: serwispro.vernex.pl\n";
    std::cout << "   Kod: github.com/Vernex2026/SerwisPRO\n";
    std::cout << "  ============================================================\n";
}

void menu() {
    std::cout << "\n  --- MENU ---------------------------------------------------\n";
    std::cout << "   1. Dodaj zlecenie\n";
    std::cout << "   2. Lista zlecen\n";
    std::cout << "   3. Lista wg kosztu (malejaco)\n";
    std::cout << "   4. Zmien status\n";
    std::cout << "   5. Szukaj po ID\n";
    std::cout << "   6. Szukaj po kliencie\n";
    std::cout << "   7. Ustaw koszt naprawy\n";
    std::cout << "   8. Gotowe do odbioru\n";
    std::cout << "   9. Usun zlecenie\n";
    std::cout << "  10. Statystyki\n";
    std::cout << "  11. Zapisz do pliku (CSV)\n";
    std::cout << "   0. Wyjscie (autozapis)\n";
    std::cout << "  ------------------------------------------------------------\n";
}

void akcjaZmienStatus(Serwis& s) {
    int id = wczytajInt("  Podaj ID (np. 3): ");
    std::cout << "  Status: 1=Przyjete  2=W trakcie  3=Gotowe  4=Wydane\n";
    int w = wczytajInt("  Wybor: ");
    switch (w) {
        case 1: s.zmienStatus(id, Status::Przyjete); break;
        case 2: s.zmienStatus(id, Status::WTrakcie); break;
        case 3: s.zmienStatus(id, Status::Gotowe);   break;
        case 4: s.zmienStatus(id, Status::Wydane);   break;
        default: std::cout << "  !! Nieznany status.\n";
    }
}

int main() {
    Serwis serwis;
    serwis.wczytaj();   // trwalosc: dane z poprzedniego uruchomienia

    banner();
    std::cout << "  (Wczytano dane z pliku " << Serwis::PLIK
              << ", jesli istnial.)\n";

    bool dziala = true;
    while (dziala) {
        menu();
        int wybor = wczytajInt("  Twoj wybor: ");
        std::cout << "\n";
        switch (wybor) {
            case 1: {
                std::string klient = wczytajTekst("  Klient: ");
                std::string tel    = wczytajTekst("  Telefon: ");
                std::string urz    = wczytajTekst("  Urzadzenie: ");
                std::string opis   = wczytajTekst("  Opis usterki: ");
                int id = serwis.dodaj(klient, tel, urz, opis);
                std::cout << "  OK: utworzono zlecenie SRV-" << id << "\n";
                break;
            }
            case 2:  serwis.lista();               break;
            case 3:  serwis.lista(true);           break;
            case 4:  akcjaZmienStatus(serwis);     break;
            case 5:  serwis.szukajPoId(wczytajInt("  Podaj ID: ")); break;
            case 6:  serwis.szukajPoKliencie(wczytajTekst("  Klient (fragment): ")); break;
            case 7: {
                int id = wczytajInt("  Podaj ID: ");
                double k = wczytajDouble("  Koszt (zl): ");
                serwis.ustawKoszt(id, k);
                break;
            }
            case 8:  serwis.gotoweDoOdbioru();     break;
            case 9:  serwis.usun(wczytajInt("  Podaj ID do usuniecia: ")); break;
            case 10: serwis.statystyki();          break;
            case 11:
                std::cout << (serwis.zapisz()
                    ? "  OK: zapisano do " + std::string(Serwis::PLIK) + "\n"
                    : "  !! Blad zapisu pliku.\n");
                break;
            case 0:
                dziala = false;
                break;
            default:
                std::cout << "  !! Nieznana opcja.\n";
        }
    }

    if (serwis.zapisz())
        std::cout << "\n  Autozapis OK -> " << Serwis::PLIK << ". Do zobaczenia!\n";
    else
        std::cout << "\n  !! Autozapis nieudany.\n";

    return 0;
}

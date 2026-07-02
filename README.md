# SerwisPro Mini (C++)

Konsolowy system zarządzania zleceniami serwisowymi napisany w czystym **C++17**.
Projekt edukacyjny (na zajęcia) — pokazuje struktury i klasy, `enum class`, STL,
obsługę plików CSV oraz interaktywne menu.

> 🔗 **Prawdziwy projekt (produkcja):** to jest wersja konsolowa *mini*. Pełny,
> działający system serwisowy autora znajdziesz tutaj:
> - **Strona:** https://serwispro.vernex.pl
> - **Kod:** https://github.com/Vernex2026/SerwisPRO

---

## Kompilacja i uruchomienie

```bash
g++ -std=c++17 -Wall -O2 serwis.cpp -o serwis
./serwis          # Windows: serwis.exe
```

Program kompiluje się bez ostrzeżeń przy fladze `-Wall`.

## Funkcje (menu)

**Baza:**
1. Dodaj zlecenie
2. Lista zleceń
3. Zmień status (Przyjęte → W trakcie → Gotowe → Wydane)
4. Szukaj po ID
5. Szukaj po kliencie (fragment nazwiska, bez rozróżniania wielkości liter)
6. Usuń zlecenie
7. Statystyki wg statusu

**Akcenty (na wyższą ocenę):**
- **Data przyjęcia** zlecenia (`<ctime>`)
- **Priorytet** (Niski / Normalny / Pilny) — drugi `enum class`
- **Historia statusów** każdego zlecenia (wektor w zleceniu, widoczna przy szukaniu po ID)
- **Kolory w konsoli** (ANSI) — status wyróżniony kolorem
- Ustawianie kosztu naprawy dla zlecenia
- Filtr **„Gotowe do odbioru”** (lista klientów + telefony)
- Sortowanie listy po koszcie (malejąco)
- Łączny przychód (suma kosztów zleceń wydanych) + licznik zleceń pilnych

## Trwałość danych

Zlecenia zapisują się do pliku **`zlecenia.csv`** (autozapis przy wyjściu oraz
opcja ręcznego zapisu w menu). Przy starcie dane są automatycznie wczytywane,
więc utrzymują się między uruchomieniami.

## Co pokazuje technicznie

| Element języka / biblioteki | Gdzie |
|---|---|
| `struct` + `class` | `Zlecenie`, `Serwis` |
| dwa `enum class` + konwersje | `Status`, `Priorytet` + funkcje `*NaTekst` / `tekstNa*` |
| `std::vector` (też wektor w zleceniu) | kolekcja zleceń + `historia` statusów |
| STL + lambdy | `std::remove_if`, `std::sort`, `std::accumulate`, `std::count_if`, `std::transform` |
| Data / czas | `<ctime>` (`std::localtime`, `strftime`) |
| Kolory ANSI | wyróżnienie statusu w tabeli |
| Obsługa plików | `fstream`, `stringstream` (zapis/odczyt CSV z ucieczką separatora) |
| Walidacja wejścia | `std::cin` fail → `clear()` + `ignore()` |
| Formatowanie | `iomanip` (`setw`, `setprecision`, `fixed`) |

## Cel projektu

Pokazać podstawy programowania obiektowego i STL w C++ na praktycznym,
zrozumiałym przykładzie (serwis komputerowy/AGD/RTV), z zachowaniem danych
między uruchomieniami — jako mini-odpowiednik produkcyjnego systemu
[SerwisPRO](https://serwispro.vernex.pl).

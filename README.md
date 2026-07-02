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
- Ustawianie kosztu naprawy dla zlecenia
- Filtr **„Gotowe do odbioru”** (lista klientów + telefony)
- Sortowanie listy po koszcie (malejąco)
- Łączny przychód (suma kosztów zleceń wydanych)

## Trwałość danych

Zlecenia zapisują się do pliku **`zlecenia.csv`** (autozapis przy wyjściu oraz
opcja ręcznego zapisu w menu). Przy starcie dane są automatycznie wczytywane,
więc utrzymują się między uruchomieniami.

## Co pokazuje technicznie

| Element języka / biblioteki | Gdzie |
|---|---|
| `struct` + `class` | `Zlecenie`, `Serwis` |
| `enum class` + konwersje | `Status`, `statusNaTekst` / `tekstNaStatus` |
| `std::vector` | kolekcja zleceń |
| STL + lambdy | `std::remove_if`, `std::sort`, `std::accumulate`, `std::transform` |
| Obsługa plików | `fstream`, `stringstream` (zapis/odczyt CSV z ucieczką separatora) |
| Walidacja wejścia | `std::cin` fail → `clear()` + `ignore()` |
| Formatowanie | `iomanip` (`setw`, `setprecision`, `fixed`) |

## Cel projektu

Pokazać podstawy programowania obiektowego i STL w C++ na praktycznym,
zrozumiałym przykładzie (serwis komputerowy/AGD/RTV), z zachowaniem danych
między uruchomieniami — jako mini-odpowiednik produkcyjnego systemu
[SerwisPRO](https://serwispro.vernex.pl).

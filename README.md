# SerwisPro Mini (C++)

Konsolowy system zarządzania zleceniami serwisowymi napisany w czystym **C++17**,
z **warstwową architekturą** (separacja odpowiedzialności), **testami jednostkowymi**
i budowaniem przez **Makefile**.

> 🔗 **Prawdziwy projekt (produkcja):** to jest wersja konsolowa *mini*. Pełny,
> działający system serwisowy autora znajdziesz tutaj:
> - **Strona:** https://serwispro.vernex.pl
> - **Kod:** https://github.com/Vernex2026/SerwisPRO

---

## Budowanie i uruchomienie

```bash
make          # buduje aplikacje ./serwis
./serwis      # uruchamia (Windows: serwis.exe)

make test     # buduje i uruchamia testy jednostkowe
make clean    # usuwa artefakty budowania
```

Kompiluje się bez ostrzeżeń pod `-Wall -Wextra`.

## Architektura (separacja warstw)

Kod jest podzielony na moduły o jednej odpowiedzialności — zależności biegną w jedną
stronę, bez cykli. Kluczowa zasada: **logika nie drukuje na ekran i nie dotyka plików**.

```
include/ + src/
  domena       Model: Zlecenie, enum Status/Priorytet, konwersje, data.
  tekst        Narzędzia na łańcuchach: normalizacja, formatowanie, kodowanie CSV.
  repozytorium CsvRepozytorium — JEDYNA warstwa I/O plików (zapis/odczyt CSV).
  serwis       Serwis — CZYSTA logika: operacje na kolekcji, zwraca dane.
  konsola      Warstwa UI: render tabel/kolorów, walidacja wejścia, pętla menu.
  main         Composition root: spina repozytorium → serwis → aplikację.
tests/
  testy        Testy jednostkowe (bez bibliotek, na assert): Serwis, CSV, tekst.
```

Zależności: `main → konsola → serwis → domena`; `repozytorium → domena, tekst`.
Dzięki temu logikę (`Serwis`) można testować w izolacji, bez konsoli i plików.

## Funkcje (menu)

**Baza:** dodawanie, lista, zmiana statusu (Przyjęte → W trakcie → Gotowe → Wydane),
szukanie po ID i po kliencie, usuwanie, statystyki, zapis/odczyt CSV.

**Rozszerzenia:**
- **Data przyjęcia** zlecenia (`<ctime>`)
- **Priorytet** (Niski / Normalny / Pilny) — drugi `enum class`
- **Historia statusów** każdego zlecenia (widoczna przy szukaniu po ID)
- **Kolory w konsoli** (ANSI) — status wyróżniony kolorem
- Ustawianie kosztu naprawy, filtr **„Gotowe do odbioru”**, sortowanie po koszcie (malejąco)
- Łączny przychód (suma kosztów zleceń wydanych) + licznik zleceń pilnych

## Trwałość danych

Zlecenia zapisują się do pliku **`zlecenia.csv`** (autozapis przy wyjściu + opcja ręczna).
Numeracja ID jest wyprowadzana z danych (`max(id)+1`), więc plik zawiera tylko rekordy.
Pola ze znakami specjalnymi (`;`, `\`) są bezpiecznie kodowane (ucieczka).

## Co pokazuje technicznie

| Element języka / biblioteki | Gdzie |
|---|---|
| Architektura warstwowa | podział `domena` / `serwis` / `repozytorium` / `konsola` |
| `struct` + klasy | `Zlecenie`, `Serwis`, `CsvRepozytorium`, `Aplikacja` |
| dwa `enum class` + konwersje | `Status`, `Priorytet` (`include/domena.hpp`) |
| `std::vector` (też wektor w zleceniu) | kolekcja zleceń + `historia` |
| STL + lambdy | `std::find_if`, `std::remove_if`, `std::sort`, `std::accumulate`, `std::count_if`, `std::max_element` |
| `std::optional` / `std::string_view` | parsowanie CSV / API konwersji |
| Data / czas | `<ctime>` (`std::localtime`, `strftime`) |
| Obsługa plików | `fstream`, `stringstream` (CSV z ucieczką separatora) |
| Testy jednostkowe | `tests/testy.cpp` (`make test`) |

## Cel projektu

Pokazać podstawy programowania obiektowego i STL w C++ z **solidnym podziałem na warstwy**
(logika osobno od I/O i prezentacji) oraz pokryciem testami — jako mini-odpowiednik
produkcyjnego systemu [SerwisPRO](https://serwispro.vernex.pl).

---

## O prawdziwym projekcie — SerwisPro (SaaS)

**SerwisPro** to wielodostępowy (**multi-tenant**) system klasy **SaaS** dla
serwisów elektroniki i motoryzacji w Polsce — od przyjęcia sprzętu, przez
naprawę i komunikację z klientem, aż po fakturę. Zamiast zeszytu i Excela
warsztat dostaje jedno spójne narzędzie działające w przeglądarce i jako
aplikacja mobilna (**PWA**).

### Plan / wizja
Jeden panel, w którym mały i średni serwis prowadzi **cały cykl życia zlecenia**
i całą firmę — bez sklejania kilku aplikacji. Każda firma to osobna, odizolowana
przestrzeń danych z kontrolą dostępu wg ról.

### Co potrafi
- **Zlecenia serwisowe** — pełny obieg: przyjęcie → diagnoza → naprawa → wydanie,
  z historią statusów, zdjęciami, kosztorysem i zapotrzebowaniem na części.
- **Portal klienta przez QR** — klient skanuje kod i na żywo widzi status, koszt
  i termin naprawy; może zaakceptować kosztorys i napisać do serwisu.
- **Klienci i sprzęt** — baza klientów, urządzeń, pojazdów, historii napraw.
- **Magazyn i zamówienia** — stany części, przyjęcia/wydania, kolejka zamówień
  u dostawców, magazyn opon (wertykał moto).
- **Komunikacja** — integracja Gmail (poczta), SMS, powiadomienia, czat zespołu.
- **Faktury i finanse** — wystawianie faktur, integracja **KSeF**, udostępnianie
  dokumentów.
- **Zespół i uprawnienia (RBAC)** — role owner/manager/technik/recepcja; każdy
  widzi tylko to, co powinien (izolacja danych na poziomie bazy).
- **Analityka (telemetria)** — przychody, obłożenie, czasy napraw, koszty AI.
- **Automatyzacje i asystent AI** — reguły, szablony, pomoc AI w obsłudze poczty.
- **Marketplace i reklamacje (RMA)** — obrót częściami, obsługa gwarancji.

### Co to daje
- **Serwisowi:** mniej chaosu, szybsza obsługa, kontrola kosztów i pełen wgląd
  w firmę z jednego miejsca — także z telefonu.
- **Klientowi:** przejrzystość — wie co się dzieje z jego sprzętem, ile zapłaci
  i kiedy odbierze, bez dzwonienia do serwisu.

### Stack
React + TypeScript + Vite (PWA), Supabase (Postgres z RLS, Auth, Storage,
Edge Functions), deploy na Vercel.

**Live:** https://serwispro.vernex.pl · **Kod:** https://github.com/Vernex2026/SerwisPRO

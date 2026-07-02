// ============================================================================
//  repozytorium.hpp — trwalosc zlecen w pliku CSV.
//  Jedyna warstwa dotykajaca plikow. Nie zna logiki biznesowej ani UI —
//  tlumaczy tylko miedzy kolekcja Zlecenie a plikiem tekstowym.
// ============================================================================
#ifndef SERWIS_REPOZYTORIUM_HPP
#define SERWIS_REPOZYTORIUM_HPP

#include "domena.hpp"

#include <optional>
#include <string>
#include <vector>

namespace serwis {

class CsvRepozytorium {
public:
    // Wczytuje zlecenia z pliku; brak pliku -> pusta lista (nie blad).
    [[nodiscard]] std::vector<Zlecenie> wczytaj(const std::string& sciezka) const;

    // Zapisuje zlecenia; zwraca false przy bledzie otwarcia pliku.
    bool zapisz(const std::string& sciezka, const std::vector<Zlecenie>& dane) const;

private:
    static std::string doLinii(const Zlecenie& z);
    static std::optional<Zlecenie> zLinii(const std::string& linia);
};

}  // namespace serwis

#endif  // SERWIS_REPOZYTORIUM_HPP

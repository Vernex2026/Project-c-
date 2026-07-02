// ============================================================================
//  main.cpp — punkt startowy (composition root).
//  Spina warstwy: repozytorium (trwalosc) -> serwis (logika) -> aplikacja (UI).
// ============================================================================
#include "konsola.hpp"
#include "repozytorium.hpp"
#include "serwis.hpp"

#include <string>

int main() {
    const std::string sciezka = "zlecenia.csv";

    serwis::CsvRepozytorium repo;
    serwis::Serwis serwis(repo.wczytaj(sciezka));   // dane z poprzedniej sesji
    serwis::Aplikacja aplikacja(serwis, repo, sciezka);

    aplikacja.uruchom();
    return 0;
}

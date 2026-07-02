// ============================================================================
//  domena.cpp — implementacja konwersji i drobnych operacji modelu.
// ============================================================================
#include "domena.hpp"

#include <ctime>

namespace serwis {

std::string statusNaTekst(Status s) {
    switch (s) {
        case Status::Przyjete: return "Przyjete";
        case Status::WTrakcie: return "W trakcie";
        case Status::Gotowe:   return "Gotowe";
        case Status::Wydane:   return "Wydane";
    }
    return "?";
}

Status tekstNaStatus(std::string_view t) {
    if (t == "W trakcie") return Status::WTrakcie;
    if (t == "Gotowe")    return Status::Gotowe;
    if (t == "Wydane")    return Status::Wydane;
    return Status::Przyjete;
}

std::string priorytetNaTekst(Priorytet p) {
    switch (p) {
        case Priorytet::Niski: return "Niski";
        case Priorytet::Pilny: return "PILNY";
        case Priorytet::Normalny: break;
    }
    return "Normalny";
}

Priorytet tekstNaPriorytet(std::string_view t) {
    if (t == "Niski") return Priorytet::Niski;
    if (t == "PILNY") return Priorytet::Pilny;
    return Priorytet::Normalny;
}

const char* kolorStatus(Status s) {
    switch (s) {
        case Status::Przyjete: return "\033[36m";  // cyan
        case Status::WTrakcie: return "\033[33m";  // zolty
        case Status::Gotowe:   return "\033[32m";  // zielony
        case Status::Wydane:   return "\033[90m";  // szary
    }
    return "\033[0m";
}

std::size_t indeks(Status s) {
    return static_cast<std::size_t>(s);
}

std::string dzisiajISO() {
    std::time_t t = std::time(nullptr);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}

std::string wpisHistorii(std::string_view data, Status s) {
    std::string out(data);
    out += " - ";
    out += statusNaTekst(s);
    return out;
}

}  // namespace serwis

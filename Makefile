# ============================================================================
#  Makefile — SerwisPro Mini
#    make        -> buduje aplikacje ./serwis
#    make test   -> buduje i uruchamia testy jednostkowe
#    make clean  -> usuwa artefakty budowania
# ============================================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
BIN      := bin

# Zrodla aplikacji (main + wszystkie moduly).
SRC      := $(wildcard src/*.cpp)
OBJ      := $(SRC:src/%.cpp=$(BIN)/%.o)

# Moduly bez main.cpp — wspoldzielone z testami.
LIB_SRC  := $(filter-out src/main.cpp,$(SRC))
LIB_OBJ  := $(LIB_SRC:src/%.cpp=$(BIN)/%.o)

TARGET   := serwis
TEST_BIN := $(BIN)/testy

.PHONY: all test clean

all: $(TARGET)

# --- Aplikacja --------------------------------------------------------------
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# --- Testy ------------------------------------------------------------------
test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): tests/testy.cpp $(LIB_OBJ) | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

# --- Kompilacja modulow -----------------------------------------------------
$(BIN)/%.o: src/%.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -rf $(BIN) $(TARGET)

CXX = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror -Iinclude
ASAN_FLAGS = -fsanitize=address -g
OPT_FLAGS = -O3 -march=native -DNDEBUG

BUILD_DIR = build
BIN_DIR = bin

all: test_asan bench

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compilation du test unitaire avec AddressSanitizer
$(BIN_DIR)/test_math_core: native/src/math_core.cpp native/tests/test_math_core.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(ASAN_FLAGS) -I native/include $^ -o $@

# Compilation du banc de performance Mercuria (optimisé)
$(BIN_DIR)/bench_math_core: native/src/math_core.cpp native/bench/bench_math_core.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) -I native/include $^ -o $@

test_asan: $(BIN_DIR)/test_math_core
	@echo "--- Exécution des tests sous AddressSanitizer ---"
	./$(BIN_DIR)/test_math_core

bench: $(BIN_DIR)/bench_math_core
	@echo "--- Exécution du banc Mercuria ---"
	./$(BIN_DIR)/bench_math_core

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

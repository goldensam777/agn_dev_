ifeq ($(origin CXX),default)
  CXX = clang++
endif
CXX ?= clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror -Wconversion -Iinclude
ASAN_FLAGS = -fsanitize=address,undefined -g
OPT_FLAGS = -O3 -march=native -DNDEBUG

BUILD_DIR = build
BIN_DIR = bin

all: test_asan bench $(BIN_DIR)/forge_compute

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Exécutable CLI pour le bridge serveur
$(BIN_DIR)/forge_compute: native/src/math_core.cpp native/src/cli_main.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) -I native/include $^ -o $@

# Compilation du test unitaire avec AddressSanitizer
$(BIN_DIR)/test_math_core: native/src/math_core.cpp native/tests/test_math_core.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(ASAN_FLAGS) -I native/include $^ -o $@

# Compilation de l'exemple du moteur Onyx avec AddressSanitizer
ONYX_SRCS = examples/onyx/lexer.cpp examples/onyx/parser.cpp examples/onyx/linear_check.cpp examples/onyx/runtime.cpp
$(BIN_DIR)/test_onyx: $(ONYX_SRCS) examples/test_onyx.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(ASAN_FLAGS) -I native/include -I examples $^ -o $@

# Compilation du banc de performance Mercuria (optimisé)
$(BIN_DIR)/bench_math_core: native/src/math_core.cpp native/bench/bench_math_core.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OPT_FLAGS) -I native/include $^ -o $@

test_asan: $(BIN_DIR)/test_math_core $(BIN_DIR)/test_onyx
	@echo "--- Exécution des tests sous AddressSanitizer (Math Core) ---"
	./$(BIN_DIR)/test_math_core
	@echo "--- Exécution des tests sous AddressSanitizer (Onyx Engine) ---"
	./$(BIN_DIR)/test_onyx

bench: $(BIN_DIR)/bench_math_core
	@echo "--- Exécution du banc Mercuria ---"
	./$(BIN_DIR)/bench_math_core

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

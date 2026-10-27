# =============================================================================
#  PawPocalypse - Makefile
#
#  Comandos:
#    make        -> compila o jogo
#    make run    -> compila e executa
#    make clean  -> apaga os arquivos gerados na compilação
#
#  Requisito: raylib instalada
#    macOS   : brew install raylib
#    Linux   : sudo apt install libraylib-dev  (ou compilar a partir do código-fonte)
#    Windows : MinGW + raylib (ajuste RAYLIB_PATH abaixo, se necessário)
# =============================================================================

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2 -Iinclude -MMD -MP
LDFLAGS =
LDLIBS  =

SRC_DIR = src
OBJ_DIR = build
EXEC    = pawpocalypse

# Todos os .c dentro de src/ viram .o dentro de build/
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

# ----------------------------------------------------------------------------
#  Configuração por sistema operacional
# ----------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
    EXEC        := $(EXEC).exe
    RAYLIB_PATH ?= C:/raylib/raylib/src
    CFLAGS      += -I$(RAYLIB_PATH)
    LDFLAGS     += -L$(RAYLIB_PATH)
    LDLIBS      += -lraylib -lopengl32 -lgdi32 -lwinmm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        # macOS: raylib instalada pelo Homebrew
        BREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /opt/homebrew)
        CFLAGS      += -I$(BREW_PREFIX)/include
        LDFLAGS     += -L$(BREW_PREFIX)/lib
        LDLIBS      += -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        # Linux
        LDLIBS      += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif

# ----------------------------------------------------------------------------
#  Regras
# ----------------------------------------------------------------------------
.PHONY: all run clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJ_DIR) $(EXEC)

# Recompila automaticamente quando um .h muda
-include $(DEPS)

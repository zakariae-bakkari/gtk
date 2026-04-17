# ===========================================================================
# Makefile — Banc de Poisson (GTK4 + Cairo)
# ===========================================================================

# --- Compilateur & flags ---
CC       = gcc
CFLAGS   = -Wall -Wextra -g \
           $(shell pkg-config --cflags gtk4) \
           -I./include \
           -I./src/widgets
LDFLAGS  = $(shell pkg-config --libs gtk4) -lm

# --- Répertoires ---
SRC_DIR     = src
INC_DIR     = include
WIDGET_DIR  = src/widgets
BUILD_DIR   = build
BIN         = banc_de_poisson

# --- Sources du PROJET ---
PROJECT_SRCS = \
    $(SRC_DIR)/main.c          \
    $(SRC_DIR)/entities.c      \
    $(SRC_DIR)/draw.c          \
    $(SRC_DIR)/bassin.c        \
    $(SRC_DIR)/game.c          \
    $(SRC_DIR)/screen_accueil.c  \
    $(SRC_DIR)/screen_createur.c \
    $(SRC_DIR)/screen_jeux.c

# --- Sources des WIDGETS (bibliothèque fournie) ---
WIDGET_SRCS = \
    $(WIDGET_DIR)/fenetre.c          \
    $(WIDGET_DIR)/conteneur.c        \
    $(WIDGET_DIR)/bouton.c           \
    $(WIDGET_DIR)/bouton_checklist.c \
    $(WIDGET_DIR)/bouton_radio.c     \
    $(WIDGET_DIR)/champ_texte.c      \
    $(WIDGET_DIR)/champ_nombre.c     \
    $(WIDGET_DIR)/champ_select.c     \
    $(WIDGET_DIR)/champ_motdepasse.c \
    $(WIDGET_DIR)/champ_zone_texte.c \
    $(WIDGET_DIR)/slider.c           \
    $(WIDGET_DIR)/texte.c            \
    $(WIDGET_DIR)/image.c            \
    $(WIDGET_DIR)/menu.c             \
    $(WIDGET_DIR)/dialog.c           \
    $(WIDGET_DIR)/common.c           \
    $(WIDGET_DIR)/export_xml.c       \
    $(WIDGET_DIR)/xml_parser.c

ALL_SRCS = $(PROJECT_SRCS) $(WIDGET_SRCS)

# --- Objets ---
PROJECT_OBJS = $(patsubst $(SRC_DIR)/%.c,   $(BUILD_DIR)/%.o, $(PROJECT_SRCS))
WIDGET_OBJS  = $(patsubst $(WIDGET_DIR)/%.c,$(BUILD_DIR)/w_%.o, $(WIDGET_SRCS))
ALL_OBJS     = $(PROJECT_OBJS) $(WIDGET_OBJS)

# ===========================================================================
# CIBLES PRINCIPALES
# ===========================================================================

.PHONY: all clean run debug dirs help

all: dirs $(BIN)

$(BIN): $(ALL_OBJS)
	@echo "🔗 Linkage → $@"
	$(CC) $(ALL_OBJS) -o $@ $(LDFLAGS)
	@echo "✅ Build OK : ./$(BIN)"

# --- Compilation sources projet ---
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "📦 Compile $<"
	$(CC) $(CFLAGS) -c $< -o $@

# --- Compilation sources widgets ---
$(BUILD_DIR)/w_%.o: $(WIDGET_DIR)/%.c
	@echo "🔧 Widget  $<"
	$(CC) $(CFLAGS) -c $< -o $@

# --- Création répertoires ---
dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(WIDGET_DIR)
	@mkdir -p assets/sounds
	@mkdir -p xml
	@mkdir -p data

# --- Lancement ---
run: all
	./$(BIN)

# --- Debug avec valgrind ---
debug: all
	valgrind --leak-check=full --track-origins=yes ./$(BIN)

# --- Nettoyage ---
clean:
	@rm -rf $(BUILD_DIR) $(BIN)
	@echo "🧹 Nettoyage terminé"

# --- Aide ---
help:
	@echo ""
	@echo "  make          → Compile le projet"
	@echo "  make run      → Compile et lance"
	@echo "  make debug    → Lance sous valgrind"
	@echo "  make clean    → Supprime les fichiers compilés"
	@echo ""
	@echo "  Dépendances requises :"
	@echo "    pkg-config gtk4"
	@echo "    gcc"
	@echo ""
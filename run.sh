#!/bin/bash

# Add MSYS2 MinGW to PATH so gcc and pkg-config can be found automatically
MSYS_PATH="/c/msys64/mingw64/bin"
if [ -d "$MSYS_PATH" ]; then
    export PATH="$MSYS_PATH:$PATH"
fi

# Validate input
if [ -z "$1" ]; then
    echo "================================================================"
    echo "                  GTK4 EXAM WORKSPACE RUNNER"
    echo "================================================================"
    echo "[ERROR] No C file or exam name specified."
    echo ""
    echo "Usage:"
    echo "  ./run.sh [path_to_c_file_or_exam_name]"
    echo ""
    echo "Examples:"
    echo "  ./run.sh src/tests/exams/exame2"
    echo "  ./run.sh exame2                 # (Resolves to src/tests/exams/exame2.c)"
    echo "  ./run.sh src/tests/test_menu.c"
    echo "================================================================"
    exit 1
fi

TARGET="$1"

# If target does not contain "/" or "\", assume it is in src/tests/exams/
if [[ ! "$TARGET" =~ [/] ]]; then
    TARGET="src/tests/exams/$TARGET"
fi

# Append .c extension if not present
if [[ ! "$TARGET" == *.c ]]; then
    TARGET="${TARGET}.c"
fi

TARGET_DIR=$(dirname "$TARGET")
TARGET_NAME=$(basename "$TARGET" .c)

# Get the depth of the target path relative to current dir to dynamically set REL_PATH
DEPTH=$(echo "$TARGET" | tr -cd '/' | wc -c)
REL_PATH=""
for ((i=0; i<DEPTH; i++)); do
    REL_PATH="${REL_PATH}../"
done
if [ -z "$REL_PATH" ]; then
    REL_PATH="./"
fi

# If file does not exist, create it with your custom widgets GTK4 boilerplate
if [ ! -f "$TARGET" ]; then
    echo "[INFO] File '$TARGET' does not exist."
    echo "[INFO] Creating directory '$TARGET_DIR'..."
    mkdir -p "$TARGET_DIR"
    
    echo "[INFO] Creating file '$TARGET' with default Minimal GTK4 Fenetre template..."
    cat << EOF > "$TARGET"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "${REL_PATH}widgets/headers/fenetre.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("${TARGET_NAME}") + 1);
    strcpy(fenetre.title, "${TARGET_NAME}");
    fenetre.taille.width = 800;
    fenetre.taille.height = 600;

    GtkWidget *window = fenetre_creer(&fenetre, app);

    // 2. Present the window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("fr.exam.${TARGET_NAME}", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
EOF
    echo "[OK] Boilerplate created successfully at '$TARGET'."
fi

# Terminate any previously running instances of this executable to release the file lock
taskkill -f -im "${TARGET_NAME}.exe" >/dev/null 2>&1 || killall -9 "${TARGET_NAME}.exe" >/dev/null 2>&1 || killall -9 "${TARGET_NAME}" >/dev/null 2>&1

# Compile the C file
echo "[INFO] Compiling '$TARGET'..."

# Fetch compiler flags and libs using pkg-config
CFLAGS=$(pkg-config --cflags gtk4 2>/dev/null)
LIBS=$(pkg-config --libs gtk4 2>/dev/null)

if [ -z "$CFLAGS" ]; then
    echo "[WARNING] pkg-config failed or was not found in PATH."
fi

EXE_NAME="${TARGET_DIR}/${TARGET_NAME}.exe"

# Compile with GCC, combining the exam file and custom widgets sources
echo "[COMMAND] gcc -o \"$EXE_NAME\" \"$TARGET\" widgets/sources/*.c -Isrc -Iwidgets/headers $CFLAGS $LIBS -lwinmm -lm"
gcc -o "$EXE_NAME" "$TARGET" widgets/sources/*.c -Isrc -Iwidgets/headers $CFLAGS $LIBS -lwinmm -lm

if [ $? -eq 0 ]; then
    echo "[OK] Compilation successful! Output binary: '$EXE_NAME'"
    echo "[INFO] Running application..."
    echo ""
    "./$EXE_NAME"
    echo ""
    echo "[INFO] Execution completed."
else
    echo "[ERROR] Compilation failed. Please check compiler errors above."
    exit 1
fi

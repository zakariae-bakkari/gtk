# Compiler
CC = gcc

# Target executable
TARGET = banc_poisson

# Directories
SRCDIR = src
WDIR   = $(SRCDIR)/widgets

# GTK package
PKG = gtk4

# Flags
CFLAGS = $(shell pkg-config --cflags $(PKG)) -Wall -Wextra -I$(SRCDIR) -I$(WDIR)/headers
LIBS   = $(shell pkg-config --libs $(PKG)) -lm

# Project sources
SRCS = $(SRCDIR)/main.c            \
       $(SRCDIR)/entities.c        \
       $(SRCDIR)/draw.c            \
       $(SRCDIR)/bassin.c          \
       $(SRCDIR)/screen_accueil.c  \
       $(SRCDIR)/screen_createur.c \
       $(SRCDIR)/screen_jeux.c

# Widget sources
WIDGET_SRCS = $(wildcard $(WDIR)/sources/*.c)

# All sources
ALL_SRCS = $(SRCS) $(WIDGET_SRCS)

# Object files
OBJS = $(ALL_SRCS:.c=.o)

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

# Compile
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

# Run
run: all
	./$(TARGET)

# Windows-friendly run (optional)
run-win: all
	./$(TARGET).exe

.PHONY: all clean run run-win
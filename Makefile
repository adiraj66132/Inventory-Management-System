# Makefile for Inventory Management System
# Compatible with Arch Linux, Debian, and other distributions

CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lncurses
TARGET = inventory
SRC = inventory.c

# Installation directories
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

.PHONY: all clean install uninstall help

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET) *.o *.dat *.log *.csv
	rm -f inventory_backup_*.dat

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "Installation complete! Run with: $(TARGET)"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "Uninstalled successfully"

# Development targets
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

static: CFLAGS += -static
static: $(TARGET)

# Clean user data (use with caution!)
clean-data:
	@echo "WARNING: This will delete all inventory data!"
	@read -p "Are you sure? [y/N] " -n 1 -r; \
	if [[ $$REPLY =~ ^[Yy]$$ ]]; then \
		rm -f inventory.dat audit.log config.dat *.csv inventory_backup_*.dat; \
		echo "\nData cleaned."; \
	else \
		echo "\nCancelled."; \
	fi

help:
	@echo "Inventory Management System - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build the application"
	@echo "  make clean        - Remove built files"
	@echo "  make install      - Install to system (requires root)"
	@echo "  make uninstall    - Remove from system (requires root)"
	@echo "  make debug        - Build with debug symbols"
	@echo "  make static       - Build statically linked binary"
	@echo "  make clean-data   - Remove all user data (interactive)"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - GCC compiler"
	@echo "  - ncurses development library"
	@echo ""
	@echo "Arch Linux installation:"
	@echo "  sudo pacman -S gcc ncurses"
	@echo ""
	@echo "Debian/Ubuntu installation:"
	@echo "  sudo apt-get install gcc libncurses-dev"

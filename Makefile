# Makefile for Inventory Management System v3.0
# SQLite-based version with multi-user support

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
LDFLAGS = -lncurses -lsqlite3 -lm

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = .

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = $(BIN_DIR)/inventory

INCLUDE = -I$(INC_DIR)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p data
	@echo "Linking $@..."
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

debug: CFLAGS += -g -O0 -DDEBUG
debug: clean $(TARGET)
	@echo "Debug build complete"

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)

install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	install -D -m 644 $(INC_DIR)/*.h $(DESTDIR)$(PREFIX)/include/inventory-manager/
	install -D -m 644 config.sample $(DESTDIR)$(PREFIX)/etc/inventory.conf
	@echo "Installation complete! Run with: $(TARGET)"

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	rm -rf $(DESTDIR)$(PREFIX)/include/inventory-manager
	rm -f $(DESTDIR)$(PREFIX)/etc/inventory.conf
	@echo "Uninstalled successfully"

clean-data:
	@echo "WARNING: This will delete all inventory data!"
	@echo "Database, logs, and backups will be removed."
	@read -p "Are you sure? [y/N] " -n 1 -r; \
	if [[ $$REPLY =~ ^[Yy]$$ ]]; then \
		rm -rf data/; \
		echo "\nData cleaned."; \
	else \
		echo "\nCancelled."; \
	fi

.PHONY: all clean debug install uninstall clean-data help

help:
	@echo "Inventory Management System v3.0"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the application"
	@echo "  make debug    - Build with debug symbols"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make install  - Install to system"
	@echo "  make uninstall - Remove from system"
	@echo "  make clean-data - Delete all data (interactive)"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Dependencies:"
	@echo "  - gcc (C compiler)"
	@echo "  - ncurses (terminal UI)"
	@echo "  - sqlite3 (database)"
	@echo ""
	@echo "Arch Linux installation:"
	@echo "  sudo pacman -S gcc ncurses sqlite"
	@echo ""
	@echo "Debian/Ubuntu installation:"
	@echo "  sudo apt-get install gcc libncurses-dev libsqlite3-dev"

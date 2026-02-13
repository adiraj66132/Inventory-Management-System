#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "db.h"
#include "auth.h"
#include "item.h"
#include "ui.h"
#include "config.h"

void print_banner(void) {
    printf("\n");
    printf("========================================\n");
    printf("       INVENTORY MANAGEMENT SYSTEM      \n");
    printf("              Version 3.0              \n");
    printf("========================================\n");
    printf("\n");
}

void print_usage(const char *prog) {
    printf("Usage: %s [OPTIONS]\n\n", prog);
    printf("Options:\n");
    printf("  -h, --help           Show this help message\n");
    printf("  -c, --config FILE    Specify config file\n");
    printf("  -d, --db FILE       Specify database file\n");
    printf("  -v, --version       Show version\n");
    printf("\n");
}

void print_version(void) {
    printf("Inventory Management System v3.0\n");
    printf("Built with SQLite + ncurses\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    const char *config_file = "config.ini";
    const char *db_file = NULL;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                config_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--db") == 0) {
            if (i + 1 < argc) {
                db_file = argv[++i];
            }
        }
    }
    
    config_load(config_file);
    
    if (db_file) {
        db_set_db_path(db_file);
    }
    
    Config *cfg = config_get();
    
    if (!db_init(cfg->db_path)) {
        fprintf(stderr, "Error: Failed to initialize database!\n");
        return 1;
    }
    
    auth_init();
    
    print_banner();
    printf("Database: %s\n", db_get_db_path());
    printf("\n");
    
    ui_init();
    ui_login_screen();
    
    if (auth_get_current_user()) {
        ui_main_menu();
    }
    
    ui_cleanup();
    db_close();
    
    return 0;
}

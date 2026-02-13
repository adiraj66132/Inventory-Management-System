#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Config global_config;

void config_set_defaults(void) {
    strncpy(global_config.db_path, "data/inventory.db", sizeof(global_config.db_path) - 1);
    global_config.items_per_page = 15;
    global_config.low_stock_default = 0;
    strncpy(global_config.default_category, "Uncategorized", sizeof(global_config.default_category) - 1);
    global_config.auto_backup = false;
    strncpy(global_config.backup_path, "data/backups", sizeof(global_config.backup_path) - 1);
}

bool config_load(const char *filename) {
    config_set_defaults();
    
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return false;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char key[100] = {0};
        char value[256] = {0};
        
        if (sscanf(line, "%99[^=]=%255[^\n]", key, value) == 2) {
            char *k = key;
            while (*k == ' ' || *k == '\t') k++;
            char *ke = k + strlen(k) - 1;
            while (ke > k && (*ke == ' ' || *ke == '\t')) *ke-- = '\0';
            
            char *v = value;
            while (*v == ' ' || *v == '\t') v++;
            
            if (strcmp(k, "db_path") == 0) {
                strncpy(global_config.db_path, v, sizeof(global_config.db_path) - 1);
            } else if (strcmp(k, "items_per_page") == 0) {
                global_config.items_per_page = atoi(v);
            } else if (strcmp(k, "low_stock_default") == 0) {
                global_config.low_stock_default = atoi(v);
            } else if (strcmp(k, "default_category") == 0) {
                strncpy(global_config.default_category, v, sizeof(global_config.default_category) - 1);
            } else if (strcmp(k, "auto_backup") == 0) {
                global_config.auto_backup = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);
            } else if (strcmp(k, "backup_path") == 0) {
                strncpy(global_config.backup_path, v, sizeof(global_config.backup_path) - 1);
            }
        }
    }
    
    fclose(fp);
    return true;
}

bool config_save(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return false;
    }
    
    fprintf(fp, "db_path=%s\n", global_config.db_path);
    fprintf(fp, "items_per_page=%d\n", global_config.items_per_page);
    fprintf(fp, "low_stock_default=%d\n", global_config.low_stock_default);
    fprintf(fp, "default_category=%s\n", global_config.default_category);
    fprintf(fp, "auto_backup=%s\n", global_config.auto_backup ? "true" : "false");
    fprintf(fp, "backup_path=%s\n", global_config.backup_path);
    
    fclose(fp);
    return true;
}

Config* config_get(void) {
    return &global_config;
}

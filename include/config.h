#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    char db_path[256];
    int items_per_page;
    int low_stock_default;
    char default_category[50];
    bool auto_backup;
    char backup_path[256];
} Config;

bool config_load(const char *filename);
bool config_save(const char *filename);
Config* config_get(void);
void config_set_defaults(void);

#endif

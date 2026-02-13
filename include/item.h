#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>

typedef enum {
    SORT_BY_ID,
    SORT_BY_NAME,
    SORT_BY_QUANTITY,
    SORT_BY_PRICE,
    SORT_BY_CATEGORY
} SortField;

typedef enum {
    SORT_ASC,
    SORT_DESC
} SortOrder;

bool item_add(const char *name, int quantity, float price, const char *category, int threshold);
bool item_update(int id, const char *name, int quantity, float price, const char *category, int threshold);
bool item_delete(int id);
bool item_list(SortField field, SortOrder order);
bool item_search(const char *query);
bool item_list_by_category(const char *category);
bool item_list_low_stock(void);
bool item_export_csv(const char *filename);
bool item_import_csv(const char *filename);
bool item_get_statistics(void);

#endif

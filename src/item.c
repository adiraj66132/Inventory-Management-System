#include "../include/item.h"
#include "../include/db.h"
#include "../include/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

static int compare_items(const void *a, const void *b, int field, int order) {
    Item *ia = (Item *)a;
    Item *ib = (Item *)b;
    int cmp = 0;
    
    switch (field) {
        case SORT_BY_ID:
            cmp = ia->id - ib->id;
            break;
        case SORT_BY_NAME:
            cmp = strcasecmp(ia->name, ib->name);
            break;
        case SORT_BY_QUANTITY:
            cmp = ia->quantity - ib->quantity;
            break;
        case SORT_BY_PRICE:
            if (ia->price < ib->price) cmp = -1;
            else if (ia->price > ib->price) cmp = 1;
            break;
        case SORT_BY_CATEGORY:
            cmp = strcasecmp(ia->category, ib->category);
            break;
    }
    
    return order == SORT_ASC ? cmp : -cmp;
}

static void swap_items(Item *a, Item *b) {
    Item temp = *a;
    *a = *b;
    *b = temp;
}

static void bubble_sort(Item *items, int count, SortField field, SortOrder order) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (compare_items(&items[j], &items[j + 1], field, order) > 0) {
                swap_items(&items[j], &items[j + 1]);
            }
        }
    }
}

bool item_add(const char *name, int quantity, float price, const char *category, int threshold) {
    if (!auth_has_permission("manager")) {
        return false;
    }
    
    Item item;
    memset(&item, 0, sizeof(Item));
    strncpy(item.name, name, sizeof(item.name) - 1);
    item.quantity = quantity;
    item.price = price;
    strncpy(item.category, category ? category : "Uncategorized", sizeof(item.category) - 1);
    item.low_stock_threshold = threshold;
    
    int id = db_add_item(&item);
    if (id > 0) {
        Session *sess = auth_get_current_user();
        char details[256];
        snprintf(details, sizeof(details), "Added item: %s (Qty: %d, Price: %.2f)", name, quantity, price);
        db_add_audit_log(sess ? sess->id : 0, "ADD_ITEM", id, details);
        
        if (category && strlen(category) > 0) {
            db_add_category(category);
        }
        
        return true;
    }
    
    return false;
}

bool item_update(int id, const char *name, int quantity, float price, const char *category, int threshold) {
    if (!auth_has_permission("manager")) {
        return false;
    }
    
    Item *existing = db_get_item(id);
    if (!existing) {
        return false;
    }
    
    Item item = *existing;
    free(existing);
    
    strncpy(item.name, name, sizeof(item.name) - 1);
    item.quantity = quantity;
    item.price = price;
    strncpy(item.category, category ? category : "Uncategorized", sizeof(item.category) - 1);
    item.low_stock_threshold = threshold;
    
    bool result = db_update_item(&item);
    
    if (result) {
        Session *sess = auth_get_current_user();
        char details[256];
        snprintf(details, sizeof(details), "Updated item: %s (Qty: %d, Price: %.2f)", name, quantity, price);
        db_add_audit_log(sess ? sess->id : 0, "UPDATE_ITEM", id, details);
        
        if (category && strlen(category) > 0) {
            db_add_category(category);
        }
    }
    
    return result;
}

bool item_delete(int id) {
    if (!auth_has_permission("admin")) {
        return false;
    }
    
    Item *item = db_get_item(id);
    if (!item) {
        return false;
    }
    
    char details[256];
    snprintf(details, sizeof(details), "Deleted item: %s (ID: %d)", item->name, id);
    
    Session *sess = auth_get_current_user();
    db_add_audit_log(sess ? sess->id : 0, "DELETE_ITEM", id, details);
    
    free(item);
    return db_delete_item(id);
}

bool item_list(SortField field, SortOrder order) {
    int count = 0;
    Item *items = db_get_all_items(&count);
    
    if (!items || count == 0) {
        if (items) free(items);
        return false;
    }
    
    bubble_sort(items, count, field, order);
    
    printf("\n");
    printf("================================================================================\n");
    printf("%-5s %-20s %-10s %-10s %-15s %-10s\n", "ID", "Name", "Category", "Quantity", "Price", "Low Stock");
    printf("================================================================================\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-10s %-10d $%-9.2f %-10d\n",
               items[i].id,
               items[i].name,
               items[i].category,
               items[i].quantity,
               items[i].price,
               items[i].low_stock_threshold);
    }
    
    printf("================================================================================\n");
    printf("Total: %d items\n", count);
    
    free(items);
    return true;
}

bool item_search(const char *query) {
    int count = 0;
    Item *items = db_search_items(query, &count);
    
    if (!items || count == 0) {
        if (items) free(items);
        return false;
    }
    
    printf("\n");
    printf("Search results for '%s':\n", query);
    printf("================================================================================\n");
    printf("%-5s %-20s %-10s %-10s %-15s %-10s\n", "ID", "Name", "Category", "Quantity", "Price", "Low Stock");
    printf("================================================================================\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-10s %-10d $%-9.2f %-10d\n",
               items[i].id,
               items[i].name,
               items[i].category,
               items[i].quantity,
               items[i].price,
               items[i].low_stock_threshold);
    }
    
    printf("================================================================================\n");
    printf("Found: %d items\n", count);
    
    free(items);
    return true;
}

bool item_list_by_category(const char *category) {
    int count = 0;
    Item *items = db_get_items_by_category(category, &count);
    
    if (!items || count == 0) {
        if (items) free(items);
        return false;
    }
    
    printf("\n");
    printf("Items in category '%s':\n", category);
    printf("================================================================================\n");
    printf("%-5s %-20s %-10s %-10s %-15s %-10s\n", "ID", "Name", "Category", "Quantity", "Price", "Low Stock");
    printf("================================================================================\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-10s %-10d $%-9.2f %-10d\n",
               items[i].id,
               items[i].name,
               items[i].category,
               items[i].quantity,
               items[i].price,
               items[i].low_stock_threshold);
    }
    
    printf("================================================================================\n");
    printf("Total: %d items in this category\n", count);
    
    free(items);
    return true;
}

bool item_list_low_stock(void) {
    int count = 0;
    Item *items = db_get_low_stock_items(&count);
    
    if (!items || count == 0) {
        if (items) free(items);
        return false;
    }
    
    printf("\n");
    printf("LOW STOCK ALERTS:\n");
    printf("================================================================================\n");
    printf("%-5s %-20s %-10s %-10s %-15s %-10s\n", "ID", "Name", "Category", "Quantity", "Price", "Threshold");
    printf("================================================================================\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-10s %-10d $%-9.2f %-10d\n",
               items[i].id,
               items[i].name,
               items[i].category,
               items[i].quantity,
               items[i].price,
               items[i].low_stock_threshold);
    }
    
    printf("================================================================================\n");
    printf("Total: %d items below threshold\n", count);
    
    free(items);
    return true;
}

bool item_export_csv(const char *filename) {
    int count = 0;
    Item *items = db_get_all_items(&count);
    
    if (!items) {
        return false;
    }
    
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        free(items);
        return false;
    }
    
    fprintf(fp, "ID,Name,Category,Quantity,Price,LowStockThreshold\n");
    
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,\"%s\",\"%s\",%d,%.2f,%d\n",
                items[i].id,
                items[i].name,
                items[i].category,
                items[i].quantity,
                items[i].price,
                items[i].low_stock_threshold);
    }
    
    fclose(fp);
    free(items);
    
    Session *sess = auth_get_current_user();
    char details[256];
    snprintf(details, sizeof(details), "Exported %d items to CSV: %s", count, filename);
    db_add_audit_log(sess ? sess->id : 0, "EXPORT_CSV", 0, details);
    
    return true;
}

bool item_import_csv(const char *filename) {
    if (!auth_has_permission("manager")) {
        return false;
    }
    
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return false;
    }
    
    char line[512];
    int imported = 0;
    int skipped = 0;
    
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return false;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        char name[51], category[31];
        int quantity, threshold;
        float price;
        
        if (sscanf(line, "%*d,\"%50[^\"]\",\"%30[^\"]\",%d,%f,%d",
                   name, category, &quantity, &price, &threshold) >= 4) {
            
            Item item;
            memset(&item, 0, sizeof(Item));
            strncpy(item.name, name, sizeof(item.name) - 1);
            item.quantity = quantity;
            item.price = price;
            strncpy(item.category, category, sizeof(item.category) - 1);
            item.low_stock_threshold = threshold;
            
            int id = db_add_item(&item);
            if (id > 0) {
                imported++;
                db_add_category(category);
            } else {
                skipped++;
            }
        } else {
            skipped++;
        }
    }
    
    fclose(fp);
    
    Session *sess = auth_get_current_user();
    char details[256];
    snprintf(details, sizeof(details), "Imported %d items from CSV: %s (skipped: %d)", imported, filename, skipped);
    db_add_audit_log(sess ? sess->id : 0, "IMPORT_CSV", 0, details);
    
    return imported > 0;
}

bool item_get_statistics(void) {
    int total_items = db_get_total_items();
    float total_value = db_get_total_value();
    int low_stock_count = db_get_low_stock_count();
    
    int category_count = 0;
    Category *categories = db_get_all_categories(&category_count);
    
    printf("\n");
    printf("==================== INVENTORY STATISTICS ====================\n");
    printf("\n");
    printf("  Total Items:        %d\n", total_items);
    printf("  Total Value:        $%.2f\n", total_value);
    printf("  Low Stock Items:    %d\n", low_stock_count);
    printf("  Categories:         %d\n", category_count);
    printf("\n");
    
    if (categories && category_count > 0) {
        printf("  Category List:\n");
        for (int i = 0; i < category_count; i++) {
            printf("    - %s\n", categories[i].name);
        }
        free(categories);
    }
    
    printf("\n");
    printf("=============================================================\n");
    
    return true;
}

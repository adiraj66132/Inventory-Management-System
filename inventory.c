#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define FILE_NAME "inventory.dat"
#define AUDIT_LOG "audit.log"
#define CONFIG_FILE "config.dat"
#define PAGE_SIZE 15

struct Item {
    int id;
    char name[50];
    int quantity;
    float price;
    char category[30];
    int low_stock_threshold;
};

struct Config {
    int next_id;
    int items_count;
};

// Function prototypes
void addItem();
void viewItems();
void deleteItem();
void updateItem();
void searchItem();
void exportToCSV();
void importFromCSV();
void viewByCategory();
void sortAndView();
void backupData();
void restoreData();
void viewAuditLog();
void setLowStockThreshold();
void menu();
int getNextID();
void incrementID();
void logAudit(const char *action, int item_id, const char *details);

int getNextID() {
    struct Config config = {1, 0};
    FILE *fp = fopen(CONFIG_FILE, "rb");
    if (fp) {
        fread(&config, sizeof(config), 1, fp);
        fclose(fp);
    }
    return config.next_id;
}

void incrementID() {
    struct Config config = {1, 0};
    FILE *fp = fopen(CONFIG_FILE, "rb");
    if (fp) {
        fread(&config, sizeof(config), 1, fp);
        fclose(fp);
    }
    config.next_id++;
    fp = fopen(CONFIG_FILE, "wb");
    if (fp) {
        fwrite(&config, sizeof(config), 1, fp);
        fclose(fp);
    }
}

void logAudit(const char *action, int item_id, const char *details) {
    FILE *fp = fopen(AUDIT_LOG, "a");
    if (!fp) return;
    
    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline
    
    char username[50];
    getlogin_r(username, sizeof(username));
    
    fprintf(fp, "[%s] User: %s | Action: %s | Item ID: %d | %s\n",
            timestamp, username, action, item_id, details);
    fclose(fp);
}

void addItem() {
    struct Item item;
    FILE *fp = fopen(FILE_NAME, "ab");

    if (!fp) {
        mvprintw(10, 5, "Error opening file!");
        getch();
        return;
    }

    echo();
    clear();
    mvprintw(2, 2, "=== Add New Item ===");

    item.id = getNextID();
    mvprintw(4, 2, "Auto-assigned ID: %d", item.id);
    
    mvprintw(5, 2, "Enter Name: ");
    getnstr(item.name, sizeof(item.name) - 1);
    
    mvprintw(6, 2, "Enter Category: ");
    getnstr(item.category, sizeof(item.category) - 1);
    
    mvprintw(7, 2, "Enter Quantity: ");
    scanw("%d", &item.quantity);
    
    mvprintw(8, 2, "Enter Price: ");
    scanw("%f", &item.price);
    
    mvprintw(9, 2, "Enter Low Stock Threshold (0 for none): ");
    scanw("%d", &item.low_stock_threshold);
    
    noecho();

    fwrite(&item, sizeof(item), 1, fp);
    fclose(fp);
    
    incrementID();
    
    char details[200];
    snprintf(details, sizeof(details), "Added: %s, Qty: %d, Price: $%.2f, Category: %s",
             item.name, item.quantity, item.price, item.category);
    logAudit("ADD", item.id, details);

    mvprintw(11, 2, "Item added successfully!");
    getch();
}

void viewItems() {
    struct Item item;
    FILE *fp = fopen(FILE_NAME, "rb");
    
    if (!fp) {
        clear();
        mvprintw(5, 2, "No inventory file found. Add items first!");
        getch();
        return;
    }

    int total_items = 0;
    while (fread(&item, sizeof(item), 1, fp)) {
        total_items++;
    }
    rewind(fp);
    
    int current_page = 0;
    int total_pages = (total_items + PAGE_SIZE - 1) / PAGE_SIZE;
    int ch;
    
    while (1) {
        clear();
        mvprintw(1, 2, "=== Inventory List (Page %d/%d) ===", 
                 current_page + 1, total_pages > 0 ? total_pages : 1);
        mvprintw(3, 2, "ID   Name                 Category         Qty    Price      Status");
        mvprintw(4, 2, "------------------------------------------------------------------------");

        fseek(fp, current_page * PAGE_SIZE * sizeof(item), SEEK_SET);
        
        int row = 5;
        int items_shown = 0;
        while (fread(&item, sizeof(item), 1, fp) && items_shown < PAGE_SIZE) {
            char status[20] = "";
            if (item.low_stock_threshold > 0 && item.quantity <= item.low_stock_threshold) {
                strcpy(status, "[LOW STOCK]");
                attron(A_BOLD | COLOR_PAIR(1));
            }
            
            mvprintw(row++, 2, "%-4d %-20s %-16s %-6d $%-9.2f %s",
                     item.id, item.name, item.category, item.quantity, item.price, status);
            
            if (strlen(status) > 0) {
                attroff(A_BOLD | COLOR_PAIR(1));
            }
            items_shown++;
        }

        mvprintw(row + 2, 2, "Navigation: [←] Prev | [→] Next | [Q] Return");
        
        ch = getch();
        if (ch == 'q' || ch == 'Q') break;
        if (ch == KEY_LEFT && current_page > 0) current_page--;
        if (ch == KEY_RIGHT && current_page < total_pages - 1) current_page++;
    }

    fclose(fp);
}

void deleteItem() {
    struct Item item;
    int id, found = 0;

    echo();
    clear();
    mvprintw(2, 2, "=== Delete Item ===");
    mvprintw(4, 2, "Enter ID of item to delete: ");
    scanw("%d", &id);
    noecho();

    FILE *fp = fopen(FILE_NAME, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    
    char deleted_name[50] = "";
    while (fread(&item, sizeof(item), 1, fp)) {
        if (item.id == id) {
            found = 1;
            strcpy(deleted_name, item.name);
            continue;
        }
        fwrite(&item, sizeof(item), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.dat", FILE_NAME);

    if (found) {
        char details[100];
        snprintf(details, sizeof(details), "Deleted item: %s", deleted_name);
        logAudit("DELETE", id, details);
    }

    mvprintw(6, 2, found ? "Item deleted." : "Item not found.");
    getch();
}

void updateItem() {
    struct Item item;
    int id, found = 0;

    echo();
    clear();
    mvprintw(2, 2, "=== Update Item ===");
    mvprintw(4, 2, "Enter ID of item to update: ");
    scanw("%d", &id);

    FILE *fp = fopen(FILE_NAME, "rb+");
    while (fread(&item, sizeof(item), 1, fp)) {
        if (item.id == id) {
            found = 1;
            mvprintw(6, 2, "Current: %s | Qty: %d | Price: $%.2f | Category: %s",
                     item.name, item.quantity, item.price, item.category);
            mvprintw(8, 2, "Enter new Name: ");
            getnstr(item.name, sizeof(item.name) - 1);
            mvprintw(9, 2, "Enter new Category: ");
            getnstr(item.category, sizeof(item.category) - 1);
            mvprintw(10, 2, "Enter new Quantity: ");
            scanw("%d", &item.quantity);
            mvprintw(11, 2, "Enter new Price: ");
            scanw("%f", &item.price);

            fseek(fp, -sizeof(item), SEEK_CUR);
            fwrite(&item, sizeof(item), 1, fp);
            
            char details[200];
            snprintf(details, sizeof(details), 
                     "Updated: %s, Qty: %d, Price: $%.2f, Category: %s",
                     item.name, item.quantity, item.price, item.category);
            logAudit("UPDATE", id, details);
            break;
        }
    }

    fclose(fp);
    noecho();
    mvprintw(13, 2, found ? "Item updated!" : "Item not found.");
    getch();
}

void searchItem() {
    struct Item item;
    char keyword[50];
    int found = 0;

    echo();
    clear();
    mvprintw(2, 2, "=== Search Item by Name ===");
    mvprintw(4, 2, "Enter name keyword: ");
    getnstr(keyword, sizeof(keyword) - 1);
    noecho();

    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) {
        mvprintw(6, 2, "No inventory file found!");
        getch();
        return;
    }

    int row = 7;
    while (fread(&item, sizeof(item), 1, fp)) {
        if (strcasestr(item.name, keyword)) {
            if (!found) {
                mvprintw(6, 2, "ID   Name                 Category         Qty    Price");
                mvprintw(row - 1, 2, "----------------------------------------------------------------");
            }
            
            // Highlight matching text
            attron(A_BOLD);
            mvprintw(row++, 2, "%-4d %-20s %-16s %-6d $%.2f",
                     item.id, item.name, item.category, item.quantity, item.price);
            attroff(A_BOLD);
            found = 1;
        }
    }

    fclose(fp);
    mvprintw(row + 2, 2, found ? "Search complete." : "No items found.");
    getch();
}

void exportToCSV() {
    struct Item item;
    char filename[100];
    
    echo();
    clear();
    mvprintw(2, 2, "=== Export to CSV ===");
    mvprintw(4, 2, "Enter filename (e.g., export.csv): ");
    getnstr(filename, sizeof(filename) - 1);
    noecho();
    
    FILE *csv = fopen(filename, "w");
    FILE *dat = fopen(FILE_NAME, "rb");
    
    if (!csv || !dat) {
        mvprintw(6, 2, "Error opening files!");
        if (csv) fclose(csv);
        if (dat) fclose(dat);
        getch();
        return;
    }
    
    fprintf(csv, "ID,Name,Category,Quantity,Price,LowStockThreshold\n");
    
    int count = 0;
    while (fread(&item, sizeof(item), 1, dat)) {
        fprintf(csv, "%d,\"%s\",\"%s\",%d,%.2f,%d\n",
                item.id, item.name, item.category, 
                item.quantity, item.price, item.low_stock_threshold);
        count++;
    }
    
    fclose(csv);
    fclose(dat);
    
    logAudit("EXPORT", 0, filename);
    mvprintw(6, 2, "Exported %d items to %s", count, filename);
    getch();
}

void importFromCSV() {
    char filename[100];
    char line[300];
    
    echo();
    clear();
    mvprintw(2, 2, "=== Import from CSV ===");
    mvprintw(4, 2, "Enter filename: ");
    getnstr(filename, sizeof(filename) - 1);
    noecho();
    
    FILE *csv = fopen(filename, "r");
    if (!csv) {
        mvprintw(6, 2, "Error: File not found!");
        getch();
        return;
    }
    
    FILE *dat = fopen(FILE_NAME, "ab");
    
    // Skip header
    fgets(line, sizeof(line), csv);
    
    int count = 0;
    struct Item item;
    while (fgets(line, sizeof(line), csv)) {
        char name_buf[50], cat_buf[30];
        if (sscanf(line, "%d,\"%49[^\"]\",\"%29[^\"]\",%d,%f,%d",
                   &item.id, name_buf, cat_buf, 
                   &item.quantity, &item.price, &item.low_stock_threshold) == 6) {
            strncpy(item.name, name_buf, sizeof(item.name) - 1);
            strncpy(item.category, cat_buf, sizeof(item.category) - 1);
            fwrite(&item, sizeof(item), 1, dat);
            count++;
        }
    }
    
    fclose(csv);
    fclose(dat);
    
    logAudit("IMPORT", 0, filename);
    mvprintw(6, 2, "Imported %d items from %s", count, filename);
    getch();
}

void viewByCategory() {
    struct Item item;
    char category[30];
    int found = 0;

    echo();
    clear();
    mvprintw(2, 2, "=== View by Category ===");
    mvprintw(4, 2, "Enter category: ");
    getnstr(category, sizeof(category) - 1);
    noecho();

    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) {
        mvprintw(6, 2, "No inventory file found!");
        getch();
        return;
    }

    int row = 7;
    while (fread(&item, sizeof(item), 1, fp)) {
        if (strcasestr(item.category, category)) {
            if (!found) {
                mvprintw(6, 2, "ID   Name                 Quantity   Price");
                mvprintw(row - 1, 2, "------------------------------------------------");
            }
            mvprintw(row++, 2, "%-4d %-20s %-10d $%.2f",
                     item.id, item.name, item.quantity, item.price);
            found = 1;
        }
    }

    fclose(fp);
    mvprintw(row + 2, 2, found ? "Done." : "No items in this category.");
    getch();
}

void sortAndView() {
    struct Item items[1000];
    int count = 0;
    
    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) {
        clear();
        mvprintw(5, 2, "No inventory file found!");
        getch();
        return;
    }
    
    while (fread(&items[count], sizeof(struct Item), 1, fp) && count < 1000) {
        count++;
    }
    fclose(fp);
    
    clear();
    mvprintw(2, 2, "=== Sort Options ===");
    mvprintw(4, 2, "1. Sort by Name");
    mvprintw(5, 2, "2. Sort by Quantity (Low to High)");
    mvprintw(6, 2, "3. Sort by Price (Low to High)");
    mvprintw(7, 2, "4. Sort by Category");
    mvprintw(9, 2, "Choose option: ");
    
    int choice = getch() - '0';
    
    // Simple bubble sort
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int swap = 0;
            switch (choice) {
                case 1: swap = strcmp(items[j].name, items[j+1].name) > 0; break;
                case 2: swap = items[j].quantity > items[j+1].quantity; break;
                case 3: swap = items[j].price > items[j+1].price; break;
                case 4: swap = strcmp(items[j].category, items[j+1].category) > 0; break;
            }
            if (swap) {
                struct Item temp = items[j];
                items[j] = items[j+1];
                items[j+1] = temp;
            }
        }
    }
    
    clear();
    mvprintw(1, 2, "=== Sorted Inventory ===");
    mvprintw(3, 2, "ID   Name                 Category         Qty    Price");
    mvprintw(4, 2, "----------------------------------------------------------------");
    
    for (int i = 0; i < count && i < 20; i++) {
        mvprintw(5 + i, 2, "%-4d %-20s %-16s %-6d $%.2f",
                 items[i].id, items[i].name, items[i].category, 
                 items[i].quantity, items[i].price);
    }
    
    mvprintw(count + 7, 2, "Press any key to return...");
    getch();
}

void backupData() {
    char backup_name[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    snprintf(backup_name, sizeof(backup_name), 
             "inventory_backup_%04d%02d%02d_%02d%02d%02d.dat",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE *src = fopen(FILE_NAME, "rb");
    FILE *dst = fopen(backup_name, "wb");
    
    if (!src) {
        clear();
        mvprintw(5, 2, "No inventory file to backup!");
        getch();
        return;
    }
    
    if (!dst) {
        fclose(src);
        clear();
        mvprintw(5, 2, "Error creating backup file!");
        getch();
        return;
    }
    
    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    
    logAudit("BACKUP", 0, backup_name);
    
    clear();
    mvprintw(5, 2, "Backup created: %s", backup_name);
    getch();
}

void restoreData() {
    char filename[100];
    
    echo();
    clear();
    mvprintw(2, 2, "=== Restore from Backup ===");
    mvprintw(4, 2, "Enter backup filename: ");
    getnstr(filename, sizeof(filename) - 1);
    noecho();
    
    FILE *src = fopen(filename, "rb");
    if (!src) {
        mvprintw(6, 2, "Backup file not found!");
        getch();
        return;
    }
    
    FILE *dst = fopen(FILE_NAME, "wb");
    
    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }
    
    fclose(src);
    fclose(dst);
    
    logAudit("RESTORE", 0, filename);
    
    mvprintw(6, 2, "Data restored from %s", filename);
    getch();
}

void viewAuditLog() {
    clear();
    mvprintw(1, 2, "=== Audit Log ===");
    
    FILE *fp = fopen(AUDIT_LOG, "r");
    if (!fp) {
        mvprintw(3, 2, "No audit log found.");
        getch();
        return;
    }
    
    char line[300];
    int row = 3;
    int total_lines = 0;
    
    // Count lines
    while (fgets(line, sizeof(line), fp)) {
        total_lines++;
    }
    rewind(fp);
    
    // Skip to last 20 entries
    int skip = total_lines > 20 ? total_lines - 20 : 0;
    for (int i = 0; i < skip; i++) {
        fgets(line, sizeof(line), fp);
    }
    
    while (fgets(line, sizeof(line), fp) && row < LINES - 2) {
        line[strcspn(line, "\n")] = 0;
        mvprintw(row++, 2, "%.*s", COLS - 4, line);
    }
    
    fclose(fp);
    mvprintw(row + 1, 2, "Press any key to return...");
    getch();
}

void setLowStockThreshold() {
    struct Item item;
    int id, threshold, found = 0;

    echo();
    clear();
    mvprintw(2, 2, "=== Set Low Stock Threshold ===");
    mvprintw(4, 2, "Enter Item ID: ");
    scanw("%d", &id);
    mvprintw(5, 2, "Enter threshold quantity: ");
    scanw("%d", &threshold);
    noecho();

    FILE *fp = fopen(FILE_NAME, "rb+");
    while (fread(&item, sizeof(item), 1, fp)) {
        if (item.id == id) {
            found = 1;
            item.low_stock_threshold = threshold;
            fseek(fp, -sizeof(item), SEEK_CUR);
            fwrite(&item, sizeof(item), 1, fp);
            
            char details[100];
            snprintf(details, sizeof(details), 
                     "Set threshold to %d for item: %s", threshold, item.name);
            logAudit("THRESHOLD", id, details);
            break;
        }
    }

    fclose(fp);
    mvprintw(7, 2, found ? "Threshold updated!" : "Item not found.");
    getch();
}

void menu() {
    char *options[] = {
        "1. Add Item",
        "2. View Items",
        "3. Update Item",
        "4. Delete Item",
        "5. Search Item",
        "6. Export to CSV",
        "7. Import from CSV",
        "8. View by Category",
        "9. Sort & View",
        "A. Set Low Stock Alert",
        "B. Backup Data",
        "C. Restore Data",
        "D. View Audit Log",
        "E. Exit"
    };
    int num_options = 14;
    int choice = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(1, 2, "╔════════════════════════════════════╗");
        mvprintw(2, 2, "║   INVENTORY MANAGEMENT SYSTEM      ║");
        mvprintw(3, 2, "╚════════════════════════════════════╝");
        
        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                attron(A_REVERSE | A_BOLD);
            }
            mvprintw(5 + i, 4, "%s", options[i]);
            if (i == choice) {
                attroff(A_REVERSE | A_BOLD);
            }
        }
        
        mvprintw(21, 2, "Use ↑↓ to navigate, Enter to select");

        ch = getch();
        switch (ch) {
            case KEY_UP:
                choice = (choice + num_options - 1) % num_options;
                break;
            case KEY_DOWN:
                choice = (choice + 1) % num_options;
                break;
            case 10: // Enter
                switch (choice) {
                    case 0: addItem(); break;
                    case 1: viewItems(); break;
                    case 2: updateItem(); break;
                    case 3: deleteItem(); break;
                    case 4: searchItem(); break;
                    case 5: exportToCSV(); break;
                    case 6: importFromCSV(); break;
                    case 7: viewByCategory(); break;
                    case 8: sortAndView(); break;
                    case 9: setLowStockThreshold(); break;
                    case 10: backupData(); break;
                    case 11: restoreData(); break;
                    case 12: viewAuditLog(); break;
                    case 13: endwin(); exit(0);
                }
                break;
        }
    }
}

int main() {
    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    menu();
    endwin();
    return 0;
}

#include "../include/ui.h"
#include "../include/db.h"
#include "../include/auth.h"
#include "../include/item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int current_page = 0;
static int items_per_page = 15;
static Item *displayed_items = NULL;
static int displayed_count = 0;

void ui_init(void) {
    initscr();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLUE);
    curs_set(1);
    noecho();
    keypad(stdscr, TRUE);
    cbreak();
}

void ui_cleanup(void) {
    endwin();
}

void ui_login_screen(void) {
    clear();
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    attron(COLOR_PAIR(1));
    mvprintw(height/4, (width - 40)/2, "========================================");
    mvprintw(height/4 + 1, (width - 40)/2, "       INVENTORY MANAGEMENT SYSTEM       ");
    mvprintw(height/4 + 2, (width - 40)/2, "========================================");
    attroff(COLOR_PAIR(1));
    
    mvprintw(height/2, (width - 20)/2, "LOGIN");
    
    char username[50] = {0};
    char password[50] = {0};
    
    mvprintw(height/2 + 3, (width - 30)/2, "Username: ");
    echo();
    getnstr(username, sizeof(username) - 1);
    
    mvprintw(height/2 + 4, (width - 30)/2, "Password: ");
    noecho();
    getnstr(password, sizeof(password) - 1);
    
    if (auth_login(username, password)) {
        mvprintw(height/2 + 6, (width - 20)/2, "Login successful!");
        refresh();
        napms(1000);
    } else {
        attron(COLOR_PAIR(3));
        mvprintw(height/2 + 6, (width - 20)/2, "Login failed!");
        attroff(COLOR_PAIR(3));
        refresh();
        napms(1500);
        ui_login_screen();
    }
    
    noecho();
}

static void display_items_page(Item *items, int total, int page, const char *title) {
    clear();
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    int start = page * items_per_page;
    int end = start + items_per_page;
    if (end > total) end = total;
    
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, (width - strlen(title))/2, "%s", title);
    attroff(COLOR_PAIR(1) | A_BOLD);
    
    mvprintw(3, 2, "%-5s %-20s %-12s %-8s %-10s %-10s", "ID", "Name", "Category", "Qty", "Price", "Threshold");
    
    for (int i = start; i < end; i++) {
        int row = 5 + (i - start);
        if (items[i].low_stock_threshold > 0 && items[i].quantity <= items[i].low_stock_threshold) {
            attron(COLOR_PAIR(3));
        }
        mvprintw(row, 2, "%-5d %-20s %-12s %-8d $%-9.2f %-10d",
                items[i].id,
                items[i].name,
                items[i].category,
                items[i].quantity,
                items[i].price,
                items[i].low_stock_threshold);
        if (items[i].low_stock_threshold > 0 && items[i].quantity <= items[i].low_stock_threshold) {
            attroff(COLOR_PAIR(3));
        }
    }
    
    int total_pages = (total + items_per_page - 1) / items_per_page;
    mvprintw(height - 3, 2, "Page %d/%d | Total: %d items", page + 1, total_pages, total);
    mvprintw(height - 2, 2, "Arrow keys: Navigate | Q: Quit | N: Next | P: Previous");
}

void ui_add_item_screen(void) {
    if (!auth_has_permission("manager")) {
        clear();
        mvprintw(10, 5, "Permission denied!");
        getch();
        return;
    }
    
    clear();
    
    Item item;
    memset(&item, 0, sizeof(Item));
    
    echo();
    mvprintw(2, 2, "=== Add New Item ===");
    
    mvprintw(4, 2, "Name: ");
    getnstr(item.name, 50);
    
    mvprintw(5, 2, "Quantity: ");
    char qty_str[20];
    getnstr(qty_str, 19);
    item.quantity = atoi(qty_str);
    
    mvprintw(6, 2, "Price: ");
    char price_str[20];
    getnstr(price_str, 19);
    item.price = atof(price_str);
    
    mvprintw(7, 2, "Category: ");
    getnstr(item.category, 30);
    
    mvprintw(8, 2, "Low Stock Threshold: ");
    char thresh_str[20];
    getnstr(thresh_str, 19);
    item.low_stock_threshold = atoi(thresh_str);
    
    noecho();
    
    if (strlen(item.name) == 0) {
        mvprintw(10, 2, "Error: Name is required!");
        getch();
        return;
    }
    
    if (item_add(item.name, item.quantity, item.price, 
                 strlen(item.category) > 0 ? item.category : "Uncategorized", 
                 item.low_stock_threshold)) {
        mvprintw(10, 2, "Item added successfully!");
    } else {
        mvprintw(10, 2, "Error: Failed to add item!");
    }
    
    refresh();
    napms(1500);
}

void ui_view_items_screen(void) {
    int count = 0;
    Item *items = db_get_all_items(&count);
    
    if (!items || count == 0) {
        if (items) free(items);
        clear();
        mvprintw(10, 5, "No items found!");
        getch();
        return;
    }
    
    current_page = 0;
    displayed_items = items;
    displayed_count = count;
    
    int ch;
    while (1) {
        display_items_page(displayed_items, displayed_count, current_page, "View Items");
        
        ch = getch();
        
        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == KEY_RIGHT || ch == 'n' || ch == 'N') {
            int total_pages = (displayed_count + items_per_page - 1) / items_per_page;
            if (current_page < total_pages - 1) {
                current_page++;
            }
        } else if (ch == KEY_LEFT || ch == 'p' || ch == 'P') {
            if (current_page > 0) {
                current_page--;
            }
        }
    }
    
    free(displayed_items);
    displayed_items = NULL;
    displayed_count = 0;
}

void ui_update_item_screen(void) {
    if (!auth_has_permission("manager")) {
        clear();
        mvprintw(10, 5, "Permission denied!");
        getch();
        return;
    }
    
    clear();
    echo();
    
    mvprintw(2, 2, "=== Update Item ===");
    mvprintw(4, 2, "Enter Item ID: ");
    char id_str[20];
    getnstr(id_str, 19);
    int id = atoi(id_str);
    
    noecho();
    
    Item *item = db_get_item(id);
    if (!item) {
        clear();
        mvprintw(10, 5, "Item not found!");
        getch();
        return;
    }
    
    clear();
    echo();
    
    mvprintw(2, 2, "=== Update Item #%d ===", id);
    
    mvprintw(4, 2, "Name [%s]: ", item->name);
    char name[51];
    getnstr(name, 50);
    if (strlen(name) == 0) strncpy(name, item->name, 50);
    
    mvprintw(5, 2, "Quantity [%d]: ", item->quantity);
    char qty_str[20];
    getnstr(qty_str, 19);
    int quantity = atoi(qty_str);
    if (quantity == 0 && qty_str[0] != '0') quantity = item->quantity;
    
    mvprintw(6, 2, "Price [%.2f]: ", item->price);
    char price_str[20];
    getnstr(price_str, 19);
    float price = atof(price_str);
    if (price == 0 && price_str[0] != '0') price = item->price;
    
    mvprintw(7, 2, "Category [%s]: ", item->category);
    char category[31];
    getnstr(category, 30);
    if (strlen(category) == 0) strncpy(category, item->category, 30);
    
    mvprintw(8, 2, "Low Stock Threshold [%d]: ", item->low_stock_threshold);
    char thresh_str[20];
    getnstr(thresh_str, 19);
    int threshold = atoi(thresh_str);
    if (threshold == 0 && thresh_str[0] != '0') threshold = item->low_stock_threshold;
    
    noecho();
    
    if (item_update(id, name, quantity, price, category, threshold)) {
        mvprintw(10, 2, "Item updated successfully!");
    } else {
        mvprintw(10, 2, "Error: Failed to update item!");
    }
    
    free(item);
    refresh();
    napms(1500);
}

void ui_delete_item_screen(void) {
    if (!auth_has_permission("admin")) {
        clear();
        mvprintw(10, 5, "Permission denied! Admin access required.");
        getch();
        return;
    }
    
    clear();
    echo();
    
    mvprintw(2, 2, "=== Delete Item ===");
    mvprintw(4, 2, "Enter Item ID to delete: ");
    char id_str[20];
    getnstr(id_str, 19);
    int id = atoi(id_str);
    
    noecho();
    
    Item *item = db_get_item(id);
    if (!item) {
        clear();
        mvprintw(10, 5, "Item not found!");
        getch();
        return;
    }
    
    mvprintw(6, 2, "Delete '%s'? (y/n): ", item->name);
    echo();
    char confirm[5];
    getnstr(confirm, 4);
    noecho();
    
    if (confirm[0] == 'y' || confirm[0] == 'Y') {
        if (item_delete(id)) {
            mvprintw(8, 2, "Item deleted successfully!");
        } else {
            mvprintw(8, 2, "Error: Failed to delete item!");
        }
    }
    
    free(item);
    refresh();
    napms(1500);
}

void ui_search_screen(void) {
    clear();
    echo();
    
    mvprintw(2, 2, "=== Search Items ===");
    mvprintw(4, 2, "Search term: ");
    char query[100];
    getnstr(query, 99);
    
    noecho();
    
    int count = 0;
    Item *items = db_search_items(query, &count);
    
    if (!items || count == 0) {
        if (items) free(items);
        clear();
        mvprintw(10, 5, "No items found!");
        getch();
        return;
    }
    
    current_page = 0;
    displayed_items = items;
    displayed_count = count;
    
    int ch;
    while (1) {
        display_items_page(displayed_items, displayed_count, current_page, "Search Results");
        
        ch = getch();
        
        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == KEY_RIGHT || ch == 'n' || ch == 'N') {
            int total_pages = (displayed_count + items_per_page - 1) / items_per_page;
            if (current_page < total_pages - 1) {
                current_page++;
            }
        } else if (ch == KEY_LEFT || ch == 'p' || ch == 'P') {
            if (current_page > 0) {
                current_page--;
            }
        }
    }
    
    free(displayed_items);
    displayed_items = NULL;
    displayed_count = 0;
}

void ui_category_screen(void) {
    clear();
    
    int count = 0;
    Category *categories = db_get_all_categories(&count);
    
    if (!categories || count == 0) {
        if (categories) free(categories);
        mvprintw(10, 5, "No categories found!");
        getch();
        return;
    }
    
    echo();
    mvprintw(2, 2, "=== Select Category ===");
    
    for (int i = 0; i < count; i++) {
        mvprintw(4 + i, 2, "%d. %s", i + 1, categories[i].name);
    }
    
    mvprintw(4 + count + 2, 2, "Enter category number: ");
    char choice[10];
    getnstr(choice, 9);
    noecho();
    
    int idx = atoi(choice) - 1;
    if (idx >= 0 && idx < count) {
        int item_count = 0;
        Item *items = db_get_items_by_category(categories[idx].name, &item_count);
        
        if (items && item_count > 0) {
            current_page = 0;
            displayed_items = items;
            displayed_count = item_count;
            
            int ch;
            while (1) {
                display_items_page(displayed_items, displayed_count, current_page, categories[idx].name);
                
                ch = getch();
                
                if (ch == 'q' || ch == 'Q') {
                    break;
                } else if (ch == KEY_RIGHT || ch == 'n' || ch == 'N') {
                    int total_pages = (displayed_count + items_per_page - 1) / items_per_page;
                    if (current_page < total_pages - 1) {
                        current_page++;
                    }
                } else if (ch == KEY_LEFT || ch == 'p' || ch == 'P') {
                    if (current_page > 0) {
                        current_page--;
                    }
                }
            }
            
            free(displayed_items);
            displayed_items = NULL;
            displayed_count = 0;
        } else {
            if (items) free(items);
            mvprintw(10, 5, "No items in this category!");
            getch();
        }
    }
    
    free(categories);
}

void ui_export_screen(void) {
    clear();
    echo();
    
    mvprintw(2, 2, "=== Export to CSV ===");
    mvprintw(4, 2, "Enter filename: ");
    char filename[256];
    getnstr(filename, 255);
    
    noecho();
    
    if (item_export_csv(filename)) {
        mvprintw(6, 2, "Export successful!");
    } else {
        mvprintw(6, 2, "Export failed!");
    }
    
    refresh();
    napms(1500);
}

void ui_import_screen(void) {
    if (!auth_has_permission("manager")) {
        clear();
        mvprintw(10, 5, "Permission denied!");
        getch();
        return;
    }
    
    clear();
    echo();
    
    mvprintw(2, 2, "=== Import from CSV ===");
    mvprintw(4, 2, "Enter filename: ");
    char filename[256];
    getnstr(filename, 255);
    
    noecho();
    
    if (item_import_csv(filename)) {
        mvprintw(6, 2, "Import successful!");
    } else {
        mvprintw(6, 2, "Import failed! Check file format.");
    }
    
    refresh();
    napms(1500);
}

void ui_statistics_screen(void) {
    item_get_statistics();
    getch();
}

void ui_audit_log_screen(void) {
    clear();
    
    int count = 0;
    AuditLog *logs = db_get_audit_logs(&count);
    
    if (!logs || count == 0) {
        if (logs) free(logs);
        clear();
        mvprintw(10, 5, "No audit logs found!");
        getch();
        return;
    }
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    int ch;
    int page = 0;
    int total_pages = (count + items_per_page - 1) / items_per_page;
    
    while (1) {
        clear();
        
        mvprintw(1, (width - 30)/2, "=== Audit Log ===");
        
        int start = page * items_per_page;
        int end = start + items_per_page;
        if (end > count) end = count;
        
        mvprintw(3, 2, "%-5s %-20s %-12s %-8s %-30s", "ID", "Timestamp", "Action", "Item ID", "Details");
        
        for (int i = start; i < end; i++) {
            mvprintw(5 + (i - start), 2, "%-5d %-20s %-12s %-8d %-30s",
                    logs[i].id,
                    logs[i].timestamp,
                    logs[i].action,
                    logs[i].item_id,
                    logs[i].details);
        }
        
        mvprintw(height - 3, 2, "Page %d/%d | Total: %d logs", page + 1, total_pages, count);
        mvprintw(height - 2, 2, "Arrow keys: Navigate | Q: Quit");
        
        ch = getch();
        
        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == KEY_RIGHT || ch == 'n' || ch == 'N') {
            if (page < total_pages - 1) {
                page++;
            }
        } else if (ch == KEY_LEFT || ch == 'p' || ch == 'P') {
            if (page > 0) {
                page--;
            }
        }
    }
    
    free(logs);
}

void ui_user_management_screen(void) {
    if (!auth_has_permission("admin")) {
        clear();
        mvprintw(10, 5, "Permission denied! Admin access required.");
        getch();
        return;
    }
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    int ch;
    while (1) {
        clear();
        
        mvprintw(1, (width - 30)/2, "=== User Management ===");
        
        int count = 0;
        User *users = db_get_all_users(&count);
        
        if (users && count > 0) {
            mvprintw(3, 2, "%-5s %-20s %-10s %-20s", "ID", "Username", "Role", "Created");
            
            for (int i = 0; i < count && i < height - 6; i++) {
                mvprintw(5 + i, 2, "%-5d %-20s %-10s %-20s",
                        users[i].id,
                        users[i].username,
                        users[i].role,
                        users[i].created_at);
            }
            
            free(users);
        }
        
        mvprintw(height - 4, 2, "A: Add User | D: Delete User | Q: Quit");
        
        ch = getch();
        
        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == 'a' || ch == 'A') {
            clear();
            echo();
            
            char username[50] = {0};
            char password[50] = {0};
            char role[16] = "viewer";
            
            mvprintw(2, 2, "=== Add User ===");
            mvprintw(4, 2, "Username: ");
            getnstr(username, 49);
            
            mvprintw(5, 2, "Password: ");
            getnstr(password, 49);
            
            mvprintw(6, 2, "Role (admin/manager/viewer): ");
            getnstr(role, 15);
            
            noecho();
            
            if (strcmp(role, "admin") != 0 && strcmp(role, "manager") != 0 && strcmp(role, "viewer") != 0) {
                strcpy(role, "viewer");
            }
            
            if (auth_create_user(username, password, role)) {
                mvprintw(8, 2, "User created successfully!");
            } else {
                mvprintw(8, 2, "Failed to create user!");
            }
            
            refresh();
            napms(1500);
        } else if (ch == 'd' || ch == 'D') {
            clear();
            echo();
            
            mvprintw(2, 2, "=== Delete User ===");
            mvprintw(4, 2, "Enter User ID to delete: ");
            char id_str[20];
            getnstr(id_str, 19);
            int user_id = atoi(id_str);
            
            noecho();
            
            if (auth_delete_user(user_id)) {
                mvprintw(6, 2, "User deleted successfully!");
            } else {
                mvprintw(6, 2, "Failed to delete user!");
            }
            
            refresh();
            napms(1500);
        }
    }
}

void ui_main_menu(void) {
    int height, width;
    getmaxyx(stdscr, height, width);
    (void)height;
    
    Session *sess = auth_get_current_user();
    
    const char *options[] = {
        "1. View All Items",
        "2. Add New Item",
        "3. Update Item",
        "4. Delete Item",
        "5. Search Items",
        "6. View by Category",
        "7. Low Stock Items",
        "8. Statistics",
        "9. Export to CSV",
        "10. Import from CSV",
        "11. Audit Log",
        "12. User Management",
        "13. Logout",
        "14. Exit"
    };
    
    int num_options = 14;
    int selected = 0;
    
    int ch;
    while (1) {
        clear();
        
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(1, (width - 40)/2, "========================================");
        mvprintw(2, (width - 40)/2, "       INVENTORY MANAGEMENT SYSTEM       ");
        mvprintw(3, (width - 40)/2, "========================================");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        if (sess) {
            mvprintw(5, 2, "Logged in as: %s (%s)", sess->username, sess->role);
        }
        
        for (int i = 0; i < num_options; i++) {
            if (i == selected) {
                attron(COLOR_PAIR(2) | A_REVERSE);
                mvprintw(7 + i, 4, "%s", options[i]);
                attroff(COLOR_PAIR(2) | A_REVERSE);
            } else {
                mvprintw(7 + i, 4, "%s", options[i]);
            }
        }
        
        ch = getch();
        
        if (ch == KEY_UP) {
            selected = (selected - 1 + num_options) % num_options;
        } else if (ch == KEY_DOWN) {
            selected = (selected + 1) % num_options;
        } else if (ch == '\n' || ch == '\r') {
            break;
        }
    }
    
    switch (selected) {
        case 0: ui_view_items_screen(); break;
        case 1: ui_add_item_screen(); break;
        case 2: ui_update_item_screen(); break;
        case 3: ui_delete_item_screen(); break;
        case 4: ui_search_screen(); break;
        case 5: ui_category_screen(); break;
        case 6: 
            clear();
            if (!item_list_low_stock()) {
                mvprintw(10, 5, "No low stock items!");
                getch();
            }
            break;
        case 7: ui_statistics_screen(); break;
        case 8: ui_export_screen(); break;
        case 9: ui_import_screen(); break;
        case 10: ui_audit_log_screen(); break;
        case 11: ui_user_management_screen(); break;
        case 12:
            auth_logout();
            ui_login_screen();
            if (auth_get_current_user()) {
                ui_main_menu();
            }
            return;
        case 13:
            return;
    }
    
    if (selected != 12 && selected != 13 && auth_get_current_user()) {
        ui_main_menu();
    }
}

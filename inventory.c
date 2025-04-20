#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "inventory.dat"

struct Item {
    int id;
    char name[50];
    int quantity;
    float price;
};

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

    mvprintw(4, 2, "Enter ID: ");
    scanw("%d", &item.id);
    mvprintw(5, 2, "Enter Name: ");
    getstr(item.name);
    mvprintw(6, 2, "Enter Quantity: ");
    scanw("%d", &item.quantity);
    mvprintw(7, 2, "Enter Price: ");
    scanw("%f", &item.price);
    noecho();

    fwrite(&item, sizeof(item), 1, fp);
    fclose(fp);

    mvprintw(9, 2, "Item added successfully!");
    getch();
}

void viewItems() {
    struct Item item;
    FILE *fp = fopen(FILE_NAME, "rb");

    clear();
    mvprintw(1, 2, "=== Inventory List ===");
    mvprintw(3, 2, "ID   Name                 Quantity   Price");
    mvprintw(4, 2, "----------------------------------------------");

    int row = 5;
    while (fread(&item, sizeof(item), 1, fp)) {
        mvprintw(row++, 2, "%-4d %-20s %-10d $%.2f",
                 item.id, item.name, item.quantity, item.price);
    }

    fclose(fp);
    mvprintw(row + 2, 2, "Press any key to return...");
    getch();
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

    while (fread(&item, sizeof(item), 1, fp)) {
        if (item.id == id) {
            found = 1;
            continue;
        }
        fwrite(&item, sizeof(item), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.dat", FILE_NAME);

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
            mvprintw(6, 2, "Enter new Name: ");
            getstr(item.name);
            mvprintw(7, 2, "Enter new Quantity: ");
            scanw("%d", &item.quantity);
            mvprintw(8, 2, "Enter new Price: ");
            scanw("%f", &item.price);

            fseek(fp, -sizeof(item), SEEK_CUR);
            fwrite(&item, sizeof(item), 1, fp);
            break;
        }
    }

    fclose(fp);
    noecho();
    mvprintw(10, 2, found ? "Item updated!" : "Item not found.");
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
    getstr(keyword);
    noecho();

    FILE *fp = fopen(FILE_NAME, "rb");

    int row = 6;
    while (fread(&item, sizeof(item), 1, fp)) {
        if (strstr(item.name, keyword)) {
            if (!found) {
                mvprintw(5, 2, "ID   Name                 Quantity   Price");
            }
            mvprintw(row++, 2, "%-4d %-20s %-10d $%.2f",
                     item.id, item.name, item.quantity, item.price);
            found = 1;
        }
    }

    fclose(fp);
    mvprintw(row + 2, 2, found ? "Search complete." : "No items found.");
    getch();
}

void menu() {
    char *options[] = {
        "Add Item", "View Items", "Update Item",
        "Delete Item", "Search Item", "Exit"
    };
    int choice = 0;
    int ch;

    while (1) {
        clear();
        mvprintw(1, 2, "=== Inventory Management ===");
        for (int i = 0; i < 6; i++) {
            if (i == choice) {
                attron(A_REVERSE);
            }
            mvprintw(3 + i, 4, options[i]);
            attroff(A_REVERSE);
        }

        ch = getch();
        switch (ch) {
            case KEY_UP:
                choice = (choice + 5) % 6;
                break;
            case KEY_DOWN:
                choice = (choice + 1) % 6;
                break;
            case 10: // Enter
                switch (choice) {
                    case 0: addItem(); break;
                    case 1: viewItems(); break;
                    case 2: updateItem(); break;
                    case 3: deleteItem(); break;
                    case 4: searchItem(); break;
                    case 5: endwin(); exit(0);
                }
                break;
        }
    }
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    menu();
    endwin();
    return 0;
}

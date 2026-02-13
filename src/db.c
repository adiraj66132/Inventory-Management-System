#include "../include/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static sqlite3 *db = NULL;
static char db_path[256] = "data/inventory.db";

bool db_init(const char *db_path_param) {
    if (db_path_param) {
        strncpy(db_path, db_path_param, sizeof(db_path) - 1);
    }
    
    int rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_exec(db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON", NULL, NULL, NULL);
    
    return db_create_tables();
}

void db_close() {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

char* db_get_db_path(void) {
    return db_path;
}

void db_set_db_path(const char *path) {
    strncpy(db_path, path, sizeof(db_path) - 1);
}

bool db_create_tables(void) {
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS categories ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name TEXT UNIQUE NOT NULL"
        ");"
        
        "CREATE TABLE IF NOT EXISTS users ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    username TEXT UNIQUE NOT NULL,"
        "    password_hash TEXT NOT NULL,"
        "    role TEXT DEFAULT 'viewer' CHECK(role IN ('admin', 'manager', 'viewer')),"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        
        "CREATE TABLE IF NOT EXISTS items ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name TEXT NOT NULL,"
        "    quantity INTEGER DEFAULT 0,"
        "    price REAL DEFAULT 0.0,"
        "    category TEXT,"
        "    low_stock_threshold INTEGER DEFAULT 0,"
        "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        
        "CREATE INDEX IF NOT EXISTS idx_items_name ON items(name);"
        "CREATE INDEX IF NOT EXISTS idx_items_category ON items(category);"
        "CREATE INDEX IF NOT EXISTS idx_items_quantity ON items(quantity);"
        
        "CREATE TABLE IF NOT EXISTS audit_log ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    user_id INTEGER,"
        "    action TEXT NOT NULL,"
        "    item_id INTEGER,"
        "    details TEXT,"
        "    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    FOREIGN KEY (user_id) REFERENCES users(id)"
        ");"
        
        "CREATE TABLE IF NOT EXISTS db_version ("
        "    version INTEGER PRIMARY KEY DEFAULT 1"
        ");"
        
        "INSERT OR IGNORE INTO db_version (version) VALUES (1);";
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

bool db_migrate(void) {
    return db_create_tables();
}

int db_add_item(Item *item) {
    const char *sql = "INSERT INTO items (name, quantity, price, category, low_stock_threshold) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, item->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, item->quantity);
    sqlite3_bind_double(stmt, 3, item->price);
    sqlite3_bind_text(stmt, 4, item->category, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, item->low_stock_threshold);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        return -1;
    }
    
    return (int)sqlite3_last_insert_rowid(db);
}

Item* db_get_item(int id) {
    const char *sql = "SELECT id, name, quantity, price, category, low_stock_threshold, created_at, updated_at FROM items WHERE id = ?";
    sqlite3_stmt *stmt;
    Item *item = NULL;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        item = malloc(sizeof(Item));
        if (item) {
            item->id = sqlite3_column_int(stmt, 0);
            strncpy(item->name, (const char*)sqlite3_column_text(stmt, 1), 50);
            item->quantity = sqlite3_column_int(stmt, 2);
            item->price = (float)sqlite3_column_double(stmt, 3);
            strncpy(item->category, (const char*)sqlite3_column_text(stmt, 4), 30);
            item->low_stock_threshold = sqlite3_column_int(stmt, 5);
            strncpy(item->created_at, (const char*)sqlite3_column_text(stmt, 6), 31);
            strncpy(item->updated_at, (const char*)sqlite3_column_text(stmt, 7), 31);
        }
    }
    
    sqlite3_finalize(stmt);
    return item;
}

Item* db_get_all_items(int *count) {
    const char *sql = "SELECT id, name, quantity, price, category, low_stock_threshold, created_at, updated_at FROM items ORDER BY id";
    sqlite3_stmt *stmt;
    Item *items = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    int capacity = 100;
    items = malloc(capacity * sizeof(Item));
    if (!items) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            Item *temp = realloc(items, capacity * sizeof(Item));
            if (!temp) {
                free(items);
                sqlite3_finalize(stmt);
                return NULL;
            }
            items = temp;
        }
        
        Item *item = &items[*count];
        item->id = sqlite3_column_int(stmt, 0);
        strncpy(item->name, (const char*)sqlite3_column_text(stmt, 1), 50);
        item->quantity = sqlite3_column_int(stmt, 2);
        item->price = (float)sqlite3_column_double(stmt, 3);
        strncpy(item->category, (const char*)sqlite3_column_text(stmt, 4), 30);
        item->low_stock_threshold = sqlite3_column_int(stmt, 5);
        strncpy(item->created_at, (const char*)sqlite3_column_text(stmt, 6), 31);
        strncpy(item->updated_at, (const char*)sqlite3_column_text(stmt, 7), 31);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return items;
}

Item* db_search_items(const char *query, int *count) {
    const char *sql = "SELECT id, name, quantity, price, category, low_stock_threshold, created_at, updated_at FROM items WHERE LOWER(name) LIKE LOWER(?) ORDER BY id";
    sqlite3_stmt *stmt;
    Item *items = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    char search_term[100];
    snprintf(search_term, sizeof(search_term), "%%%s%%", query);
    sqlite3_bind_text(stmt, 1, search_term, -1, SQLITE_TRANSIENT);
    
    int capacity = 100;
    items = malloc(capacity * sizeof(Item));
    if (!items) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            Item *temp = realloc(items, capacity * sizeof(Item));
            if (!temp) {
                free(items);
                sqlite3_finalize(stmt);
                return NULL;
            }
            items = temp;
        }
        
        Item *item = &items[*count];
        item->id = sqlite3_column_int(stmt, 0);
        strncpy(item->name, (const char*)sqlite3_column_text(stmt, 1), 50);
        item->quantity = sqlite3_column_int(stmt, 2);
        item->price = (float)sqlite3_column_double(stmt, 3);
        strncpy(item->category, (const char*)sqlite3_column_text(stmt, 4), 30);
        item->low_stock_threshold = sqlite3_column_int(stmt, 5);
        strncpy(item->created_at, (const char*)sqlite3_column_text(stmt, 6), 31);
        strncpy(item->updated_at, (const char*)sqlite3_column_text(stmt, 7), 31);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return items;
}

Item* db_get_items_by_category(const char *category, int *count) {
    const char *sql = "SELECT id, name, quantity, price, category, low_stock_threshold, created_at, updated_at FROM items WHERE category = ? ORDER BY id";
    sqlite3_stmt *stmt;
    Item *items = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_text(stmt, 1, category, -1, SQLITE_TRANSIENT);
    
    int capacity = 100;
    items = malloc(capacity * sizeof(Item));
    if (!items) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            Item *temp = realloc(items, capacity * sizeof(Item));
            if (!temp) {
                free(items);
                sqlite3_finalize(stmt);
                return NULL;
            }
            items = temp;
        }
        
        Item *item = &items[*count];
        item->id = sqlite3_column_int(stmt, 0);
        strncpy(item->name, (const char*)sqlite3_column_text(stmt, 1), 50);
        item->quantity = sqlite3_column_int(stmt, 2);
        item->price = (float)sqlite3_column_double(stmt, 3);
        strncpy(item->category, (const char*)sqlite3_column_text(stmt, 4), 30);
        item->low_stock_threshold = sqlite3_column_int(stmt, 5);
        strncpy(item->created_at, (const char*)sqlite3_column_text(stmt, 6), 31);
        strncpy(item->updated_at, (const char*)sqlite3_column_text(stmt, 7), 31);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return items;
}

Item* db_get_low_stock_items(int *count) {
    const char *sql = "SELECT id, name, quantity, price, category, low_stock_threshold, created_at, updated_at FROM items WHERE low_stock_threshold > 0 AND quantity <= low_stock_threshold ORDER BY quantity";
    sqlite3_stmt *stmt;
    Item *items = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    int capacity = 100;
    items = malloc(capacity * sizeof(Item));
    if (!items) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            Item *temp = realloc(items, capacity * sizeof(Item));
            if (!temp) {
                free(items);
                sqlite3_finalize(stmt);
                return NULL;
            }
            items = temp;
        }
        
        Item *item = &items[*count];
        item->id = sqlite3_column_int(stmt, 0);
        strncpy(item->name, (const char*)sqlite3_column_text(stmt, 1), 50);
        item->quantity = sqlite3_column_int(stmt, 2);
        item->price = (float)sqlite3_column_double(stmt, 3);
        strncpy(item->category, (const char*)sqlite3_column_text(stmt, 4), 30);
        item->low_stock_threshold = sqlite3_column_int(stmt, 5);
        strncpy(item->created_at, (const char*)sqlite3_column_text(stmt, 6), 31);
        strncpy(item->updated_at, (const char*)sqlite3_column_text(stmt, 7), 31);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return items;
}

bool db_update_item(Item *item) {
    const char *sql = "UPDATE items SET name = ?, quantity = ?, price = ?, category = ?, low_stock_threshold = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, item->name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, item->quantity);
    sqlite3_bind_double(stmt, 3, item->price);
    sqlite3_bind_text(stmt, 4, item->category, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, item->low_stock_threshold);
    sqlite3_bind_int(stmt, 6, item->id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_delete_item(int id) {
    const char *sql = "DELETE FROM items WHERE id = ?";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

int db_add_user(User *user) {
    const char *sql = "INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?)";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->password_hash, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user->role, -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        return -1;
    }
    
    return (int)sqlite3_last_insert_rowid(db);
}

User* db_get_user_by_username(const char *username) {
    const char *sql = "SELECT id, username, password_hash, role, created_at FROM users WHERE username = ?";
    sqlite3_stmt *stmt;
    User *user = NULL;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = malloc(sizeof(User));
        if (user) {
            user->id = sqlite3_column_int(stmt, 0);
            strncpy(user->username, (const char*)sqlite3_column_text(stmt, 1), 50);
            strncpy(user->password_hash, (const char*)sqlite3_column_text(stmt, 2), 127);
            strncpy(user->role, (const char*)sqlite3_column_text(stmt, 3), 15);
            strncpy(user->created_at, (const char*)sqlite3_column_text(stmt, 4), 31);
        }
    }
    
    sqlite3_finalize(stmt);
    return user;
}

User* db_get_user(int id) {
    const char *sql = "SELECT id, username, password_hash, role, created_at FROM users WHERE id = ?";
    sqlite3_stmt *stmt;
    User *user = NULL;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = malloc(sizeof(User));
        if (user) {
            user->id = sqlite3_column_int(stmt, 0);
            strncpy(user->username, (const char*)sqlite3_column_text(stmt, 1), 50);
            strncpy(user->password_hash, (const char*)sqlite3_column_text(stmt, 2), 127);
            strncpy(user->role, (const char*)sqlite3_column_text(stmt, 3), 15);
            strncpy(user->created_at, (const char*)sqlite3_column_text(stmt, 4), 31);
        }
    }
    
    sqlite3_finalize(stmt);
    return user;
}

User* db_get_all_users(int *count) {
    const char *sql = "SELECT id, username, password_hash, role, created_at FROM users ORDER BY id";
    sqlite3_stmt *stmt;
    User *users = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    int capacity = 50;
    users = malloc(capacity * sizeof(User));
    if (!users) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            User *temp = realloc(users, capacity * sizeof(User));
            if (!temp) {
                free(users);
                sqlite3_finalize(stmt);
                return NULL;
            }
            users = temp;
        }
        
        User *user = &users[*count];
        user->id = sqlite3_column_int(stmt, 0);
        strncpy(user->username, (const char*)sqlite3_column_text(stmt, 1), 50);
        strncpy(user->password_hash, (const char*)sqlite3_column_text(stmt, 2), 127);
        strncpy(user->role, (const char*)sqlite3_column_text(stmt, 3), 15);
        strncpy(user->created_at, (const char*)sqlite3_column_text(stmt, 4), 31);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return users;
}

bool db_update_user(User *user) {
    const char *sql = "UPDATE users SET username = ?, password_hash = ?, role = ? WHERE id = ?";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->password_hash, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user->role, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, user->id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_delete_user(int id) {
    const char *sql = "DELETE FROM users WHERE id = ?";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool db_verify_password(const char *username, const char *password) {
    User *user = db_get_user_by_username(username);
    if (!user) {
        return false;
    }
    
    bool valid = (strcmp(user->password_hash, password) == 0);
    free(user);
    return valid;
}

int db_add_category(const char *name) {
    const char *sql = "INSERT OR IGNORE INTO categories (name) VALUES (?)";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        return -1;
    }
    
    return (int)sqlite3_last_insert_rowid(db);
}

Category* db_get_all_categories(int *count) {
    const char *sql = "SELECT id, name FROM categories ORDER BY name";
    sqlite3_stmt *stmt;
    Category *categories = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    int capacity = 50;
    categories = malloc(capacity * sizeof(Category));
    if (!categories) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            Category *temp = realloc(categories, capacity * sizeof(Category));
            if (!temp) {
                free(categories);
                sqlite3_finalize(stmt);
                return NULL;
            }
            categories = temp;
        }
        
        Category *cat = &categories[*count];
        cat->id = sqlite3_column_int(stmt, 0);
        strncpy(cat->name, (const char*)sqlite3_column_text(stmt, 1), 50);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return categories;
}

bool db_delete_category(int id) {
    const char *sql = "DELETE FROM categories WHERE id = ?";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

int db_add_audit_log(int user_id, const char *action, int item_id, const char *details) {
    const char *sql = "INSERT INTO audit_log (user_id, action, item_id, details) VALUES (?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, action, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, item_id);
    sqlite3_bind_text(stmt, 4, details, -1, SQLITE_TRANSIENT);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        return -1;
    }
    
    return (int)sqlite3_last_insert_rowid(db);
}

AuditLog* db_get_audit_logs(int *count) {
    const char *sql = "SELECT a.id, a.user_id, a.action, a.item_id, a.details, a.timestamp, u.username FROM audit_log a LEFT JOIN users u ON a.user_id = u.id ORDER BY a.timestamp DESC LIMIT 500";
    sqlite3_stmt *stmt;
    AuditLog *logs = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    int capacity = 100;
    logs = malloc(capacity * sizeof(AuditLog));
    if (!logs) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            AuditLog *temp = realloc(logs, capacity * sizeof(AuditLog));
            if (!temp) {
                free(logs);
                sqlite3_finalize(stmt);
                return NULL;
            }
            logs = temp;
        }
        
        AuditLog *log = &logs[*count];
        log->id = sqlite3_column_int(stmt, 0);
        log->user_id = sqlite3_column_int(stmt, 1);
        strncpy(log->action, (const char*)sqlite3_column_text(stmt, 2), 31);
        log->item_id = sqlite3_column_int(stmt, 3);
        strncpy(log->details, (const char*)sqlite3_column_text(stmt, 4), 255);
        strncpy(log->timestamp, (const char*)sqlite3_column_text(stmt, 5), 31);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return logs;
}

AuditLog* db_get_audit_logs_by_item(int item_id, int *count) {
    const char *sql = "SELECT a.id, a.user_id, a.action, a.item_id, a.details, a.timestamp, u.username FROM audit_log a LEFT JOIN users u ON a.user_id = u.id WHERE a.item_id = ? ORDER BY a.timestamp DESC";
    sqlite3_stmt *stmt;
    AuditLog *logs = NULL;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, item_id);
    
    int capacity = 50;
    logs = malloc(capacity * sizeof(AuditLog));
    if (!logs) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            AuditLog *temp = realloc(logs, capacity * sizeof(AuditLog));
            if (!temp) {
                free(logs);
                sqlite3_finalize(stmt);
                return NULL;
            }
            logs = temp;
        }
        
        AuditLog *log = &logs[*count];
        log->id = sqlite3_column_int(stmt, 0);
        log->user_id = sqlite3_column_int(stmt, 1);
        strncpy(log->action, (const char*)sqlite3_column_text(stmt, 2), 31);
        log->item_id = sqlite3_column_int(stmt, 3);
        strncpy(log->details, (const char*)sqlite3_column_text(stmt, 4), 255);
        strncpy(log->timestamp, (const char*)sqlite3_column_text(stmt, 5), 31);
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return logs;
}

int db_get_total_items(void) {
    const char *sql = "SELECT COUNT(*) FROM items";
    sqlite3_stmt *stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

float db_get_total_value(void) {
    const char *sql = "SELECT COALESCE(SUM(quantity * price), 0) FROM items";
    sqlite3_stmt *stmt;
    float value = 0.0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0.0;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        value = (float)sqlite3_column_double(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return value;
}

int db_get_low_stock_count(void) {
    const char *sql = "SELECT COUNT(*) FROM items WHERE low_stock_threshold > 0 AND quantity <= low_stock_threshold";
    sqlite3_stmt *stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

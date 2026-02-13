#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stdbool.h>

typedef struct {
    int id;
    char name[51];
    int quantity;
    float price;
    char category[31];
    int low_stock_threshold;
    char created_at[32];
    char updated_at[32];
} Item;

typedef struct {
    int id;
    char username[51];
    char password_hash[128];
    char role[16];
    char created_at[32];
} User;

typedef struct {
    int id;
    int user_id;
    char action[32];
    int item_id;
    char details[256];
    char timestamp[32];
} AuditLog;

typedef struct {
    int id;
    char name[51];
} Category;

bool db_init(const char *db_path);
void db_close();

bool db_create_tables(void);
bool db_migrate(void);

int db_add_item(Item *item);
Item* db_get_item(int id);
Item* db_get_all_items(int *count);
Item* db_search_items(const char *query, int *count);
Item* db_get_items_by_category(const char *category, int *count);
Item* db_get_low_stock_items(int *count);
bool db_update_item(Item *item);
bool db_delete_item(int id);

int db_add_user(User *user);
User* db_get_user_by_username(const char *username);
User* db_get_user(int id);
User* db_get_all_users(int *count);
bool db_update_user(User *user);
bool db_delete_user(int id);
bool db_verify_password(const char *username, const char *password);

int db_add_category(const char *name);
Category* db_get_all_categories(int *count);
bool db_delete_category(int id);

int db_add_audit_log(int user_id, const char *action, int item_id, const char *details);
AuditLog* db_get_audit_logs(int *count);
AuditLog* db_get_audit_logs_by_item(int item_id, int *count);

int db_get_total_items(void);
float db_get_total_value(void);
int db_get_low_stock_count(void);

char* db_get_db_path(void);
void db_set_db_path(const char *path);

#endif

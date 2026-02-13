#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>

typedef struct {
    int id;
    char username[51];
    char role[16];
} Session;

bool auth_init(void);
bool auth_login(const char *username, const char *password);
void auth_logout(void);
Session* auth_get_current_user(void);
bool auth_has_permission(const char *required_role);
bool auth_create_user(const char *username, const char *password, const char *role);
bool auth_delete_user(int user_id);
bool auth_list_users(void);
bool auth_change_password(int user_id, const char *new_password);

#endif

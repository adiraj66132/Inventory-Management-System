#include "../include/auth.h"
#include "../include/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Session *current_session = NULL;

bool auth_init(void) {
    int count = 0;
    User *users = db_get_all_users(&count);
    
    if (count == 0) {
        User admin;
        strncpy(admin.username, "admin", sizeof(admin.username) - 1);
        strncpy(admin.password_hash, "admin123", sizeof(admin.password_hash) - 1);
        strncpy(admin.role, "admin", sizeof(admin.role) - 1);
        
        if (users) free(users);
        
        int result = db_add_user(&admin);
        if (result > 0) {
            printf("Default admin user created (admin/admin123)\n");
            return true;
        }
        return false;
    }
    
    if (users) free(users);
    return true;
}

bool auth_login(const char *username, const char *password) {
    User *user = db_get_user_by_username(username);
    
    if (!user) {
        return false;
    }
    
    bool valid = (strcmp(user->password_hash, password) == 0);
    
    if (valid) {
        if (current_session) {
            free(current_session);
        }
        
        current_session = malloc(sizeof(Session));
        if (current_session) {
            current_session->id = user->id;
            strncpy(current_session->username, user->username, sizeof(current_session->username) - 1);
            strncpy(current_session->role, user->role, sizeof(current_session->role) - 1);
        }
    }
    
    free(user);
    return valid;
}

void auth_logout(void) {
    if (current_session) {
        free(current_session);
        current_session = NULL;
    }
}

Session* auth_get_current_user(void) {
    return current_session;
}

bool auth_has_permission(const char *required_role) {
    if (!current_session) {
        return false;
    }
    
    if (strcmp(current_session->role, "admin") == 0) {
        return true;
    }
    
    if (strcmp(required_role, "admin") == 0) {
        return false;
    }
    
    if (strcmp(current_session->role, "manager") == 0) {
        if (strcmp(required_role, "manager") == 0 || strcmp(required_role, "viewer") == 0) {
            return true;
        }
        return false;
    }
    
    if (strcmp(required_role, "viewer") == 0) {
        return true;
    }
    
    return false;
}

bool auth_create_user(const char *username, const char *password, const char *role) {
    if (!auth_has_permission("admin")) {
        return false;
    }
    
    User user;
    strncpy(user.username, username, sizeof(user.username) - 1);
    strncpy(user.password_hash, password, sizeof(user.password_hash) - 1);
    strncpy(user.role, role, sizeof(user.role) - 1);
    
    int result = db_add_user(&user);
    return result > 0;
}

bool auth_delete_user(int user_id) {
    if (!auth_has_permission("admin")) {
        return false;
    }
    
    if (current_session && current_session->id == user_id) {
        return false;
    }
    
    return db_delete_user(user_id);
}

bool auth_list_users(void) {
    if (!auth_has_permission("admin")) {
        return false;
    }
    
    int count = 0;
    User *users = db_get_all_users(&count);
    
    if (!users || count == 0) {
        if (users) free(users);
        return false;
    }
    
    printf("\n=== User List ===\n");
    printf("%-5s %-20s %-10s %-20s\n", "ID", "Username", "Role", "Created");
    printf("------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-10s %-20s\n", 
               users[i].id, 
               users[i].username, 
               users[i].role,
               users[i].created_at);
    }
    
    free(users);
    return true;
}

bool auth_change_password(int user_id, const char *new_password) {
    if (!auth_has_permission("admin") && 
        (!current_session || current_session->id != user_id)) {
        return false;
    }
    
    User *user = db_get_user(user_id);
    if (!user) {
        return false;
    }
    
    strncpy(user->password_hash, new_password, sizeof(user->password_hash) - 1);
    
    bool result = db_update_user(user);
    free(user);
    return result;
}

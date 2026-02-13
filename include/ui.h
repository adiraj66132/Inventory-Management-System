#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <stdbool.h>

void ui_init(void);
void ui_cleanup(void);
void ui_login_screen(void);
void ui_main_menu(void);
void ui_add_item_screen(void);
void ui_view_items_screen(void);
void ui_update_item_screen(void);
void ui_delete_item_screen(void);
void ui_search_screen(void);
void ui_category_screen(void);
void ui_export_screen(void);
void ui_import_screen(void);
void ui_statistics_screen(void);
void ui_audit_log_screen(void);
void ui_user_management_screen(void);

#endif

#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *page_menu;
    lv_obj_t *page_weather;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *menu_time;
    lv_obj_t *menu_date;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *cpu_temp;
    lv_obj_t *cpu_using;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *gpu_temp;
    lv_obj_t *gpu_using;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *ram_used_bar;
    lv_obj_t *ram_used_text;
    lv_obj_t *obj15;
    lv_obj_t *ram_free_bar;
    lv_obj_t *ram_free_text;
    lv_obj_t *obj16;
    lv_obj_t *cpu_power_bar;
    lv_obj_t *cpu_power_text;
    lv_obj_t *obj17;
    lv_obj_t *gpu_power_bar;
    lv_obj_t *gpu_power_text;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *weather_time;
    lv_obj_t *weather_date;
    lv_obj_t *obj21;
    lv_obj_t *temp_weather;
    lv_obj_t *wind_weather;
    lv_obj_t *hum_weather;
    lv_obj_t *press_weather;
    lv_obj_t *main_weather;
    lv_obj_t *city_weather;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_PAGE_MENU = 2,
    SCREEN_ID_PAGE_WEATHER = 3,
};

void create_screen_main();
void tick_screen_main();

void create_screen_page_menu();
void tick_screen_page_menu();

void create_screen_page_weather();
void tick_screen_page_weather();

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/
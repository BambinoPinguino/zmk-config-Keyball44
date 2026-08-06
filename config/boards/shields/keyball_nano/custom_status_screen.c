/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * Custom status screen for the Keyball44 right half. Hosts two full-screen
 * views, toggled by the `bongo_screen` behavior:
 *   - `&bongo_screen 1`: full-screen Bongo Cat animation + WPM counter
 *   - `&bongo_screen 0`: stock ZMK status widgets (battery, output, layer, WPM)
 */

#include "bongo_cat.h"
#include "bongo_screen.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/display/widgets/battery_status.h>
#include <zmk/display/widgets/layer_status.h>
#include <zmk/display/widgets/output_status.h>
#include <zmk/display/widgets/wpm_status.h>

static struct zmk_widget_bongo_cat bongo_cat_widget;

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
static struct zmk_widget_battery_status battery_status_widget;
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
static struct zmk_widget_output_status output_status_widget;
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
static struct zmk_widget_layer_status layer_status_widget;
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
static struct zmk_widget_wpm_status wpm_status_widget;
#endif

static lv_obj_t *bongo_container;
static lv_obj_t *stock_container;

static void set_screen_mode(struct bongo_screen_mode mode) {
    if (mode.which == 1) {
        lv_obj_clear_flag(bongo_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(stock_container, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(bongo_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(stock_container, LV_OBJ_FLAG_HIDDEN);
    }
}

static struct bongo_screen_mode bongo_screen_mode_get_state(const zmk_event_t *eh) {
    if (eh == NULL) {
        return (struct bongo_screen_mode){.which = 1};
    }

    struct bongo_screen_mode *ev = as_bongo_screen_mode(eh);
    return ev ? *ev : (struct bongo_screen_mode){.which = 1};
};

ZMK_DISPLAY_WIDGET_LISTENER(bongo_screen_mode_listener, struct bongo_screen_mode, set_screen_mode,
                            bongo_screen_mode_get_state)

ZMK_SUBSCRIPTION(bongo_screen_mode_listener, bongo_screen_mode);

static lv_obj_t *make_container(lv_obj_t *screen) {
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, 128, 32);
    lv_obj_set_style_bg_color(container, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);
    return container;
}

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen;

    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);

    bongo_container = make_container(screen);
    zmk_widget_bongo_cat_init(&bongo_cat_widget, bongo_container);
    lv_obj_align(zmk_widget_bongo_cat_obj(&bongo_cat_widget), LV_ALIGN_CENTER, 0, 0);

    stock_container = make_container(screen);
#if IS_ENABLED(CONFIG_ZMK_WIDGET_BATTERY_STATUS)
    zmk_widget_battery_status_init(&battery_status_widget, stock_container);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_status_widget), LV_ALIGN_TOP_RIGHT, 0, 0);
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_OUTPUT_STATUS)
    zmk_widget_output_status_init(&output_status_widget, stock_container);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget), LV_ALIGN_TOP_LEFT, 0, 0);
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_LAYER_STATUS)
    zmk_widget_layer_status_init(&layer_status_widget, stock_container);
    lv_obj_set_style_text_font(zmk_widget_layer_status_obj(&layer_status_widget),
                               lv_theme_get_font_small(screen), LV_PART_MAIN);
    lv_obj_align(zmk_widget_layer_status_obj(&layer_status_widget), LV_ALIGN_BOTTOM_LEFT, 0, 0);
#endif
#if IS_ENABLED(CONFIG_ZMK_WIDGET_WPM_STATUS)
    zmk_widget_wpm_status_init(&wpm_status_widget, stock_container);
    lv_obj_align(zmk_widget_wpm_status_obj(&wpm_status_widget), LV_ALIGN_BOTTOM_RIGHT, 0, 0);
#endif

    bongo_screen_mode_listener_init();

    return screen;
}

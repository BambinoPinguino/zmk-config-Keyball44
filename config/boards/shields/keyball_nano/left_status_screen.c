/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * Status screen for the left (peripheral) half. Matches the stock ZMK
 * peripheral page (white background / black text): a battery label plus the
 * left-right connection indicator. The battery label collapses to the compact
 * icon by default (bongo mode) and shows an icon plus percentage in status
 * mode; the connection indicator is shown in both variants.
 */

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>

#include "battery_detail.h"
#include "battery_label.h"

#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
#include <zmk/display/widgets/peripheral_status.h>
#endif

static struct battery_label_widget battery_label_widget;

#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
static struct zmk_widget_peripheral_status peripheral_status_widget;
#endif

static void battery_detail_update_cb(struct battery_detail state) {
    battery_label_widget_set_detail(&battery_label_widget, state.detail != 0);
}

static struct battery_detail battery_detail_get_state(const zmk_event_t *eh) {
    struct battery_detail *ev = as_battery_detail(eh);
    return ev ? *ev : (struct battery_detail){.detail = 0};
}

ZMK_DISPLAY_WIDGET_LISTENER(battery_detail_listener, struct battery_detail,
                            battery_detail_update_cb, battery_detail_get_state)

ZMK_SUBSCRIPTION(battery_detail_listener, battery_detail);

lv_obj_t *zmk_display_status_screen() {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);

    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, 128, 32);
    lv_obj_set_style_bg_color(container, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);

    battery_label_widget_init(&battery_label_widget, container);
    lv_obj_set_style_text_color(battery_label_widget_obj(&battery_label_widget),
                                lv_color_black(), LV_PART_MAIN);
    lv_obj_align(battery_label_widget_obj(&battery_label_widget), LV_ALIGN_TOP_RIGHT, 0, 0);

#if IS_ENABLED(CONFIG_ZMK_WIDGET_PERIPHERAL_STATUS)
    zmk_widget_peripheral_status_init(&peripheral_status_widget, container);
    lv_obj_set_style_text_color(zmk_widget_peripheral_status_obj(&peripheral_status_widget),
                                lv_color_black(), LV_PART_MAIN);
    lv_obj_align(zmk_widget_peripheral_status_obj(&peripheral_status_widget), LV_ALIGN_TOP_LEFT, 0,
                 0);
#endif

    battery_detail_listener_init();

    return screen;
}
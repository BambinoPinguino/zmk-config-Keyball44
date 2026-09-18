/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * Generic battery label widget. Renders the local battery as either a bare
 * icon (LV_SYMBOL_BATTERY_*) or an icon plus percentage. USB power adds a
 * charge glyph prefix. `battery_label_widget_set_detail(false)` switches to
 * the compact icon-only view.
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/usb.h>
#include <zmk/events/usb_conn_state_changed.h>

#include "battery_label.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_label_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif
};

static const char *battery_symbol(uint8_t level) {
    if (level > 95) {
        return LV_SYMBOL_BATTERY_FULL;
    } else if (level > 65) {
        return LV_SYMBOL_BATTERY_3;
    } else if (level > 35) {
        return LV_SYMBOL_BATTERY_2;
    } else if (level > 5) {
        return LV_SYMBOL_BATTERY_1;
    } else {
        return LV_SYMBOL_BATTERY_EMPTY;
    }
}

static void set_battery_text(lv_obj_t *label, struct battery_label_state state, bool detail) {
    char text[16] = {};

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    if (state.usb_present) {
        strcat(text, LV_SYMBOL_CHARGE " ");
    }
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

    strcat(text, battery_symbol(state.level));

    if (detail) {
        char perc[5] = {};
        snprintf(perc, sizeof(perc), "%3u%%", state.level);
        strcat(text, " ");
        strcat(text, perc);
    }

    lv_label_set_text(label, text);
}

static void battery_label_update_cb(struct battery_label_state state) {
    struct battery_label_widget *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_battery_text(widget->obj, state, widget->detail);
    }
}

static struct battery_label_state battery_label_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_label_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(battery_label, struct battery_label_state, battery_label_update_cb,
                            battery_label_get_state)

ZMK_SUBSCRIPTION(battery_label, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(battery_label, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

int battery_label_widget_init(struct battery_label_widget *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    widget->detail = true;

    sys_slist_append(&widgets, &widget->node);

    battery_label_init();
    return 0;
}

lv_obj_t *battery_label_widget_obj(struct battery_label_widget *widget) {
    return widget->obj;
}

void battery_label_widget_set_detail(struct battery_label_widget *widget, bool detail) {
    widget->detail = detail;
    set_battery_text(widget->obj, battery_label_get_state(NULL), detail);
}
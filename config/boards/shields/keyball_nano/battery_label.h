/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/sys/slist.h>

struct battery_label_widget {
    sys_snode_t node;
    lv_obj_t *obj;
    bool detail;
};

int battery_label_widget_init(struct battery_label_widget *widget, lv_obj_t *parent);

lv_obj_t *battery_label_widget_obj(struct battery_label_widget *widget);

void battery_label_widget_set_detail(struct battery_label_widget *widget, bool detail);
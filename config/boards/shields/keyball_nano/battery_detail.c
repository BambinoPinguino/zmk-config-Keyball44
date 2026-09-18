/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * `&battery_detail 0` switches the battery label to the compact icon-only
 * view, `&battery_detail 1` switches it to an icon plus percentage. The
 * behavior only raises an event; the screen switch happens in the local
 * status screen. Compiled on both halves so the keymap resolves here, and so
 * the central half can mirror screen changes to the peripheral.
 */

#define DT_DRV_COMPAT zmk_behavior_battery_detail

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <drivers/behavior.h>
#include <zmk/event_manager.h>

#include "battery_detail.h"

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

ZMK_EVENT_IMPL(battery_detail);

static int on_battery_detail_binding_pressed(struct zmk_behavior_binding *binding,
                                             struct zmk_behavior_binding_event event) {
    return raise_battery_detail((struct battery_detail){.detail = (uint8_t)binding->param1});
}

static int on_battery_detail_binding_released(struct zmk_behavior_binding *binding,
                                              struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api battery_detail_driver_api = {
    .binding_pressed = on_battery_detail_binding_pressed,
    .binding_released = on_battery_detail_binding_released,
};

static int battery_detail_init(const struct device *dev) { return 0; }

BEHAVIOR_DT_INST_DEFINE(0, battery_detail_init, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &battery_detail_driver_api);

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
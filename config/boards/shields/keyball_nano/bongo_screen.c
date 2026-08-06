/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * `&bongo_screen 0` switches the display to the stock status screen,
 * `&bongo_screen 1` switches it back to the Bongo Cat animation. The behavior
 * only raises an event; the actual screen switch happens in the central half's
 * custom status screen. Compiled on both halves so the keymap resolves here.
 */

#define DT_DRV_COMPAT zmk_behavior_bongo_screen

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <drivers/behavior.h>
#include <zmk/event_manager.h>

#include "bongo_screen.h"

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

ZMK_EVENT_IMPL(bongo_screen_mode);

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {
        .display_name = "Screen",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE,
        .range = {.min = 0, .max = 1},
    },
};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static int on_bongo_screen_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    return raise_bongo_screen_mode((struct bongo_screen_mode){.which = binding->param1});
}

static int on_bongo_screen_binding_released(struct zmk_behavior_binding *binding,
                                            struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api bongo_screen_driver_api = {
    .binding_pressed = on_bongo_screen_binding_pressed,
    .binding_released = on_bongo_screen_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

static int bongo_screen_init(const struct device *dev) { return 0; }

BEHAVIOR_DT_INST_DEFINE(0, bongo_screen_init, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &bongo_screen_driver_api);

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

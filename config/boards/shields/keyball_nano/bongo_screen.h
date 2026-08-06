/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * Event + behavior for switching the display between the Bongo Cat animation
 * and the stock status screen. Param 0 = stock, 1 = Bongo Cat.
 */

#pragma once

#include <zmk/event_manager.h>

struct bongo_screen_mode {
    uint8_t which;
};

ZMK_EVENT_DECLARE(bongo_screen_mode);

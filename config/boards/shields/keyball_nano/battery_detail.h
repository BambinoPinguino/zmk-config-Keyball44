/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * Event for switching the battery label between its detailed (icon +
 * percentage) and compact (icon only) views. Param 0 = compact, 1 = detail.
 */

#pragma once

#include <zmk/event_manager.h>

struct battery_detail {
    uint8_t detail;
};

ZMK_EVENT_DECLARE(battery_detail);
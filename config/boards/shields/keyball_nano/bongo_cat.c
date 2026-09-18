/*
 * Copyright (c) 2026 The Bongocat ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * WPM-responsive Bongo Cat animation for a 128x32 OLED, ported from the QMK
 * doio/kb16 Bongo Cat animation. Frame art lives in bongo_cat_images.c.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

#include "bongo_cat.h"

#define SRC(array) (const void **)array, sizeof(array) / sizeof(lv_img_dsc_t *)

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

LV_IMG_DECLARE(bongo_idle1);
LV_IMG_DECLARE(bongo_idle2);
LV_IMG_DECLARE(bongo_idle3);
LV_IMG_DECLARE(bongo_idle4);
LV_IMG_DECLARE(bongo_idle5);
LV_IMG_DECLARE(bongo_prep);
LV_IMG_DECLARE(bongo_tap1);
LV_IMG_DECLARE(bongo_tap2);

/* Idle: 5 frames at 300ms each (QMK IDLE_FRAME_DURATION). */
#define ANIMATION_SPEED_IDLE 1500
const lv_img_dsc_t *idle_imgs[] = {
    &bongo_idle1,
    &bongo_idle2,
    &bongo_idle3,
    &bongo_idle4,
    &bongo_idle5,
};

/* Wind-up: static prep frame shown between idle and typing (QMK prep band). */
#define ANIMATION_SPEED_PREP 1000
const lv_img_dsc_t *prep_imgs[] = {
    &bongo_prep,
};

/* Typing: prep + alternating paws. Per-frame time follows the QMK curve
 * max(100, 450 - 2.5 * wpm) at 250 / 125 / 100 ms. */
#define ANIMATION_SPEED_SLOW 1000
const lv_img_dsc_t *slow_imgs[] = {
    &bongo_prep,
    &bongo_tap1,
    &bongo_prep,
    &bongo_tap2,
};

#define ANIMATION_SPEED_MID 500
const lv_img_dsc_t *mid_imgs[] = {
    &bongo_prep,
    &bongo_tap1,
    &bongo_prep,
    &bongo_tap2,
};

#define ANIMATION_SPEED_FAST 400
const lv_img_dsc_t *fast_imgs[] = {
    &bongo_tap1,
    &bongo_tap2,
    &bongo_tap1,
    &bongo_tap2,
};

struct bongo_cat_wpm_status_state {
    uint8_t wpm;
};

enum anim_state {
    anim_state_none,
    anim_state_idle,
    anim_state_prep,
    anim_state_slow,
    anim_state_mid,
    anim_state_fast
} current_anim_state;

static void set_animation(lv_obj_t *animing, struct bongo_cat_wpm_status_state state) {
    if (state.wpm < 10) {
        if (current_anim_state != anim_state_idle) {
            lv_animimg_set_src(animing, SRC(idle_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_IDLE);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_idle;
        }
    } else if (state.wpm < 20) {
        if (current_anim_state != anim_state_prep) {
            lv_animimg_set_src(animing, SRC(prep_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_PREP);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_prep;
        }
    } else if (state.wpm < 60) {
        if (current_anim_state != anim_state_slow) {
            lv_animimg_set_src(animing, SRC(slow_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_SLOW);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_slow;
        }
    } else if (state.wpm < 100) {
        if (current_anim_state != anim_state_mid) {
            lv_animimg_set_src(animing, SRC(mid_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_MID);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_mid;
        }
    } else {
        if (current_anim_state != anim_state_fast) {
            lv_animimg_set_src(animing, SRC(fast_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_FAST);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_fast;
        }
    }
}

#define WPM_WINDOW_SECONDS 15
#define WPM_BUCKETS WPM_WINDOW_SECONDS
#define CHARS_PER_WORD 5.0

/* Sliding 15 s WPM window, refreshed once per second. One bucket holds the
 * number of key releases folded in each elapsed second; buckets rotate over
 * WPM_BUCKETS slots, so summing the array yields a rolling 15 s count. */
static uint16_t wpm_release_buckets[WPM_BUCKETS];
static uint64_t wpm_folded_seconds;
static uint16_t wpm_current_releases;
static lv_timer_t *wpm_refresh_timer;

static void wpm_fold_seconds(uint64_t up_to) {
    if (wpm_folded_seconds == 0) {
        wpm_folded_seconds = up_to;
        return;
    }

    while (wpm_folded_seconds < up_to) {
        wpm_release_buckets[wpm_folded_seconds % WPM_BUCKETS] = wpm_current_releases;
        wpm_current_releases = 0;
        wpm_folded_seconds++;
    }
}

static uint32_t wpm_window_sum(void) {
    uint32_t sum = 0;
    for (size_t i = 0; i < WPM_BUCKETS; i++) {
        sum += wpm_release_buckets[i];
    }
    return sum;
}

static void bongo_cat_wpm_update(uint8_t wpm) {
    struct bongo_cat_wpm_status_state state = {.wpm = wpm};
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        set_animation(widget->obj, state);
        lv_label_set_text_fmt(widget->wpm_label, "WPM:%03u", wpm);
    }
}

static void bongo_cat_wpm_refresh(lv_timer_t *timer) {
    (void)timer;
    wpm_fold_seconds(k_uptime_get() / 1000);
    uint32_t wpm = (uint32_t)((wpm_window_sum() / CHARS_PER_WORD) / (WPM_WINDOW_SECONDS / 60.0));
    if (wpm > 255) {
        wpm = 255;
    }
    bongo_cat_wpm_update((uint8_t)wpm);
}

static int bongo_cat_wpm_keycode_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev && !ev->state) {
        wpm_current_releases++;
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bongo_cat_wpm, bongo_cat_wpm_keycode_listener);
ZMK_SUBSCRIPTION(bongo_cat_wpm, zmk_keycode_state_changed);

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    widget->obj = lv_animimg_create(parent);
    lv_obj_center(widget->obj);

    widget->wpm_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->wpm_label, &lv_font_unscii_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->wpm_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(widget->wpm_label, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_label_set_text(widget->wpm_label, "WPM:000");

    sys_slist_append(&widgets, &widget->node);

    if (wpm_refresh_timer == NULL) {
        wpm_refresh_timer = lv_timer_create(bongo_cat_wpm_refresh, 1000, NULL);
        bongo_cat_wpm_refresh(wpm_refresh_timer);
    }

    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) {
    return widget->obj;
}

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

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>

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

struct bongo_cat_wpm_status_state bongo_cat_wpm_status_get_state(const zmk_event_t *eh) {
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    return (struct bongo_cat_wpm_status_state){.wpm = ev->state};
};

void bongo_cat_wpm_status_update_cb(struct bongo_cat_wpm_status_state state) {
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_animation(widget->obj, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_bongo_cat, struct bongo_cat_wpm_status_state,
                            bongo_cat_wpm_status_update_cb, bongo_cat_wpm_status_get_state)

ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    widget->obj = lv_animimg_create(parent);
    lv_obj_center(widget->obj);

    sys_slist_append(&widgets, &widget->node);

    widget_bongo_cat_init();

    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) {
    return widget->obj;
}

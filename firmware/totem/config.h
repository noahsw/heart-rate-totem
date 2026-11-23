#pragma once

#include <stdint.h>

// ============================================================================
// CONFIG: your strap, the board's I2C bus, brightness levels and HR zones
// ============================================================================
//
// The totem's own settings (panels, shows, phone remote, beat drop) are in
// totem_config.h. This file holds the ones that describe you and the board.

// --- 1. Your strap's MAC address ---
// Defined in secrets.h (gitignored, never committed) so it doesn't end up in
// a public repo. Copy secrets.h.example to secrets.h in this folder and fill
// it in; find_whoop_mac/ prints the MAC of every heart rate strap nearby.
#include "secrets.h"

// --- 2. The board's I2C bus ---
// The thumb wheel is wired to the board's PCA9557 I/O expander, which sits on
// this bus with the RTC and the audio chips (ES7210 mics, ES8311 speaker).
// From the board maker's wiki: https://seengreat.com/wiki/214/rgb-matrix-hub75-s3
#define I2C_SDA 1
#define I2C_SCL 2

// --- 3. Brightness levels, set with the thumb wheel ---
// Roll the wheel (top edge, K1/K2/K3) for brighter/dimmer, or press it in to
// step through and wrap; the choice is saved across reboots. See brightness.h.
// The totem scales these against LED_MAX_MA (totem_config.h), which is what
// actually keeps the panels inside the battery's budget.
//
// Levels 1 and 2 are for dimming down (neighbours, battery), so they sit well
// below the usual level 3: about a fifth and two fifths of it.
const uint8_t BRIGHTNESS_LEVELS[] = {18, 36, 90, 140, 200};
// Index into BRIGHTNESS_LEVELS used until the wheel saves a different one.
#define BRIGHTNESS_DEFAULT_LEVEL 2
// Which PCA9557 pin each wheel motion is. The wiki gives K1 = IO1,
// K2 = IO3, K3 = IO2, but not which K is up, down or press. The serial log
// names each key as it's used: if rolling up dims the panels, swap UP and
// DOWN; if the push doesn't cycle, move it to the right K.
#define BRIGHT_KEY_UP_BIT    1   // K1
#define BRIGHT_KEY_PRESS_BIT 3   // K2
#define BRIGHT_KEY_DOWN_BIT  2   // K3

// --- 4. HR zone thresholds (BPM) ---
// Set these for YOUR heart, not a generic training chart. The defaults are for
// someone whose resting HR is 55-60 and who peaks around 110 dancing hard; a
// stock chart would put 110 in zone 1 and the bar would sit dead-left all night.
//
// REST_BPM is the bar's zero point, not just zone 0's label: the bar should be
// empty when you're doing nothing, not 80% full because it starts at 0 bpm.
//
//   Z0 60-99 white | Z1 100-109 yellow | Z2 110-119 orange | Z3 120+ red
//
// Everything below 100 is one plain white zone. The colours only start once
// you're actually moving, in 10 bpm steps. Starting white at 60 makes it exactly
// 40 bpm, so on each face's 4-px slice of the bar each pixel is 10 bpm.
//
// The host tests (make -C tests) check that any set of thresholds you pick
// still makes a sane bar: run them after a retune.
#define REST_BPM  60
#define ZONE1_BPM 100
#define ZONE2_BPM 110
#define ZONE3_BPM 120
// The top of the red band, used only to compute progress ACROSS that last
// segment of the bar (there's no zone above it to spill into).
#define ZONE_MAX_BPM 140

// With no strap connected the heart keeps beating at this rate, with no
// number beside it. A dead-looking totem reads as broken; a heart with no
// number is obvious enough as "searching".
#define IDLE_HEART_BPM 60

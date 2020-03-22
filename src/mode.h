#pragma once

/*
 * There are different modes to be operated. Each mode has a different
 * multiplier for the delay before and after operating the servo. This may
 * result in a 1:1 ventilation and a 1:2 ventilation.
 */

/**
 * Simple struct type to store those two multipliers.
 */
struct mode_t {
  double mult_inhale;
  double mult_exhale;
};

/* The _two_ preconfigured ventilation profiles: 1:1 and 1:2 */
const int mode_sizeof = 2;
const mode_t modes[mode_sizeof] = {
  mode_t{0.5, 0.5},
  mode_t{0.33, 0.66},
};

/* Selection of the current mode from the modes above. */
volatile int mode = 0;

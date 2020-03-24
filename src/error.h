#pragma once

/* Error types to distinguish between different errors */
enum error_t { err_none, err_servo_count, err_light_barrier };
extern volatile error_t error;

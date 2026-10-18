#ifndef VITAL_SIGNS_FILTER_H
#define VITAL_SIGNS_FILTER_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* Timestamped samples only. The reference-display consumer accepts positive rates.
 * A full window is required after startup, absence, invalid data or a gap.
 * Capacity exhaustion restarts acquisition instead of using a shorter window. */
#define VITAL_SIGNS_WINDOW_MS 3000U
#define VITAL_SIGNS_WINDOW_CAPACITY 64U

typedef struct
{
    float value;
    uint32_t at;
} vital_signs_sample_t;

typedef struct
{
    vital_signs_sample_t samples[VITAL_SIGNS_WINDOW_CAPACITY];
    uint32_t started_at;
    uint16_t head;
    uint16_t count;
    bool collecting;
} vital_signs_window_t;

static inline void vital_signs_window_reset(vital_signs_window_t *window)
{
    window->head = 0U;
    window->count = 0U;
    window->collecting = false;
}

static inline void vital_signs_window_expire(vital_signs_window_t *window, uint32_t now)
{
    while(window->count != 0U &&
          (uint32_t)(now - window->samples[window->head].at) >= VITAL_SIGNS_WINDOW_MS)
    {
        window->head = (window->head + 1U) % VITAL_SIGNS_WINDOW_CAPACITY;
        --window->count;
    }
    if(window->count == 0U)
    {
        vital_signs_window_reset(window);
    }
}

static inline void vital_signs_window_push(vital_signs_window_t *window,
                                          uint32_t now, float value)
{
    vital_signs_window_expire(window, now);
    if(!isfinite(value) || value < 0.0f)
    {
        vital_signs_window_reset(window);
        return;
    }
    if(window->count == VITAL_SIGNS_WINDOW_CAPACITY)
    {
        vital_signs_window_reset(window);
    }
    if(!window->collecting)
    {
        window->started_at = now;
        window->collecting = true;
    }
    uint16_t tail = (window->head + window->count) % VITAL_SIGNS_WINDOW_CAPACITY;
    window->samples[tail].value = value;
    window->samples[tail].at = now;
    ++window->count;
}

static inline bool vital_signs_window_mean(vital_signs_window_t *window,
                                          uint32_t now, float *mean)
{
    vital_signs_window_expire(window, now);
    if(!window->collecting || window->count < 2U ||
       (uint32_t)(now - window->started_at) < VITAL_SIGNS_WINDOW_MS)
    {
        return false;
    }
    /* Accumulate in double so finite float samples cannot overflow the sum. */
    double sum = 0.0;
    for(uint16_t i = 0U; i < window->count; ++i)
    {
        sum += window->samples[(window->head + i) % VITAL_SIGNS_WINDOW_CAPACITY].value;
    }
    *mean = (float)(sum / window->count);
    return true;
}

#endif

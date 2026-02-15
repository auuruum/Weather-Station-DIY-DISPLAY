#pragma once

#include <SettingsGyver.h>
#include "../config/constants.h"

// Get color based on temperature for UI display
static sets::Colors getColorByTemp(float t) {
    // Comfortable zone
    if (t >= COMFORT_MIN && t <= COMFORT_MAX)
        return sets::Colors::Green;

    if (t < COMFORT_MIN) {
        if (t < COMFORT_MIN - 15) return sets::Colors::Aqua;        // very cold
        if (t < COMFORT_MIN - 10)  return sets::Colors::Blue;       // cool
        return sets::Colors::Blue;                                  // slightly cold
    }

    if (t > COMFORT_MAX) {
        if (t > COMFORT_MAX + 10) return sets::Colors::Red;         // very hot
        if (t > COMFORT_MAX + 5)  return sets::Colors::Orange;      // warm-hot
        return sets::Colors::Yellow;                                // slightly warm
    }

    return sets::Colors::White;
}

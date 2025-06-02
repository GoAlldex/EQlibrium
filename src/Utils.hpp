#pragma once
#include <type_traits>

// This file contains utility classes and functions for the AvSynth audio plugin.

/// @brief A simple linear ramp generator for smooth transitions between two values without any branching.
template <typename T>
    requires std::is_arithmetic_v<T> || requires(T a, T b, float f) {
        { a + b };
        { a - b };
        { a *f };
        { a / f };
    }
class LinearRamp {
  public:
    void reset(T start, T end, int steps) {
        current = start;
        increment = (steps > 0) ? (end - start) / static_cast<std::common_type_t<T, float>>(steps) : T{};
        remainingSteps = steps;
    }

    T getNext() {
        T value = current;

        // Active flag: 0 or 1
        int active = (remainingSteps > 0);
        auto fActive = static_cast<typename std::common_type<T, float>::type>(active);

        // Only accumulate if active
        current += increment * fActive;
        remainingSteps -= active;

        return value;
    }

  private:
    T current{};
    T increment{};
    int remainingSteps = 0;
};

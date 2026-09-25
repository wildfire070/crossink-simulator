#pragma once

#include <cstdint>
#include <random>

// Host stand-in for the ESP32 hardware RNG.
inline uint32_t esp_random() {
  static std::mt19937 rng{std::random_device{}()};
  return rng();
}

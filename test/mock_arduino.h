#pragma once
// Minimal Arduino stubs for PlatformIO native unit tests.
// Provides just enough to compile pure-logic functions on the host PC.

#include <stdint.h>
#include <algorithm>
#include <cmath>

using std::max;
using std::min;

// Arduino integer helpers
#define constrain(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))
#define map(val, il, ih, ol, oh) ((long)(val - il) * (oh - ol) / (ih - il) + ol)

// Dummy millis / delay
inline uint32_t millis() { return 0; }
inline void delay(uint32_t) {}
inline void delayMicroseconds(uint32_t) {}

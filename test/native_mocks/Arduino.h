#pragma once
// Minimal Arduino.h stand-in so real firmware .cpp files (e.g.
// feeding_scheduler.cpp) can be compiled and linked directly into a native
// unit test, instead of hand-copying their logic.
//
// Only the pieces actually used by the modules under test are stubbed here —
// extend as needed when a new real .cpp is pulled into native tests.

#include "../mock_arduino.h"
#include <cstdarg>
#include <cstdio>

struct SerialClass {
  void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
  void println(const char* msg) { puts(msg); }
};

inline SerialClass Serial;

inline void ets_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

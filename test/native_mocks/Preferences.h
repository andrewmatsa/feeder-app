#pragma once
// Minimal Preferences.h stand-in for native unit tests. FeedingScheduler only
// stores/loads via these calls when begin()/loadFromPreferences()/
// saveToPreferences() are invoked — tests that never call those methods don't
// need working persistence, just matching signatures so the real .cpp compiles.

#include <cstddef>
#include <cstdint>

class Preferences {
public:
  int getInt(const char*, int32_t defaultValue = 0) { return defaultValue; }
  size_t putInt(const char*, int32_t) { return 0; }
  int64_t getLong64(const char*, int64_t defaultValue = 0) { return defaultValue; }
  size_t putLong64(const char*, int64_t) { return 0; }
  bool isKey(const char*) { return false; }
};

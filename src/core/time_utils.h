#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>

inline unsigned long getTimestamp() {
  time_t now = time(nullptr);
  if (now > 1000000000) {
    return (unsigned long)now * 1000;  // Convert to milliseconds
  }
  return millis();  // Fallback to uptime if NTP not synced
}

#endif

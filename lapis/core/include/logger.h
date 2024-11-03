#pragma once

#define LOG_INFO(...) printf(__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) \
  fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
  fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_DEBUG_POINT() \
  fprintf(stderr, "[%s:%d] {%s}\n", __FILE__, __LINE__, __func__)

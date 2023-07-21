#ifndef DBG_H
#define DBG_H

void dbg_print(const char *file, const char *func, int line, const char *fmt,
               ...);

#define dbg_info(fmt, ...) \
  dbg_print(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#endif
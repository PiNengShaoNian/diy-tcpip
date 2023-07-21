#include "dbg.h"

#include <stdarg.h>

#include "sys_plat.h"

void dbg_print(const char *file, const char *func, int line, const char *fmt,
               ...) {
  const char *end = file + plat_strlen(file);
  while (end >= file) {
    if (*end == '\\' || *end == '/') {
      break;
    }

    end--;
  }

  end++;

  plat_printf("(%s-%s-%d): ", end, func, line);

  va_list args;
  char str_buf[128];

  va_start(args, fmt);

  plat_vsprintf(str_buf, fmt, args);
  plat_printf("%s\n", str_buf);

  va_end(args);
}

#include "dbg.h"

#include <stdarg.h>

#include "sys_plat.h"

void dbg_print(int m_level, int s_level, const char *file, const char *func,
               int line, const char *fmt, ...) {
  static const char *title[] = {
      [DBG_LEVEL_NONE] = "none",
      [DBG_LEVEL_ERROR] = DBG_STYLE_ERROR "error",
      [DBG_LEVEL_WARNING] = DBG_STYLE_WARNING "warning",
      [DBG_LEVEL_INFO] = "info",
  };

  if (m_level >= s_level) {
    const char *end = file + plat_strlen(file);
    while (end >= file) {
      if (*end == '\\' || *end == '/') {
        break;
      }

      end--;
    }

    end++;

    plat_printf(title[s_level]);
    plat_printf("(%s-%s-%d): ", end, func, line);

    va_list args;
    char str_buf[128];

    va_start(args, fmt);

    plat_vsprintf(str_buf, fmt, args);
    plat_printf("%s\n" DBG_STYLE_RESET, str_buf);

    va_end(args);
  }
}

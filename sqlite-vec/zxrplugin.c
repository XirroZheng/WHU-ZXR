#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

#include <string.h>
#include <stdio.h>

// 我们的扩展函数实现
static void helloFunc(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
  if (argc < 1)
  {
    sqlite3_result_text(ctx, "Hello, world!", -1, SQLITE_TRANSIENT);
    return;
  }

  const char *name = (const char *)sqlite3_value_text(argv[0]);
  char buf[100];
  snprintf(buf, sizeof(buf), "Hello, %s!", name);
  sqlite3_result_text(ctx, buf, -1, SQLITE_TRANSIENT);
}

// 扩展初始化函数（SQLite通过这个函数识别插件）
int sqlite3_zxrplugin_init(
    sqlite3 *db,
    char **pzErrMsg,
    const sqlite3_api_routines *pApi)
{
  SQLITE_EXTENSION_INIT2(pApi);
  // 注册 SQL 函数：hello(name)
  sqlite3_create_function(db, "hello", 1, SQLITE_UTF8, 0, helloFunc, 0, 0);
  return SQLITE_OK;
}

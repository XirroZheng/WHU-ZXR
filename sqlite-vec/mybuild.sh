#!/bin/bash

SRC="sqlite-vec.c"
OUT="vec0.so"
SQLITE_INC="/home/xirro/sqlite-vec/vendor"


if [ ! -f "$SRC" ]; then
  echo "❌ 源文件 $SRC 不存在，请确认路径。"
  exit 1
fi
gcc -g -fPIC -shared sqlite-vec.c -o vec0.so -I/home/xirro/sqlite-vec/vendor
if [ $? -eq 0 ]; then
  echo "✅ 构建完成：$OUT"
else
  echo "❌ 构建失败"
  exit 1
fi

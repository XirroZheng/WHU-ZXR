CXX = g++
CC = gcc

SRC_DIR = .
SQLITE_DIR = ../build

# 编译选项
CXXFLAGS = -std=c++17 -Wall -g -I$(SQLITE_DIR) -DSQLITE_ENABLE_LOAD_EXTENSION

# 👉 关键：不再用系统 sqlite
LDFLAGS = -ldl

SRCS = main.cpp main2.cpp
TARGETS = $(SRCS:.cpp=)

# 关闭内建规则
.SUFFIXES:

all: libsqlite3.a $(TARGETS)

# =====================
# 链接（🔥核心：导出符号）
# =====================
$(TARGETS): %: %.o libsqlite3.a
	$(CXX) $< libsqlite3.a $(LDFLAGS) -Wl,--export-dynamic -o $@

# =====================
# 编译 cpp
# =====================
%.o: %.cpp $(SQLITE_DIR)/sqlite3.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =====================
# 编译 sqlite3（你的版本）
# =====================
sqlite3.o: $(SQLITE_DIR)/sqlite3.c $(SQLITE_DIR)/sqlite3.h
	$(CC) -c $(SQLITE_DIR)/sqlite3.c -o sqlite3.o -DSQLITE_ENABLE_LOAD_EXTENSION

libsqlite3.a: sqlite3.o
	ar rcs libsqlite3.a sqlite3.o

# =====================
# 清理
# =====================
clean:
	rm -f *.o *.a $(TARGETS)
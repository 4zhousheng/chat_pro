# # =============================================================================
# # Top-level Makefile for My Chat Project
# # =============================================================================

# # --- 1. 定义核心变量 ---

# # 编译器
# CC := gcc
# CXX := g++ # 如果有C++代码

# # 编译和链接标志
# # CFLAGS: 用于C文件编译
# # CXXFLAGS: 用于C++文件编译
# # LDFLAGS: 用于链接
# # -g: 添加调试信息
# # -Wall: 开启所有警告
# # -I./include: **核心！** 告诉编译器去 ./include 目录下查找头文件
# CFLAGS := -g -Wall -I./include
# LDFLAGS := 

# # 目录定义
# SRC_DIR := ./src
# BUILD_DIR := ./build
# BIN_DIR := ./bin

# # --- 2. 定义源文件和目标 ---

# # 服务器源文件 (使用 wildcard 函数自动查找所有 .c 文件)
# # SERVER_SRC_DIRS := $(shell find $(SRC_DIR)/server -type d)
# # SERVER_SOURCES := $(foreach dir,$(SERVER_SRC_DIRS),$(wildcard $(dir)/*.c))
# SERVER_SOURCES := $(shell find $(SRC_DIR)/server -name '*.c')

# # 客户端源文件
# # CLIENT_SRC_DIRS := $(shell find $(SRC_DIR)/client -type d)
# # CLIENT_SOURCES := $(foreach dir,$(CLIENT_SRC_DIRS),$(wildcard $(dir)/*.c))
# CLIENT_SOURCES := $(shell find $(SRC_DIR)/client -name '*.c')

# # 目标文件 (.o 文件)
# # patsubst 函数将 ./src/server/core/log.c 这样的路径替换为 ./build/server/core/log.o
# SERVER_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SERVER_SOURCES))
# CLIENT_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CLIENT_SOURCES))

# # 最终可执行文件名
# SERVER_TARGET := $(BIN_DIR)/chat_server
# CLIENT_TARGET := $(BIN_DIR)/chat_client

# # --- 3. 定义链接外部库 ---
# # 在这里添加你需要链接的库，例如 -lmysqlclient, -lhiredis, -lpthread
# # **这是你需要根据实际情况修改的地方**
# LIBS := -lpthread -lmysqlclient -lhiredis

# # --- 4. 编写核心规则 ---

# # 伪目标 (Phony Targets)，告诉make这些不是真正的文件名
# .PHONY: all server client clean

# # "all" 规则: 默认目标，当只输入 `make` 时执行
# all: $(SERVER_TARGET) $(CLIENT_TARGET)

# # "server" 规则: 单独编译服务器
# server: $(SERVER_TARGET)

# # "client" rule: 单独编译客户端
# client: $(CLIENT_TARGET)

# # 链接服务器可执行文件的规则
# # 依赖于所有服务器的 .o 文件
# $(SERVER_TARGET): $(SERVER_OBJECTS)
# 	@mkdir -p $(@D) # @D 代表目标的目录部分，即 ./bin/
# 	@echo "Linking server executable..."
# 	$(CC) $(SERVER_OBJECTS) -o $@ $(LDFLAGS) $(LIBS) # $@ 代表目标本身
# 	@echo "Server built successfully: $@"

# # 链接客户端可执行文件的规则
# $(CLIENT_TARGET): $(CLIENT_OBJECTS)
# 	@mkdir -p $(@D)
# 	@echo "Linking client executable..."
# 	$(CC) $(CLIENT_OBJECTS) -o $@ $(LDFLAGS)
# 	@echo "Client built successfully: $@"

# # --- 5. 编译 .c 到 .o 的通用规则 ---
# # 这是一个模式规则，适用于所有 .o 文件
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
# 	@mkdir -p $(@D)
# 	@echo "Compiling $< ..." # $< 代表第一个依赖项，即 .c 文件
# 	$(CC) $(CFLAGS) -c $< -o $@

# # --- 6. 清理规则 ---
# clean:
# 	@echo "Cleaning up build artifacts..."
# 	@rm -rf $(BUILD_DIR) $(BIN_DIR)
# 	@echo "Clean complete."
# =============================================================================
# Top-level Makefile for My Chat Project
# =============================================================================

# --- 1. 定义核心变量 ---

# 编译器
CC := gcc
CXX := g++ # 如果有C++代码

# 编译和链接标志
# CFLAGS: 用于C文件编译
# CXXFLAGS: 用于C++文件编译
# LDFLAGS: 用于链接
# -g: 添加调试信息
# -Wall: 开启所有警告
# -I./include: **核心！** 告诉编译器去 ./include 目录下查找头文件
CFLAGS := -g -Wall -I./include
LDFLAGS := 

# 目录定义
SRC_DIR := ./src
BUILD_DIR := ./build
BIN_DIR := ./bin

# --- 2. 定义源文件和目标 ---

# 【新增】共享模块源文件 (例如, logic, model 等)
# 我们将 logic 目录视为共享模块
COMMON_SOURCES := $(shell find $(SRC_DIR)/logic -name '*.c')

# 服务器源文件 (使用 wildcard 函数自动查找所有 .c 文件)
SERVER_SOURCES := $(shell find $(SRC_DIR)/server -name '*.c')

# 客户端源文件
CLIENT_SOURCES := $(shell find $(SRC_DIR)/client -name '*.c')

# 目标文件 (.o 文件)
# patsubst 函数将 ./src/server/core/log.c 这样的路径替换为 ./build/server/core/log.o

# 【新增】共享模块的目标文件
COMMON_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(COMMON_SOURCES))

SERVER_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SERVER_SOURCES))
CLIENT_OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CLIENT_SOURCES))

# 最终可执行文件名
SERVER_TARGET := $(BIN_DIR)/chat_server
CLIENT_TARGET := $(BIN_DIR)/chat_client

# --- 3. 定义链接外部库 ---
# 在这里添加你需要链接的库，例如 -lmysqlclient, -lhiredis, -lpthread
# **这是你需要根据实际情况修改的地方**
LIBS := -lpthread -lmysqlclient -lhiredis

# --- 4. 编写核心规则 ---

# 伪目标 (Phony Targets)，告诉make这些不是真正的文件名
.PHONY: all server client clean

# "all" 规则: 默认目标，当只输入 `make` 时执行
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# "server" 规则: 单独编译服务器
server: $(SERVER_TARGET)

# "client" rule: 单独编译客户端
client: $(CLIENT_TARGET)

# 链接服务器可执行文件的规则
# 【修改】依赖项中加入共享的 .o 文件
$(SERVER_TARGET): $(SERVER_OBJECTS) $(COMMON_OBJECTS)
	@mkdir -p $(@D) # @D 代表目标的目录部分，即 ./bin/
	@echo "Linking server executable..."
	# 【修改】链接命令中加入共享的 .o 文件
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS) # $^ 代表所有依赖项
	@echo "Server built successfully: $@"

# 链接客户端可执行文件的规则
# 【修改】依赖项中加入共享的 .o 文件
$(CLIENT_TARGET): $(CLIENT_OBJECTS) $(COMMON_OBJECTS)
	@mkdir -p $(@D)
	@echo "Linking client executable..."
	# 【修改】链接命令中加入共享的 .o 文件
	# 【修改】客户端通常也需要线程库，所以把 $(LIBS) 也加上 (如果需要)
	# 如果客户端不需要数据库等库，可以单独定义 CLIENT_LIBS
	$(CC) $^ -o $@ $(LDFLAGS) -lpthread
	@echo "Client built successfully: $@"

# --- 5. 编译 .c 到 .o 的通用规则 ---
# 这是一个模式规则，适用于所有 .o 文件
# 这个规则是通用的，无需修改，它会自动处理 common, server, client 目录下的文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@echo "Compiling $< ..." # $< 代表第一个依赖项，即 .c 文件
	$(CC) $(CFLAGS) -c $< -o $@

# --- 6. 清理规则 ---
clean:
	@echo "Cleaning up build artifacts..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete."
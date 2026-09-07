# ============================================================
# VNET CLIENT - Makefile v4.0 (Fast Parallel Build)
# VEKTRAOS v9.5 CYBERWARFARE ENGINE
# ============================================================

# ---- COLORS ----
RED    := \033[0;31m
GREEN  := \033[0;32m
YELLOW := \033[0;33m
BLUE   := \033[0;34m
MAGENTA:= \033[0;35m
CYAN   := \033[0;36m
WHITE  := \033[0;37m
BOLD   := \033[1m
RESET  := \033[0m

# ---- PROJECT DIRS ----
PROJECT_DIR := $(CURDIR)
SRC_DIR     := src
BUILD_DIR   := build_client
VENDOR_DIR  := vendor

# ---- PATHS ----
RAYLIB_INCLUDE := $(VENDOR_DIR)/raylib/include
RAYLIB_LIB     := $(VENDOR_DIR)/raylib/lib

# ---- COMPILER ----
CXX      := g++
CXXFLAGS := -std=c++17 -O3 -g -Wall -Wextra
CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES -D_WIN32_WINNT=0x0600

# ---- INCLUDES ----
INCLUDES := -I$(PROJECT_DIR) \
            -I$(PROJECT_DIR)/$(SRC_DIR)/client \
            -I$(PROJECT_DIR)/$(SRC_DIR)/client/desktop \
            -I$(PROJECT_DIR)/$(SRC_DIR)/client/desktop/apps \
            -I$(PROJECT_DIR)/$(SRC_DIR)/client/desktop/apps/vdec \
            -I$(PROJECT_DIR)/$(SRC_DIR)/client/desktop/settings \
            -I$(PROJECT_DIR)/$(SRC_DIR)/client/connection \
            -I$(PROJECT_DIR)/$(SRC_DIR)/client/raid \
            -I$(PROJECT_DIR)/$(SRC_DIR)/shared \
            -I$(PROJECT_DIR)/$(SRC_DIR)/lib \
            -I$(RAYLIB_INCLUDE)

# ---- LIBS ----
LIBS := -L$(RAYLIB_LIB) -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -lwinpthread -lws2_32 -lm

# ---- TARGET ----
TARGET := vnet_client.exe

# ============================================================
# HARDCODED SOURCE FILES (from your project structure)
# ============================================================
# This is faster than find/dir scanning and works on Windows.
# Update this list when you add new files.

# ---- CLIENT SOURCES ----
CLIENT_SRCS := \
    src/client/main.cpp \
    src/client/game.cpp \
    src/client/player.cpp \
    src/client/render.cpp \
    src/client/audio.cpp \
    src/client/vnet_client.cpp \
    src/client/connection/login_screen.cpp \
    src/client/desktop/desktop.cpp \
    src/client/desktop/desktop_icons.cpp \
    src/client/desktop/wallpaper.cpp \
    src/client/desktop/apps/browser.cpp \
    src/client/desktop/apps/feed.cpp \
    src/client/desktop/apps/hellroom.cpp \
    src/client/desktop/apps/intruder_detector.cpp \
    src/client/desktop/apps/profile.cpp \
    src/client/desktop/apps/settings.cpp \
    src/client/desktop/apps/terminal.cpp \
    src/client/desktop/apps/vdec/vdec.cpp \
    src/client/desktop/apps/vdec/decrypt.cpp \
    src/client/desktop/apps/vdec/encrypt.cpp \
    src/client/desktop/apps/vdec/hash.cpp \
    src/client/desktop/apps/vdec/keyring.cpp \
    src/client/desktop/apps/vdec/minigame.cpp \
    src/client/desktop/settings/audio.cpp \
    src/client/desktop/settings/display.cpp \
    src/client/desktop/settings/security.cpp \
    src/client/desktop/settings/theme.cpp \
    src/client/desktop/widgets/music_player.cpp \
    src/client/raid/raid_core.cpp \
    src/client/raid/raid_sequence.cpp \
    src/client/raid/raid_draw.cpp \
    src/client/raid/raid_commands.cpp \
    src/client/raid/raid_ui.cpp \
    src/lib/vnet_lib.cpp \
    src/shared/utils.cpp \
    src/shared/vnet.cpp \
    src/shared/vex_parser.cpp \
    src/shared/vnet_sites.cpp

# ---- OBJECT FILES ----
OBJ_FILES := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(CLIENT_SRCS))

# ============================================================
# TARGETS
# ============================================================

.PHONY: all clean run help

.DEFAULT_GOAL := all

# ---- HEADER ----
define header
	@printf "\n$(MAGENTA)╔═══════════════════════════════════════════════════════╗$(RESET)\n"
	@printf "$(MAGENTA)║ $1$(RESET)\n"
	@printf "$(MAGENTA)╚═══════════════════════════════════════════════════════╝$(RESET)\n\n"
endef

# ---- ALL ----
all: header_check $(BUILD_DIR) $(TARGET) post_build summary

# ---- HEADER CHECK ----
header_check:
	@$(call header, "VNET CLIENT - MAKE BUILD v4.0 (PARALLEL)")
	@printf "$(CYAN)ℹ️  Project directory: $(PROJECT_DIR)$(RESET)\n"
	@printf "$(CYAN)ℹ️  Source files: $(words $(CLIENT_SRCS))$(RESET)\n"
	@printf "$(CYAN)ℹ️  Using: make -j$(shell nproc 2>/dev/null || echo 4)$(RESET)\n"

# ---- BUILD DIR ----
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

# ---- COMPILE RULE ----
$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@printf "$(CYAN)  Compiling $(notdir $<)...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@ 2>&1 || (printf "\r$(RED)  ❌ $(notdir $<) - FAILED$(RESET)\n" && exit 1)
	@printf "\r$(GREEN)  ✅ $(notdir $<)$(RESET)\n"

# ---- LINK ----
$(TARGET): $(OBJ_FILES)
	@printf "\n$(MAGENTA)════════════════════════════════════════════════════════════════$(RESET)\n"
	@printf "$(MAGENTA)  🔗 LINKING $(words $(OBJ_FILES)) OBJECT FILES$(RESET)\n"
	@printf "$(MAGENTA)════════════════════════════════════════════════════════════════$(RESET)\n\n"
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJ_FILES) $(LIBS)
	@if [ $$? -eq 0 ]; then \
		printf "$(GREEN)✅ Linking successful!$(RESET)\n"; \
	else \
		printf "$(RED)❌ Linking failed!$(RESET)\n"; \
		exit 1; \
	fi

# ---- POST BUILD ----
post_build:
	@printf "\n$(MAGENTA)════════════════════════════════════════════════════════════════$(RESET)\n"
	@printf "$(MAGENTA)  📦 POST-BUILD$(RESET)\n"
	@printf "$(MAGENTA)════════════════════════════════════════════════════════════════$(RESET)\n"
	@if exist "$(RAYLIB_LIB)\raylib.dll" ( \
		copy "$(RAYLIB_LIB)\raylib.dll" . > nul && \
		printf "$(GREEN)✅ Copied raylib.dll$(RESET)\n" \
	)

# ---- SUMMARY ----
summary:
	@printf "\n$(MAGENTA)════════════════════════════════════════════════════════════════$(RESET)\n"
	@printf "$(MAGENTA)  📊 BUILD SUMMARY$(RESET)\n"
	@printf "$(MAGENTA)════════════════════════════════════════════════════════════════$(RESET)\n"
	@if exist "$(TARGET)" ( \
		for %%I in ("$(TARGET)") do set "SIZE=%%~zI" & \
		set /a "SIZE_KB=!SIZE!/1024" & \
		printf "$(GREEN)✅ Client executable: $(TARGET)$(RESET)\n" & \
		printf "$(CYAN)📦 Output size: !SIZE_KB! KB$(RESET)\n" & \
		printf "$(CYAN)📁 Source files compiled: $(words $(CLIENT_SRCS))$(RESET)\n" & \
		printf "$(GREEN)🎉 Client build completed successfully!$(RESET)\n" \
	) else ( \
		printf "$(RED)❌ Build failed - executable not found!$(RESET)\n" & \
		exit 1 \
	)

# ---- CLEAN ----
clean:
	@printf "$(YELLOW)🧹 Cleaning build directory...$(RESET)\n"
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@if exist "$(TARGET)" del /q "$(TARGET)"
	@printf "$(GREEN)✅ Clean complete$(RESET)\n"

# ---- RUN ----
run: $(TARGET)
	@printf "\n$(CYAN)🚀 Launching VNET Client...$(RESET)\n\n"
	@$(TARGET)

# ---- HELP ----
help:
	@printf "$(BOLD)$(CYAN)VNET Client Makefile Commands:$(RESET)\n\n"
	@printf "  $(GREEN)make$(RESET)         - Build the client (default)\n"
	@printf "  $(GREEN)make clean$(RESET)   - Remove build artifacts\n"
	@printf "  $(GREEN)make run$(RESET)     - Build and run the client\n"
	@printf "  $(GREEN)make help$(RESET)    - Show this help\n\n"
	@printf "$(CYAN)Fast parallel build:$(RESET) make -j$(shell nproc 2>/dev/null || echo 4)\n"
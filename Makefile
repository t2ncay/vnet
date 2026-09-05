# ============================================================
# VNET 3D - Makefile (MSYS/GNU - MinGW)
# ============================================================

CXX = g++
CC = gcc
RM = rm -f
MKDIR = mkdir -p

TARGET = vnet_demo.exe
BUILD_DIR = build

# Paths
RAYLIB_INCLUDE = -I./vendor/raylib/include
RAYLIB_LIB_PATH = -L./vendor/raylib/lib

# Compiler flags (same as your Vyne Makefile)
CXXFLAGS = -std=c++17 -O2 -g
CXXFLAGS += -I. $(RAYLIB_INCLUDE)
CXXFLAGS += -D_WIN32 -DWIN32_LEAN_AND_MEAN -DNOGDI -DNOUSER
CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES
CXXFLAGS += -MMD -MP

# Debug flags
DEBUG_CXXFLAGS = -std=c++17 -O0 -g
DEBUG_CXXFLAGS += -I. $(RAYLIB_INCLUDE)
DEBUG_CXXFLAGS += -D_WIN32 -DWIN32_LEAN_AND_MEAN -DNOGDI -DNOUSER
DEBUG_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES
DEBUG_CXXFLAGS += -MMD -MP

# Linker flags (same as your Vyne Makefile)
LDFLAGS = -mconsole $(RAYLIB_LIB_PATH)
LDFLAGS += -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -lwinpthread -lws2_32

# Source files
SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

# ============================================================
# Build Rules
# ============================================================

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "🔗 Linking $@..."
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "✅ Build complete! Run ./$(TARGET)"

$(BUILD_DIR)/%.o: src/%.cpp
	@$(MKDIR) $(BUILD_DIR)
	@echo "📦 Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ============================================================
# Debug Build
# ============================================================

debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: clean $(TARGET)

# ============================================================
# Run
# ============================================================

run: $(TARGET)
	@echo "🚀 Running $(TARGET)..."
	./$(TARGET)

# ============================================================
# Clean
# ============================================================

clean:
	@echo "🧹 Cleaning up..."
	$(RM) -rf $(BUILD_DIR) $(TARGET)
	@echo "✅ Clean complete"

# ============================================================
# Help
# ============================================================

help:
	@echo "========================================="
	@echo "  VNET 3D - Makefile Commands"
	@echo "========================================="
	@echo "  make         - Build the project"
	@echo "  make run     - Build and run"
	@echo "  make debug   - Build with debug symbols"
	@echo "  make clean   - Remove build files"
	@echo "  make help    - Show this help"
	@echo "========================================="

-include $(DEPS)

.PHONY: all run debug clean help
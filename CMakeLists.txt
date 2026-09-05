cmake_minimum_required(VERSION 3.10)
project(VNET3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# COMPILER SETTINGS (Match your Vyne Makefile)
# ============================================================

# Use GCC/MinGW (not Visual Studio)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# ============================================================
# PATHS (Match your Vyne Makefile)
# ============================================================

set(RAYLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/vendor/raylib/include)
set(RAYLIB_LIB_DIR ${CMAKE_SOURCE_DIR}/vendor/raylib/lib)

# OpenSSL (if needed)
set(OPENSSL_INC C:/msys64/ucrt64/include)
set(OPENSSL_LIB C:/msys64/ucrt64/lib)

# ============================================================
# COMPILE FLAGS (Match your Vyne Makefile)
# ============================================================

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SOURCE_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${RAYLIB_INCLUDE_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${OPENSSL_INC}")

# Windows defines (from your Makefile)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32 -DWIN32_LEAN_AND_MEAN -DNOGDI -DNOUSER")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCPPHTTPLIB_OPENSSL_SUPPORT")

# Optimization (O3 from your Makefile)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

# ============================================================
# LINK FLAGS (Match your Vyne Makefile)
# ============================================================

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${RAYLIB_LIB_DIR}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${OPENSSL_LIB}")

# Libraries (exactly like your Makefile)
set(LIBS
    raylib
    opengl32
    gdi32
    winmm
    shell32
    winpthread
    ws2_32
    ssl
    crypto
    crypt32
    bcrypt
)

# ============================================================
# SOURCE FILES
# ============================================================

file(GLOB SOURCES "src/*.cpp")

# ============================================================
# CREATE EXECUTABLE
# ============================================================

add_executable(vnet_demo ${SOURCES})

# Include directories
target_include_directories(vnet_demo PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${RAYLIB_INCLUDE_DIR}
    ${OPENSSL_INC}
)

# Link libraries
target_link_libraries(vnet_demo PRIVATE ${LIBS})

# ============================================================
# COPY ASSETS (Post-build)
# ============================================================

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET vnet_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        COMMENT "Copying assets..."
    )
endif()

# Copy raylib.dll if it exists
if(EXISTS "${RAYLIB_LIB_DIR}/raylib.dll")
    add_custom_command(TARGET vnet_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${RAYLIB_LIB_DIR}/raylib.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Copying raylib.dll..."
    )
endif()

# ============================================================
# PRINT CONFIGURATION
# ============================================================

message(STATUS "========================================")
message(STATUS "VNET 3D Configuration Complete!")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
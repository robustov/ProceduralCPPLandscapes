cmake_minimum_required(VERSION 3.16)
project(mountains LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Produce compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Collect sources (non-recursive) ---
# Prefer explicit, simple globs so we don't accidentally pick up files from the build dir.
file(GLOB SRC_FILES
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/*.cpp"   # optional top-level sources (e.g. main.cpp)
)

if(NOT SRC_FILES)
  message(FATAL_ERROR "No source .cpp files found in src/ or project root.")
endif()

# Print discovered sources (handy for debugging)
message(STATUS "Source files: ${SRC_FILES}")

add_executable(mountains ${SRC_FILES})

# Make headers available (include/)
target_include_directories(mountains PRIVATE "${CMAKE_SOURCE_DIR}/include")

# --- SDL2 find: prefer CMake config, fall back to pkg-config ---
find_package(SDL2 CONFIG QUIET)

if(SDL2_FOUND)
  message(STATUS "Found SDL2 via CMake config")
  target_link_libraries(mountains PRIVATE SDL2::SDL2)
else()
  message(STATUS "SDL2 CMake config not found; trying pkg-config")
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(SDL2_PKG QUIET sdl2)
    if(SDL2_PKG_FOUND)
      message(STATUS "Found SDL2 via pkg-config")
      target_include_directories(mountains PRIVATE ${SDL2_PKG_INCLUDE_DIRS})
      target_link_libraries(mountains PRIVATE ${SDL2_PKG_LIBRARIES})
      if(SDL2_PKG_CFLAGS_OTHER)
        target_compile_options(mountains PRIVATE ${SDL2_PKG_CFLAGS_OTHER})
      endif()
    else()
      message(WARNING "pkg-config could not find sdl2. If your project uses SDL2, install libsdl2-dev.")
    endif()
  else()
    message(WARNING "pkg-config not available. If your project uses SDL2, install libsdl2-dev or provide SDL2 manually.")
  endif()
endif()

# Enable useful warnings
if (MSVC)
  target_compile_options(mountains PRIVATE /W4)
else()
  target_compile_options(mountains PRIVATE -Wall -Wextra -Wpedantic)
endif()


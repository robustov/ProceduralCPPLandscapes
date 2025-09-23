cmake_minimum_required(VERSION 3.16)
project(mountains VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(SDL2 CONFIG QUIET)

if(SDL2_FOUND)
  message(STATUS "Found SDL2 via CMake config")
  set(SDL2_TARGET SDL2::SDL2)
else()
  message(STATUS "SDL2 CMake config not found; trying pkg-config")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2_PKG REQUIRED sdl2)

  if(NOT SDL2_PKG_FOUND)
    message(FATAL_ERROR "SDL2 not found. Install SDL2 dev package (e.g. libsdl2-dev).")
  endif()

  add_library(SDL2::SDL2 INTERFACE IMPORTED)
  set_target_properties(SDL2::SDL2 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_PKG_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${SDL2_PKG_LIBRARIES}"
  )
  if(SDL2_PKG_CFLAGS)
    target_compile_options(SDL2::SDL2 INTERFACE ${SDL2_PKG_CFLAGS})
  endif()
  set(SDL2_TARGET SDL2::SDL2)
endif()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

if(NOT SRC_FILES)
  if(EXISTS "${CMAKE_SOURCE_DIR}/generator.cpp")
    set(SRC_FILES "${CMAKE_SOURCE_DIR}/generator.cpp")
  else()
    message(FATAL_ERROR "No source files found in src/ and no generator.cpp at project root.")
  endif()
endif()

add_executable(mountains ${SRC_FILES})

target_link_libraries(mountains PRIVATE ${SDL2_TARGET})

if (MSVC)
  target_compile_options(mountains PRIVATE /W4 /permissive-)
else()
  target_compile_options(mountains PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS mountains DESTINATION bin)



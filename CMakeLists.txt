cmake_minimum_required(VERSION 3.16)
project(mountains LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


file(GLOB SRC_FILES CONFIGURE_DEPENDS
"${CMAKE_SOURCE_DIR}/src/*.cpp"
)
add_executable(mountains ${SRC_FILES})


target_include_directories(mountains PRIVATE "${CMAKE_SOURCE_DIR}/include")


find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_PKG REQUIRED sdl2)
if(SDL2_PKG_FOUND)
target_include_directories(mountains PRIVATE ${SDL2_PKG_INCLUDE_DIRS})
target_link_libraries(mountains PRIVATE ${SDL2_PKG_LIBRARIES})
endif()


if (MSVC)
target_compile_options(mountains PRIVATE /W4)
else()
target_compile_options(mountains PRIVATE -Wall -Wextra -Wpedantic)
endif()

cmake_minimum_required(VERSION 3.10.0)
project(Render VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_PREFIX_PATH "F:/code_field/cpp/vcpkg/installed/x64-windows")

find_package(PNG REQUIRED)
find_package(raylib REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SourceFiles
    src/main.cpp
    src/gui.cpp
)

add_executable(${PROJECT_NAME} ${SourceFiles})


target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG raylib)

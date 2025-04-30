cmake_minimum_required(VERSION 3.14)
project(timer_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add FTXUI as a subdirectory
include(FetchContent)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v2.0.0
)
FetchContent_MakeAvailable(ftxui)

# Add executable
add_executable(timer_app src/main.cpp)

# Link FTXUI
target_link_libraries(timer_app PRIVATE ftxui::screen ftxui::dom ftxui::component) 
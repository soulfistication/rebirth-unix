cmake_minimum_required(VERSION 3.16)
project(rebirth_unix VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE AND EXISTS "/opt/homebrew")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
endif()

add_executable(rebirth
    src/main.cpp
    src/engine.cpp
    src/tb303.cpp
    src/drums.cpp
    src/fx.cpp
    src/sequencer.cpp
    src/ui.cpp
)

target_include_directories(rebirth PRIVATE src)

find_package(SDL2 QUIET)
if(TARGET SDL2::SDL2)
    target_link_libraries(rebirth PRIVATE SDL2::SDL2)
elseif(SDL2_FOUND)
    target_include_directories(rebirth PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(rebirth PRIVATE ${SDL2_LIBRARIES})
else()
    find_program(SDL2_CONFIG sdl2-config REQUIRED)
    execute_process(COMMAND ${SDL2_CONFIG} --cflags
        OUTPUT_VARIABLE SDL2_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${SDL2_CONFIG} --libs
        OUTPUT_VARIABLE SDL2_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
    separate_arguments(SDL2_CFLAGS_LIST UNIX_COMMAND "${SDL2_CFLAGS}")
    separate_arguments(SDL2_LIBS_LIST UNIX_COMMAND "${SDL2_LIBS}")
    target_compile_options(rebirth PRIVATE ${SDL2_CFLAGS_LIST})
    target_link_libraries(rebirth PRIVATE ${SDL2_LIBS_LIST})
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(rebirth PRIVATE pthread m dl)
endif()

target_compile_options(rebirth PRIVATE -Wall -Wextra -O2)

install(TARGETS rebirth RUNTIME DESTINATION bin)

cmake_minimum_required(VERSION 3.10)
project(f22_game)

set(CMAKE_C_STANDARD 11)

add_executable(f22_game
    src/main.c
    src/f22.c
    src/game_state.c
    src/renderer.c
    src/wave.c
    src/asteroid.c
)

# Emscripten specific settings
if(EMSCRIPTEN)
    # Enable SDL2
    set(USE_SDL2 "-sUSE_SDL=2")

    # Set output type and flags
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    # Combine all necessary emscripten flags
    set(EMSCRIPTEN_FLAGS "${USE_SDL2} -sWASM=1 -sALLOW_MEMORY_GROWTH=1 --shell-file ${CMAKE_SOURCE_DIR}/template.html")

    # Apply to both compiler and linker
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_FLAGS}")

    # Add SDL2 include path explicitly
    include_directories(${EMSCRIPTEN_ROOT_PATH}/cache/sysroot/include)
endif()

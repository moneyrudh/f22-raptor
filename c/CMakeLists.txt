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

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    
    set(COMPILE_FLAGS 
        "-sUSE_SDL=2 \
         -sWASM=1 \
         -sALLOW_MEMORY_GROWTH=1 \
         -sPRINTF_LONG_DOUBLE=1 \
         -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'] \
         -sEXPORTED_FUNCTIONS=['_main','_malloc','_free'] \
         -sASYNCIFY \
         --shell-file ${CMAKE_SOURCE_DIR}/template.html"
    )
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMPILE_FLAGS}")
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_ttf::SDL2_ttf)
endif()
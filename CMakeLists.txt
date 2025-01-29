cmake_minimum_required(VERSION 3.10)
project(f22_raptor)

set(CMAKE_C_STANDARD 11)

add_executable(f22_raptor
    src/main.c
    src/f22.c
    src/game_state.c
    src/renderer.c
    src/wave.c
    src/asteroid.c
    src/missile.c
    src/explosion.c
    src/smoke.c
    src/sound.c
    src/security.c
)

if(EMSCRIPTEN)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/public)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ENVIRONMENT='SCORE_SECRET_KEY'")

    set(COMPILE_FLAGS 
        "-sUSE_SDL=2 \
         -sUSE_SDL_MIXER=2 \
         -sWASM=1 \
         -sALLOW_MEMORY_GROWTH=1 \
         -sPRINTF_LONG_DOUBLE=1 \
         -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap'] \
         -sEXPORTED_FUNCTIONS=['_main','_malloc','_free'] \
         -sASYNCIFY \
         -sSTACK_SIZE=131072 \
         -sSDL2_MIXER_FORMATS=['mp3'] \
         --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets \
         --shell-file ${CMAKE_SOURCE_DIR}/template.html"
    )
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMPILE_FLAGS}")
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        SDL2::SDL2 
        SDL2::SDL2main 
        SDL2_ttf::SDL2_ttf
        SDL2_mixer::SDL2_mixer
    )
endif()
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
    # Handle secret key as a compile definition
    if(DEFINED SCORE_SECRET_KEY)
        message(STATUS "Using provided SCORE_SECRET_KEY")
        target_compile_definitions(f22_raptor PRIVATE 
            SCORE_SECRET_KEY="${SCORE_SECRET_KEY}")
    else()
        message(WARNING "SCORE_SECRET_KEY not provided")
    endif()
    
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
         --shell-file ${CMAKE_SOURCE_DIR}/template.html"
    )
 
    # Set preload and compression as separate options
    set(PRELOAD_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets --use-preload-plugins")
    set(COMPRESSION_FLAGS "-sLZ4=1")  # Using LZ4 compression for better performance
 
    # Combine the flags properly
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAGS} ${PRELOAD_FLAGS} ${COMPRESSION_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMPILE_FLAGS} ${PRELOAD_FLAGS} ${COMPRESSION_FLAGS}")

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
#!/bin/bash

# make build dir if it doesn't exist
mkdir -p build

# setup emscripten build
cd build
emcmake cmake ..
emmake make

# copy built files to public
mkdir -p ../public
cp f22_game.* ../public/
cp -r ../assets ../public/
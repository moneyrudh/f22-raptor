# #!/bin/bash
# # install required tools
# npm install -g cmake-js

# # set up emscripten
# curl -L https://github.com/emscripten-core/emsdk/archive/main.tar.gz -o emsdk.tar.gz
# tar -xf emsdk.tar.gz
# cd emsdk-main
# ./emsdk install latest
# ./emsdk activate latest
# source ./emsdk_env.sh
# cd ..

# # create public directory (this is where vercel expects files)
# mkdir -p public
# cd public

# # run build commands
# emcmake cmake ..
# emmake make

# # ensure files are in public/
# cp f22_game.* ./
# cp -r ../assets ./
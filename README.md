# Development Environment Setup 
## Ubuntu (22.04+)
1. Install VS Code, cmake, gcc/g++/gdb, ninja and SDL dependencies using your package manager:
   ```bash
   sudo apt-get install build-essential git make cmake ninja-build \
   pkg-config cmake ninja-build gnome-desktop-testing libasound2-dev libpulse-dev \
   libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
   libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
   libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
   libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev libthai-dev \
   libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
   ```
   ```
2. Run `cmake --build --preset default`
3. The compiled executable can be found in `build/bin/NotA2DGame`
4. Optional: clangd setup https://clangd.llvm.org/installation

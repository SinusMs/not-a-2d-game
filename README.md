# Development Environment Setup (Windows)
**Note:** Development on other platforms or using a different cpp compiler or build system should work fine as well, though no instructions are provided here. 
1. Install [VSCode](https://code.visualstudio.com/download) and [MSYS2](https://www.msys2.org/#installation)
2. Install cmake, gcc/g++/gdb and ninja using MSYS2 UCRT64 Console: `pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja`
3. set g++/ninja binaries path in vscodes' settings.json file:
   ```settings.json
    "cmake.cmakePath": "C:/msys64/ucrt64/bin/cmake.exe",
    "cmake.generator": "C:/msys64/ucrt64/bin/ninja.exe",
    "C_Cpp.default.compilerPath": "C:/msys64/ucrt64/bin/g++.exe",
   ```
4. Install "CMake Tools" and "C/C++" VSCode Extensions
5. Build and run the application using the buttons in the bottom left of VSCodes UI
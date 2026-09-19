# SVG Viewer

The SVG viewer is a desktop application built with CMake, SDL3, and SDL3_image.

## Dependencies

- A C and C++ compiler
- CMake 3.26 or newer
- SDL3
- SDL3_image

## Install dependencies

### macOS

Install the Xcode command-line tools and Homebrew packages:

```sh
xcode-select --install
brew install cmake sdl3 sdl3_image
```

If Homebrew is not installed, follow the instructions at
[brew.sh](https://brew.sh/).

### Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload,
including its CMake component. Then install SDL through
[vcpkg](https://learn.microsoft.com/vcpkg/get_started/get-started):

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg.exe install sdl3 sdl3-image --triplet x64-windows
```

Pass the vcpkg toolchain file when configuring:

```powershell
cmake -S svgviewer -B build `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

Use `arm64-windows` instead of `x64-windows` when building for Windows on
Arm.

### Linux

Install a compiler, CMake, SDL3, and SDL3_image using your distribution's
package manager.

**Debian/Ubuntu:**

```sh
sudo apt update
sudo apt install build-essential cmake libsdl3-dev libsdl3-image-dev
```

**Fedora:**

```sh
sudo dnf install gcc-c++ cmake SDL3-devel SDL3_image-devel
```

**Arch Linux:**

```sh
sudo pacman -S --needed base-devel cmake sdl3 sdl3_image
```

If your distribution does not provide SDL3 packages yet, use vcpkg:

```sh
git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
"$HOME/vcpkg/bootstrap-vcpkg.sh"
"$HOME/vcpkg/vcpkg" install sdl3 sdl3-image
cmake -S svgviewer -B build \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Build

Run these commands from the repository root after installing the
dependencies:

```sh
cmake -S svgviewer -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

On macOS and Linux, the executable is normally `build/svgviewer`. With a
multi-configuration generator such as Visual Studio, it is normally
`build/Release/svgviewer.exe`.

## Run

Pass an SVG file to the executable:

```sh
./build/svgviewer samples/ball_bounce.svg
```

On Windows:

```powershell
.\build\Release\svgviewer.exe .\samples\ball_bounce.svg
```

Run the viewer with `--help` to see all available command-line options.

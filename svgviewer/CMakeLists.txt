cmake_minimum_required(VERSION 3.26)
project(svgviewer C CXX)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

add_executable(svgviewer)

target_include_directories(svgviewer PUBLIC ../src)

target_compile_options(svgviewer PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-fcolor-diagnostics;-fansi-escape-codes>
)

target_sources(svgviewer
PRIVATE
    svgviewer.cpp
    ../src/AnimatedSVG.cpp
)

target_link_libraries(svgviewer SDL3::SDL3 SDL3_image::SDL3_image)

cmake_minimum_required(VERSION 3.15)
project(fft)

set(CMAKE_CXX_STANDARD 14)


file (GLOB SOURCES *.cpp)
add_executable(fft ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/kiss_fft/kiss_fft.c")

set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL")
set(PORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/PORTAUDIO/.libs")

target_link_libraries(fft -L${SDL_DIR})
target_link_libraries(fft SDL2.lib SDL2main.lib SDL2.dll)

target_link_libraries(fft -L${PORT_DIR})
target_link_libraries(fft libportaudio.dll.a)
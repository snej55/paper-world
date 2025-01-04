cmake_minimum_required(VERSION 3.15...3.31)
project(PaperWorld VERSION 1.0
                  DESCRIPTION "Pixel art platformer"
                  LANGUAGES CXX)


set(CMAKE_CPP_COMPILER g++)

# for release
#set(CMAKE_CXX_FLAGS -mwindows)

# sources
set(SOURCES main.cpp src/anim.cpp src/entities.cpp src/health_bars.cpp src/particles.cpp src/player.cpp src/timer.cpp src/weapons.cpp)

# -Iinclude
include_directories(${CMAKE_SOURCE_DIR}/include)
# -Llib
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(main ${SOURCES})

# libraries to compile with -lSDL2main ...
set(SDL2_LIBRARIES mingw32 SDL2main SDL2 SDL2_image)

# link it so ld can find it
target_link_libraries(main PUBLIC ${SDL2_LIBRARIES})
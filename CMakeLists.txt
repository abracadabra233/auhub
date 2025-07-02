cmake_minimum_required(VERSION 3.16)
project(auhub)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Vcpkg integration
# Make sure CMAKE_TOOLCHAIN_FILE is set when configuring CMake, e.g.:
# cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
find_package(spdlog CONFIG REQUIRED)
find_package(Sndfile CONFIG REQUIRED) # libsndfile provides Sndfile target
find_package(PortAudio CONFIG REQUIRED)

message(STATUS "Found spdlog: ${spdlog_FOUND}")
message(STATUS "Found Sndfile: ${Sndfile_FOUND}")
message(STATUS "Found PortAudio: ${PortAudio_FOUND}")

add_subdirectory(3rdparty/uart)

file(GLOB AUDIO_SRC "src/audio/*.cc")
file(GLOB PLAYER_SRC "src/player/*.cc")
add_library(${PROJECT_NAME} ${AUDIO_SRC} ${PLAYER_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC 
    uart
    Sndfile::libsndfile # Official target name for libsndfile
    PortAudio::portaudio # Official target name for portaudio
    spdlog::spdlog)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(examples)
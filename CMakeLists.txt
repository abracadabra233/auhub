cmake_minimum_required(VERSION 3.16)
project(auhub)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_library(PortAudio_LIBRARY NAMES portaudio HINTS /usr/local/lib /usr/lib)
find_library(SndFile_LIBRARY NAMES sndfile HINTS /usr/local/lib /usr/lib)

add_subdirectory(3rdparty/uart)

file(GLOB AUDIO_SRC "src/audio/*.cc")
file(GLOB PLAYER_SRC "src/player/*.cc")
add_library(${PROJECT_NAME} ${AUDIO_SRC} ${PLAYER_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC 
     uart
     ${PortAudio_LIBRARY} 
     ${SndFile_LIBRARY} 
     spdlog::spdlog)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(examples)
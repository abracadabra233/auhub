find_library(PortAudio_LIBRARY NAMES portaudio HINTS /usr/local/lib /usr/lib)
find_library(SndFile_LIBRARY NAMES sndfile HINTS /usr/local/lib /usr/lib)
find_package(spdlog CONFIG REQUIRED)


file(GLOB TEST_SRC "./*.cc")
foreach(exec_file_path ${TEST_SRC})
    get_filename_component(exec_name ${exec_file_path} NAME_WE)

    add_executable(${exec_name} ${exec_file_path})
    target_link_libraries(${exec_name} uart 
        ${PortAudio_LIBRARY} 
        ${SndFile_LIBRARY}
        spdlog::spdlog)
    install(TARGETS ${exec_name} DESTINATION lib/${PROJECT_NAME})

endforeach()

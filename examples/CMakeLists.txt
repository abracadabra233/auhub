find_package(cxxopts CONFIG REQUIRED)

file(GLOB SRC "*.cc")

foreach(SRC_FILE ${SRC})
    get_filename_component(EXECUTABLE_NAME ${SRC_FILE} NAME_WE)

    add_executable(${EXECUTABLE_NAME} ${SRC_FILE})
    target_link_libraries(${EXECUTABLE_NAME}
        cxxopts::cxxopts
        auhub)
endforeach()
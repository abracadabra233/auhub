cmake_minimum_required(VERSION 3.10)
project(uart)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    # 对于MSVC编译器
    add_compile_options(/W0)  # 禁用所有警告
    # 或者针对特定警告
    # add_compile_options(/wd4996)  # 禁用特定警告号
else()
    # 对于GCC/Clang编译器
    add_compile_options(-w)    # 禁用所有警告
    # 或者
    # add_compile_options(-Wno-all)  # 禁用所有警告
    # 或者针对特定警告
    # add_compile_options(-Wno-unused-variable)  # 禁用特定警告
endif()

# 创建库之前先创建构建目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/_obj)

# 收集所有源文件
set(LIB_SOURCES
    ./ringbuf.c
    ./uart_common.c
)

# 收集所有头文件
set(LIB_HEADERS
    ./ringbuf.h
    ./uart_common.h
)

# 创建静态库
add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES} ${LIB_HEADERS})

# 设置库的包含路径
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库需要的系统库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    pthread
    rt
    m
    asound
)

add_subdirectory(examples)

# 安装规则（可选）
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
)

# 设置object文件输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_obj)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_obj)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_obj)
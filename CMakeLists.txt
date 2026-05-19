cmake_minimum_required(VERSION 3.10)
project(kvstore CXX) 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 导出编译命令
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找需要的库
find_package(Threads REQUIRED)

# 收集源文件
file(GLOB SOURCES 
    "src/*.cpp"
    "src/*.c"
)

# 如果没有找到源文件，列出已知的源文件
if(NOT SOURCES)
    set(SOURCES
        src/kv_core.cpp
        src/main.cpp
    )
endif()

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 添加头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE include)

# 链接线程库
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# 添加编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -g)
endif()
cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME LIB_LED)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

enable_language(C ASM)

# ── 宏定义 ────────────────────────────────────────────────
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

# ── 源文件 ────────────────────────────────────────────────
add_executable(${CMAKE_PROJECT_NAME}
    # 启动文件 & 系统初始化
    startup_stm32f407xx.s
    Core/Src/system_stm32f4xx.c

    # 应用层
    Core/Src/main.c

    # 自建驱动库
    Lib/Src/rcc.c
    Lib/Src/gpio.c
)

# ── 头文件搜索路径 ─────────────────────────────────────────
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Inc
    Lib/Inc
)

# 构建产物列表（用于 clean 目标）
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

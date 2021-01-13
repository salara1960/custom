cmake_minimum_required(VERSION 3.17)
project(custom_vk80ii_sk LANGUAGES C)

# Common build configs #################################################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

########################################################################################################################
add_library(${PROJECT_NAME} STATIC
        functions.c
        functions.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE iconv_hook)

########################################################################################################################
add_executable(${PROJECT_NAME}_main
        main.c
)
target_link_libraries(${PROJECT_NAME}_main PRIVATE ${PROJECT_NAME})

########################################################################################################################
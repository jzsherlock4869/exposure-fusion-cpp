cmake_minimum_required(VERSION 2.8)
project(ExposureFusion)

find_package(OpenCV REQUIRED)

set(MY_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/exposure_fusion/include
                    ${OpenCV_INCLUDE_DIRS}
                    )
include_directories(${MY_INCLUDE_DIRS})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/exposure_fusion/src MY_SRC)
message(STATUS "MY_SRC=${MY_SRC}")

# add_library(MY_LIB STATIC ${MY_SRC})

add_executable(${PROJECT_NAME} main.cpp ${MY_SRC})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
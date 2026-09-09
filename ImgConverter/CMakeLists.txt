cmake_minimum_required(VERSION 3.11)

project(ImgConverter CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ImgLib" ImgLibBuildDir)

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

add_executable(imgconv main.cpp)
target_link_libraries(imgconv PRIVATE ImgLib ${SYSTEM_LIBS})

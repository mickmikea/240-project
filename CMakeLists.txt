cmake_minimum_required (VERSION 3.1 FATAL_ERROR)
project(termedit)

SET(SOURCE_FILES
	src/main.cpp
        src/read_write_file.cpp
        src/edit.cpp
)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})

cmake_minimum_required(VERSION 3.16)

project(
	"When the factory"
	VERSION
	0.1
	LANGUAGES
	C
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	"${CMAKE_SOURCE_DIR}/cmake/"
)

file(GLOB_RECURSE SRC_FILES src/*.c)
add_compile_options(
	-Wall
	-Wextra
	-pedantic
	-O2
	-std=c17
)
add_executable(wtf
	${SRC_FILES}
)


target_include_directories(wtf
	PRIVATE
	${PROJECT_SOURCE_DIR}/include
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${SDL2_TTF_INCLUDE_DIRS}
)
target_link_libraries(wtf
	${SDL2_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
	${SDL2_TTF_LIBRARIES}
)

set_target_properties(wtf
	PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/bin
)

cmake_minimum_required(VERSION 3.10)

project(signature
VERSION 0.1.000001
DESCRIPTION
"The program which splits the input file on blocks with the selected size \
and computes the signature of the each block. The signatures is saved in \
output file.")


set (CMAKE_CXX_STANDARD 20)
add_definitions(-Wall -Werror -O0 -pthread)


SET(SRC
	${PROJECT_SOURCE_DIR}/WorkerManager.cpp
	${PROJECT_SOURCE_DIR}/Worker.cpp
	${PROJECT_SOURCE_DIR}/algos/HasherCrc32.cpp
	${PROJECT_SOURCE_DIR}/algos/HasherMd5.cpp
	${PROJECT_SOURCE_DIR}/common/Config.cpp
	${PROJECT_SOURCE_DIR}/common/FileReader.cpp
	${PROJECT_SOURCE_DIR}/common/Logger.cpp
	${PROJECT_SOURCE_DIR}/common/LoggerManager.cpp
	${PROJECT_SOURCE_DIR}/main.cpp
)
add_executable(signature ${SRC})
target_link_libraries (signature pthread)



configure_file(BuildVersion.hpp.in BuildVersion.hpp)
target_include_directories (
	signature PUBLIC
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}/common"
	"${PROJECT_SOURCE_DIR}/algos"
)


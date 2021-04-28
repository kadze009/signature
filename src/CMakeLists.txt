cmake_minimum_required(VERSION 3.15)

project(signature
VERSION 1.0.000001
DESCRIPTION
"The application splits the input file on blocks of selected size and computes\\n\\\n\
    the signature of each block simultaneously. The signatures are saved to\\n\\\n\
    output file."
	)

# Specify a Release build by default
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
	message(STATUS "Build type not specified: Use Release by default")
endif(NOT CMAKE_BUILD_TYPE)

configure_file(BuildVersion.hpp.in BuildVersion.hpp)


add_executable(signature
	algo/HasherFactory.cpp
	algo/HasherCrc32.cpp
	algo/HasherMd5.cpp
	common/MpocQueue.cpp
	common/MpocQueueProducer.cpp
	common/FileReader.cpp
	common/FileWriter.cpp
	common/Logger.cpp
	Config.cpp
	LoggerManager.cpp
	WorkerManager.cpp
	Worker.cpp
	main.cpp
	)


target_include_directories(signature
	PRIVATE "${PROJECT_BINARY_DIR}"
	PRIVATE "${PROJECT_SOURCE_DIR}"
	PRIVATE "${PROJECT_SOURCE_DIR}/algo"
	PRIVATE "${PROJECT_SOURCE_DIR}/common"
	)


set_target_properties(signature PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	)



target_compile_options(signature PRIVATE
	-Wall -Werror -Wextra -pedantic -pthread
	)

set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -DRELEASE_BUILD -O3")

target_link_libraries(signature
	PRIVATE pthread
	)

cmake_minimum_required(VERSION 2.8)
project(eos_server)


# google-glog compilation and linking libglog.so
message(STATUS "Building glog in ${CMAKE_BINARY_DIR}/libs/glog-compile...")
execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/libs/glog-compile)
execute_process(
	COMMAND ${CMAKE_SOURCE_DIR}/libs/glog/configure --prefix=${CMAKE_BINARY_DIR}/libs/glog
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/libs/glog-compile
	OUTPUT_QUIET
	OUTPUT_FILE "configure.log"
)
message(STATUS "Installing glog in ${CMAKE_BINARY_DIR}/libs/glog...")
execute_process(
	COMMAND make install
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/libs/glog-compile
	OUTPUT_QUIET
)
set(GLOG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libs/glog/include)
find_library(GLOG_LIBRARY
	NAMES glog
	PATHS
		${CMAKE_BINARY_DIR}/libs/glog/lib/
)
set(GLOG_LIBRARIES ${GLOG_LIBRARY})
message(STATUS "GLog includes: ${GLOG_INCLUDE_DIR} Libraries: ${GLOG_LIBRARIES}" )

# Boost thread library required
find_package(Boost 1.48 COMPONENTS thread REQUIRED)

include_directories(
	${Boost_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/utils
	${CMAKE_SOURCE_DIR}/server
	${GLOG_INCLUDE_DIR}
)

# Subfolders contain source files (temporary no libraryes, only one executable)
file(GLOB utils_SOURCES ${CMAKE_SOURCE_DIR}/utils/*.hpp ${CMAKE_SOURCE_DIR}/utils/*.cpp)
file(GLOB server_SOURCES ${CMAKE_SOURCE_DIR}/server/*.hpp ${CMAKE_SOURCE_DIR}/server/*.cpp)

add_executable(${PROJECT_NAME} ${utils_SOURCES} ${server_SOURCES} main.cpp)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ev ${GLOG_LIBRARIES})

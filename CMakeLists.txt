cmake_minimum_required(VERSION 2.8)
project(eos_server)


# google-glog
set(GLOG_LIB_DIR ${CMAKE_BINARY_DIR}/libs/glog/lib)
message(STATUS "Searching for glog library: ${GLOG_LIB_DIR}")
set(GLOG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libs/glog/include)
find_library(GLOG_LIBRARY
	NAMES glog
	PATHS ${GLOG_LIB_DIR}
)
if(NOT GLOG_LIBRARY)
	message(STATUS "Searching for glog library: ${GLOG_LIB_DIR} -- not found")
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
		COMMAND make install -j4
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/libs/glog-compile
		OUTPUT_QUIET
	)
	set(GLOG_LIBRARY ${CMAKE_BINARY_DIR}/libs/glog/lib/libglog.so)
endif(NOT GLOG_LIBRARY)
set(GLOG_LIBRARIES ${GLOG_LIBRARY})
message(STATUS "GLog includes: ${GLOG_INCLUDE_DIR} Libraries: ${GLOG_LIBRARIES}" )

# protocol
message(STATUS "Generating protocol...")
file(GLOB proto_files RELATIVE ${CMAKE_SOURCE_DIR}/protocol/proto protocol/proto/*.proto)
execute_process(
	COMMAND protoc --proto_path=. --cpp_out=../src ${proto_files}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/protocol/proto
)

# Boost thread library required
find_package(Boost 1.48 COMPONENTS thread program_options REQUIRED)

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

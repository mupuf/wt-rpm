cmake_minimum_required(VERSION 2.6)

include(CheckIncludeFileCXX)

include_directories(BEFORE "${CMAKE_BINARY_DIR}")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}")

project(wt-rpm)

set(PROJECT_NAME "Wt Remote Power Manager")
set(PROJECT_SHORT_NAME "Wt_RPM")
set(PROJECT_URL "http://mupuf.org/project/ipmi/")
set(PROJECT_AUTHORS "Martin Peres")
set(PROJECT_ORGANIZATION "MuPuF.org")
set(PROJECT_LICENSE "GPLv2 or later")
set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_STATE "alpha")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}-${PROJECT_VERSION_STATE}")
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DPROJECT_SHORT_NAME="${PROJECT_SHORT_NAME}")
add_definitions(-DPROJECT_URL="${PROJECT_URL}")
add_definitions(-DPROJECT_AUTHORS="${PROJECT_AUTHORS}")
add_definitions(-DPROJECT_ORGANIZATION="${PROJECT_ORGANIZATION}")
add_definitions(-DPROJECT_LICENSE="${PROJECT_LICENSE}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=gnu++0x")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

# Boost
SET(Boost_ADDITIONAL_VERSIONS
  "1.35" "1.36" "1.37" "1.38" "1.39"
  "1.40" "1.41" "1.42" "1.43" "1.44" "1.45" "1.46" "1.47" "1.48" "1.49"
  "1.50" "1.51" "1.52" "1.53" "1.54" "1.55" "1.56" "1.57" "1.58" "1.59")
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost 1.35 COMPONENTS signals thread system REQUIRED)
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

# PThread
find_package(Threads REQUIRED)

# Wt (I would advise to compile a minimal Wt to lower dependencies)
include(FindWt REQUIRED)
include_directories(${Wt_INCLUDE_DIR})

# Select the connector (FCGI by default)
option(USE_CON_HTTP "Build wt-rpm with an HTTP connector instead of an FCGI one" OFF)
if(USE_CON_HTTP)
	set (Wt_CONNECTOR "${Wt_HTTP_LIBRARY}")
else(USE_CON_HTTP)
	set (Wt_CONNECTOR "${Wt_FCGI_LIBRARY}")
endif(USE_CON_HTTP)

# liboping
option(USE_PING "Build wt-rpm with ping support" ON)
if(USE_PING AND (EXISTS "/usr/include/oping.h" AND EXISTS "/usr/lib/liboping.so"))
	set(USE_PING "ON")
	add_definitions(-DUSE_PING=1)
	set(PING_LIBRARIES "-loping")
else()
	set(USE_PING "OFF")
endif()

# Output binary
file(GLOB wtcomments_SOURCES "*.cpp" "*.h")
add_executable(wt_rpm ${wtcomments_SOURCES})
target_link_libraries(wt_rpm ${Wt_LIBRARY} ${Wt_CONNECTOR} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${PING_LIBRARIES})

message("")
message("-- Summary --")
message("	Build type                               : ${CMAKE_BUILD_TYPE} (-DCMAKE_BUILD_TYPE)")
message("	Wt connector, use HTTP instead of FCGI   : ${USE_CON_HTTP} (-DUSE_CON_HTTP)")
message("	Use pings (requires liboping)            : ${USE_PING} (-DUSE_PING)")
message("")

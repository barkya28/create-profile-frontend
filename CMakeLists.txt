cmake_minimum_required(VERSION 3.16)
project(GamingAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Oracle Instant Client paths (adjust these based on your Oracle installation)
set(ORACLE_HOME "/opt/oracle/instantclient_21_1")
set(ORACLE_INCLUDE_DIR "${ORACLE_HOME}/sdk/include")
set(ORACLE_LIB_DIR "${ORACLE_HOME}")

# Include directories
include_directories(${ORACLE_INCLUDE_DIR})
include_directories(src/include)

# Link directories
link_directories(${ORACLE_LIB_DIR})

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/include/*.h")

# Create executable
add_executable(gaming_api ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(gaming_api 
    clntsh
    occi
    Threads::Threads
)

# Compiler flags
target_compile_options(gaming_api PRIVATE -Wall -Wextra -O2)

# Install target
install(TARGETS gaming_api DESTINATION bin)
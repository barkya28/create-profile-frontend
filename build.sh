#!/bin/bash

# Gaming API Build Script
# This script builds the C++ Gaming API with Oracle database support

set -e  # Exit on any error

echo "Gaming API Build Script"
echo "======================"

# Check if Oracle Instant Client is installed
if [ ! -d "/opt/oracle/instantclient_21_1" ]; then
    echo "Warning: Oracle Instant Client not found at /opt/oracle/instantclient_21_1"
    echo "Please install Oracle Instant Client or adjust paths in CMakeLists.txt"
    echo ""
fi

# Set Oracle environment variables
export ORACLE_HOME=/opt/oracle/instantclient_21_1
export LD_LIBRARY_PATH=$ORACLE_HOME:$LD_LIBRARY_PATH
export PATH=$ORACLE_HOME:$PATH

echo "Oracle Home: $ORACLE_HOME"
echo ""

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Clean previous build if requested
if [ "$1" == "clean" ]; then
    echo "Cleaning previous build..."
    rm -rf *
fi

# Configure with CMake
echo "Configuring with CMake..."
cmake .. || {
    echo "CMake configuration failed!"
    echo "Make sure you have:"
    echo "1. CMake 3.16 or later installed"
    echo "2. C++17 compatible compiler (GCC 7+ or Clang 10+)"
    echo "3. Oracle Instant Client installed"
    exit 1
}

# Build the project
echo "Building the project..."
make -j$(nproc) || {
    echo "Build failed!"
    echo "Check the error messages above for details."
    exit 1
}

echo ""
echo "Build completed successfully!"
echo "Executable: ./gaming_api"
echo ""
echo "To run the server:"
echo "1. Make sure Oracle database is running"
echo "2. Run the database schema script: sqlplus gaming_user/gaming_password@localhost:1521/XE @../database/schema.sql"
echo "3. Execute: ./gaming_api"
echo ""
echo "Server will start on http://localhost:8080"
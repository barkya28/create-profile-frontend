#!/bin/bash

# Gaming API Development Setup Script
# This script helps setup the development environment

set -e

echo "Gaming API Development Setup"
echo "============================"
echo ""

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install packages on Ubuntu/Debian
install_ubuntu_packages() {
    echo "Installing required packages for Ubuntu/Debian..."
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        curl \
        wget \
        unzip \
        libaio1 \
        libaio-dev \
        pkg-config
}

# Function to install packages on CentOS/RHEL/Fedora
install_redhat_packages() {
    echo "Installing required packages for CentOS/RHEL/Fedora..."
    if command_exists dnf; then
        sudo dnf install -y \
            gcc-c++ \
            cmake \
            curl \
            wget \
            unzip \
            libaio \
            libaio-devel \
            pkgconfig
    else
        sudo yum install -y \
            gcc-c++ \
            cmake \
            curl \
            wget \
            unzip \
            libaio \
            libaio-devel \
            pkgconfig
    fi
}

# Check system requirements
echo "Checking system requirements..."

# Check for C++ compiler
if ! command_exists g++ && ! command_exists clang++; then
    echo "❌ C++ compiler not found!"
    echo "Installing development tools..."
    
    if [ -f /etc/debian_version ]; then
        install_ubuntu_packages
    elif [ -f /etc/redhat-release ]; then
        install_redhat_packages
    else
        echo "Unsupported distribution. Please install:"
        echo "- g++ or clang++ (C++17 support)"
        echo "- cmake (3.16+)"
        echo "- libaio development packages"
        exit 1
    fi
else
    echo "✅ C++ compiler found"
fi

# Check for CMake
if ! command_exists cmake; then
    echo "❌ CMake not found!"
    echo "Please install CMake 3.16 or later"
    exit 1
else
    cmake_version=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "✅ CMake found (version $cmake_version)"
fi

# Check for Oracle Instant Client
ORACLE_HOME="/opt/oracle/instantclient_21_1"
if [ ! -d "$ORACLE_HOME" ]; then
    echo "❌ Oracle Instant Client not found at $ORACLE_HOME"
    echo ""
    echo "To install Oracle Instant Client:"
    echo "1. Download from: https://www.oracle.com/database/technologies/instant-client/downloads.html"
    echo "2. Download these packages:"
    echo "   - instantclient-basic-linux.x64-21.1.0.0.0.zip"
    echo "   - instantclient-devel-linux.x64-21.1.0.0.0.zip"
    echo "3. Extract to /opt/oracle/instantclient_21_1/"
    echo "4. Create symlinks:"
    echo "   cd /opt/oracle/instantclient_21_1"
    echo "   ln -s libclntsh.so.21.1 libclntsh.so"
    echo "   ln -s libocci.so.21.1 libocci.so"
    echo ""
    echo "Alternatively, you can:"
    echo "1. Create the directory: sudo mkdir -p /opt/oracle"
    echo "2. Download and extract Oracle Instant Client there"
    echo ""
    
    read -p "Do you want to create the Oracle directory structure? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo mkdir -p /opt/oracle
        echo "Directory created. Please download and extract Oracle Instant Client manually."
    fi
else
    echo "✅ Oracle Instant Client directory found"
    
    # Check for required libraries
    if [ -f "$ORACLE_HOME/libclntsh.so" ] && [ -f "$ORACLE_HOME/libocci.so" ]; then
        echo "✅ Oracle libraries found"
    else
        echo "⚠️  Oracle libraries not found or not properly linked"
        echo "Make sure you have libclntsh.so and libocci.so in $ORACLE_HOME"
    fi
fi

# Set up environment variables
echo ""
echo "Setting up environment variables..."

ENV_SETUP="
# Oracle Instant Client Environment
export ORACLE_HOME=$ORACLE_HOME
export LD_LIBRARY_PATH=\$ORACLE_HOME:\$LD_LIBRARY_PATH
export PATH=\$ORACLE_HOME:\$PATH
"

# Add to ~/.bashrc if not already present
if ! grep -q "ORACLE_HOME.*instantclient" ~/.bashrc 2>/dev/null; then
    echo "Adding Oracle environment variables to ~/.bashrc"
    echo "$ENV_SETUP" >> ~/.bashrc
    echo "✅ Environment variables added to ~/.bashrc"
    echo "Please run: source ~/.bashrc"
else
    echo "✅ Oracle environment variables already in ~/.bashrc"
fi

echo ""
echo "Development Environment Check Complete!"
echo "======================================"
echo ""

# Create a sample database connection test
cat > test_db_connection.sql << 'EOF'
-- Test database connection script
-- Run this with: sqlplus gaming_user/gaming_password@localhost:1521/XE @test_db_connection.sql

SELECT 'Database connection successful!' as status FROM dual;
SELECT SYSDATE as current_time FROM dual;
SELECT USER as current_user FROM dual;

-- Check if tables exist
SELECT table_name, num_rows 
FROM user_tables 
WHERE table_name IN ('USERS', 'OTPS', 'SCORES', 'GAMES', 'USER_SESSIONS');

EXIT;
EOF

echo "Created test_db_connection.sql for testing database connectivity"
echo ""

echo "Next steps:"
echo "1. Install Oracle Database (if not already installed)"
echo "2. Run the database schema: sqlplus gaming_user/gaming_password@localhost:1521/XE @database/schema.sql"
echo "3. Test database connection: sqlplus gaming_user/gaming_password@localhost:1521/XE @test_db_connection.sql"
echo "4. Build the project: ./build.sh"
echo "5. Run the server: cd build && ./gaming_api"
echo "6. Test the API: ./test_api.sh"
echo ""
echo "For production deployment, make sure to:"
echo "- Use strong passwords"
echo "- Configure proper Oracle security"
echo "- Set up SSL/TLS"
echo "- Implement proper logging"
echo "- Set up monitoring"
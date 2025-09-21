#!/bin/bash

# Gaming API Test Script
# This script tests all the API endpoints

set -e

API_BASE="http://localhost:8080/api"
MOBILE="9876543210"
OTP="1234"
USER_NAME="John Doe"

echo "Gaming API Test Script"
echo "====================="
echo "Testing API endpoints at $API_BASE"
echo ""

# Function to make API calls and display results
make_api_call() {
    local method=$1
    local endpoint=$2
    local data=$3
    local description=$4
    
    echo "Testing: $description"
    echo "Endpoint: $method $endpoint"
    
    if [ "$method" == "GET" ]; then
        response=$(curl -s -w "\nHTTP_STATUS:%{http_code}" "$API_BASE$endpoint")
    else
        response=$(curl -s -w "\nHTTP_STATUS:%{http_code}" -X "$method" \
            -H "Content-Type: application/json" \
            -d "$data" \
            "$API_BASE$endpoint")
    fi
    
    # Extract HTTP status and body
    http_status=$(echo "$response" | grep "HTTP_STATUS:" | cut -d: -f2)
    body=$(echo "$response" | sed '/HTTP_STATUS:/d')
    
    echo "Status: $http_status"
    echo "Response: $body"
    echo ""
    
    # Check if request was successful (2xx status codes)
    if [[ $http_status -ge 200 && $http_status -lt 300 ]]; then
        echo "✅ Success"
    else
        echo "❌ Failed"
    fi
    echo "----------------------------------------"
    echo ""
}

# Wait for user to start the server
echo "Make sure the Gaming API server is running on port 8080"
echo "Press Enter to continue with testing..."
read

# Test 1: Send OTP
make_api_call "POST" "/send-otp" \
    "{\"mobile\":\"$MOBILE\"}" \
    "Send OTP to mobile number"

# Wait a moment before next test
sleep 1

# Test 2: Verify OTP with wrong OTP (should fail)
make_api_call "POST" "/verify-otp" \
    "{\"mobile\":\"$MOBILE\",\"otp\":\"0000\"}" \
    "Verify OTP with wrong code (should fail)"

# Test 3: Verify OTP with correct OTP
make_api_call "POST" "/verify-otp" \
    "{\"mobile\":\"$MOBILE\",\"otp\":\"$OTP\"}" \
    "Verify OTP with correct code"

# Test 4: Send OTP again for registration
make_api_call "POST" "/send-otp" \
    "{\"mobile\":\"$MOBILE\"}" \
    "Send OTP again for registration"

sleep 1

# Test 5: Register user
make_api_call "POST" "/register" \
    "{\"mobile\":\"$MOBILE\",\"name\":\"$USER_NAME\",\"otp\":\"$OTP\"}" \
    "Register new user"

# Test 6: Try to register same user again (should fail)
make_api_call "POST" "/send-otp" \
    "{\"mobile\":\"$MOBILE\"}" \
    "Send OTP for duplicate registration test"

sleep 1

make_api_call "POST" "/register" \
    "{\"mobile\":\"$MOBILE\",\"name\":\"$USER_NAME\",\"otp\":\"$OTP\"}" \
    "Try to register duplicate user (should fail)"

# Test 7: Save score
make_api_call "POST" "/save-score" \
    "{\"mobile\":\"$MOBILE\",\"score\":\"1500\",\"game_name\":\"Snake\"}" \
    "Save game score"

# Test 8: Save another score
make_api_call "POST" "/save-score" \
    "{\"mobile\":\"$MOBILE\",\"score\":\"2000\",\"game_name\":\"Tetris\"}" \
    "Save another game score"

# Test 9: Get overall dashboard
make_api_call "GET" "/dashboard/overall" \
    "" \
    "Get overall dashboard"

# Test 10: Get weekly dashboard
make_api_call "GET" "/dashboard/weekly" \
    "" \
    "Get weekly dashboard"

# Test 11: Test invalid endpoints
make_api_call "GET" "/invalid-endpoint" \
    "" \
    "Test invalid endpoint (should return 404)"

# Test 12: Test with missing parameters
make_api_call "POST" "/send-otp" \
    "{}" \
    "Send OTP with missing mobile (should fail)"

echo "Testing completed!"
echo ""
echo "Summary:"
echo "- OTP is hardcoded to 1234 with 1-minute expiry"
echo "- All endpoints tested for basic functionality"
echo "- Error cases tested for validation"
echo ""
echo "Check the server logs for detailed information about each request."
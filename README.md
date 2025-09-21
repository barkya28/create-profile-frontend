# Gaming Website API

A C++ REST API server with Oracle database integration for a gaming website. Users can register, save scores, and access dashboard analytics.

## Features

- **User Registration**: Register users with mobile number and OTP verification
- **OTP System**: Send and verify OTP with 1-minute expiry (hardcoded to 1234 for development)
- **Score Management**: Save and retrieve game scores
- **Dashboard Analytics**: Overall and weekly dashboard statistics
- **Oracle Database**: Full Oracle database integration with OCCI

## Tech Stack

- **Backend**: C++ with custom HTTP server
- **Database**: Oracle Database with OCCI (Oracle C++ Call Interface)
- **Build System**: CMake

## Prerequisites

1. **Oracle Database**: Oracle Database 19c or later
2. **Oracle Instant Client**: Version 21.1 or later
3. **C++ Compiler**: GCC 7+ or Clang 10+ with C++17 support
4. **CMake**: Version 3.16 or later

## Installation

### 1. Install Oracle Instant Client

```bash
# Download Oracle Instant Client from Oracle website
# Extract to /opt/oracle/instantclient_21_1/
# Or adjust paths in CMakeLists.txt accordingly

# Set environment variables
export ORACLE_HOME=/opt/oracle/instantclient_21_1
export LD_LIBRARY_PATH=$ORACLE_HOME:$LD_LIBRARY_PATH
export PATH=$ORACLE_HOME:$PATH
```

### 2. Setup Database

```bash
# Connect to Oracle as DBA user
sqlplus sys/password@localhost:1521/XE as sysdba

# Run the schema creation script
@database/schema.sql
```

### 3. Build the Application

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Or build with specific number of jobs
make -j4
```

### 4. Configure Database Connection

Edit the database connection details in `src/main.cpp`:

```cpp
std::string connectionString = "localhost:1521/XE";  // Your Oracle connection
std::string username = "gaming_user";               // Database user
std::string password = "gaming_password";           // Database password
```

### 5. Run the Server

```bash
# From build directory
./gaming_api
```

The server will start on `http://localhost:8080`

## API Endpoints

### 1. Send OTP
**POST** `/api/send-otp`

Send OTP to mobile number (hardcoded to 1234 with 1-minute expiry).

**Request:**
```json
{
    "mobile": "9876543210"
}
```

**Response:**
```json
{
    "status": "success",
    "message": "OTP sent successfully"
}
```

### 2. Verify OTP
**POST** `/api/verify-otp`

Verify the OTP code.

**Request:**
```json
{
    "mobile": "9876543210",
    "otp": "1234"
}
```

**Response:**
```json
{
    "status": "success",
    "message": "OTP verified successfully"
}
```

### 3. Register User
**POST** `/api/register`

Register a new user with OTP verification.

**Request:**
```json
{
    "mobile": "9876543210",
    "name": "John Doe",
    "otp": "1234"
}
```

**Response:**
```json
{
    "status": "success",
    "message": "User registered successfully"
}
```

### 4. Save Score
**POST** `/api/save-score`

Save a game score for a user.

**Request:**
```json
{
    "mobile": "9876543210",
    "score": "1500",
    "game_name": "Snake"
}
```

**Response:**
```json
{
    "status": "success",
    "message": "Score saved successfully"
}
```

### 5. Overall Dashboard
**GET** `/api/dashboard/overall`

Get overall statistics.

**Response:**
```json
{
    "status": "success",
    "total_users": "150",
    "total_games": "5",
    "highest_score": "2500",
    "message": "Overall dashboard data"
}
```

### 6. Weekly Dashboard
**GET** `/api/dashboard/weekly`

Get weekly statistics.

**Response:**
```json
{
    "status": "success",
    "weekly_users": "25",
    "weekly_games": "3",
    "weekly_highest_score": "1800",
    "message": "Weekly dashboard data"
}
```

## Database Schema

### Tables

1. **users**: User information
   - `user_id` (Primary Key)
   - `mobile_no` (Unique)
   - `name`
   - `created_at`
   - `updated_at`
   - `is_active`

2. **otps**: OTP storage with expiry
   - `otp_id` (Primary Key)
   - `mobile_no`
   - `otp_code`
   - `expiry_time`
   - `created_at`
   - `is_used`

3. **scores**: Game scores
   - `score_id` (Primary Key)
   - `user_id` (Foreign Key)
   - `score`
   - `game_name`
   - `created_at`

4. **games**: Game information
   - `game_id` (Primary Key)
   - `game_name` (Unique)
   - `description`
   - `created_at`
   - `is_active`

5. **user_sessions**: Session management
   - `session_id` (Primary Key)
   - `user_id` (Foreign Key)
   - `session_token`
   - `created_at`
   - `expires_at`
   - `is_active`

### Views

- `v_overall_dashboard`: Overall statistics
- `v_weekly_dashboard`: Weekly statistics  
- `v_top_scores`: Top scores by game

### Procedures

- `clean_expired_otps`: Remove expired OTP codes
- `clean_expired_sessions`: Remove expired sessions

## Testing

### Using curl

```bash
# Send OTP
curl -X POST http://localhost:8080/api/send-otp \
  -H "Content-Type: application/json" \
  -d '{"mobile":"9876543210"}'

# Verify OTP
curl -X POST http://localhost:8080/api/verify-otp \
  -H "Content-Type: application/json" \
  -d '{"mobile":"9876543210","otp":"1234"}'

# Register User
curl -X POST http://localhost:8080/api/register \
  -H "Content-Type: application/json" \
  -d '{"mobile":"9876543210","name":"John Doe","otp":"1234"}'

# Save Score
curl -X POST http://localhost:8080/api/save-score \
  -H "Content-Type: application/json" \
  -d '{"mobile":"9876543210","score":"1500","game_name":"Snake"}'

# Get Overall Dashboard
curl http://localhost:8080/api/dashboard/overall

# Get Weekly Dashboard
curl http://localhost:8080/api/dashboard/weekly
```

## Development

### Project Structure

```
gaming-api/
├── CMakeLists.txt
├── README.md
├── database/
│   └── schema.sql
├── src/
│   ├── include/
│   │   ├── database.h
│   │   └── api_server.h
│   ├── database.cpp
│   ├── api_server.cpp
│   └── main.cpp
└── build/
    └── (generated files)
```

### Adding New Endpoints

1. Add route handler method to `APIServer` class in `api_server.h`
2. Implement the method in `api_server.cpp`
3. Register the route in `setupRoutes()` method
4. Add database methods in `DatabaseManager` if needed

### Error Handling

The API returns appropriate HTTP status codes:
- `200`: Success
- `400`: Bad Request (invalid input)
- `404`: Not Found (user/resource not found)
- `500`: Internal Server Error

## Security Considerations

- OTP is hardcoded for development (1234)
- No authentication/authorization implemented
- No rate limiting on OTP requests
- No input sanitization beyond basic validation
- No HTTPS support (HTTP only)

For production use, implement:
- Real SMS gateway integration
- JWT-based authentication
- Rate limiting
- Input sanitization
- HTTPS support
- Logging and monitoring

## Troubleshooting

### Common Issues

1. **Oracle Connection Failed**
   - Check Oracle database is running
   - Verify connection string, username, password
   - Ensure Oracle Instant Client is properly installed
   - Check LD_LIBRARY_PATH includes Oracle library path

2. **Build Errors**
   - Ensure C++17 compiler support
   - Check Oracle include/library paths in CMakeLists.txt
   - Install required development packages

3. **Socket Binding Failed**
   - Port 8080 might be in use
   - Change port in main.cpp
   - Run with sudo if using port < 1024

## License

This project is for educational/development purposes.
#ifndef DATABASE_H
#define DATABASE_H

#include <occi.h>
#include <string>
#include <memory>
#include <chrono>

using namespace oracle::occi;

class DatabaseManager {
private:
    Environment* env;
    Connection* conn;
    std::string connectionString;
    std::string username;
    std::string password;

public:
    DatabaseManager(const std::string& connStr, const std::string& user, const std::string& pass);
    ~DatabaseManager();
    
    bool connect();
    void disconnect();
    bool isConnected();
    
    // OTP related methods
    bool storeOTP(const std::string& mobile, const std::string& otp, const std::chrono::system_clock::time_point& expiry);
    bool verifyOTP(const std::string& mobile, const std::string& otp);
    bool cleanExpiredOTPs();
    
    // User related methods
    bool createUser(const std::string& mobile, const std::string& name);
    bool userExists(const std::string& mobile);
    int getUserId(const std::string& mobile);
    
    // Score related methods
    bool saveScore(int userId, int score, const std::string& gameName);
    
    Connection* getConnection() { return conn; }
};

#endif // DATABASE_H
#include "database.h"
#include <iostream>
#include <sstream>

DatabaseManager::DatabaseManager(const std::string& connStr, const std::string& user, const std::string& pass)
    : env(nullptr), conn(nullptr), connectionString(connStr), username(user), password(pass) {
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

bool DatabaseManager::connect() {
    try {
        env = Environment::createEnvironment(Environment::DEFAULT);
        conn = env->createConnection(username, password, connectionString);
        
        std::cout << "Connected to Oracle database successfully" << std::endl;
        return true;
    } catch (SQLException& ex) {
        std::cerr << "Database connection failed: " << ex.getMessage() << std::endl;
        return false;
    }
}

void DatabaseManager::disconnect() {
    try {
        if (conn) {
            env->terminateConnection(conn);
            conn = nullptr;
        }
        if (env) {
            Environment::terminateEnvironment(env);
            env = nullptr;
        }
    } catch (SQLException& ex) {
        std::cerr << "Error disconnecting: " << ex.getMessage() << std::endl;
    }
}

bool DatabaseManager::isConnected() {
    return conn != nullptr;
}

bool DatabaseManager::storeOTP(const std::string& mobile, const std::string& otp, 
                               const std::chrono::system_clock::time_point& expiry) {
    try {
        // Convert expiry time to Oracle timestamp
        auto time_t_expiry = std::chrono::system_clock::to_time_t(expiry);
        
        // First, delete any existing OTP for this mobile
        Statement* stmt = conn->createStatement(
            "DELETE FROM otps WHERE mobile_no = :1"
        );
        stmt->setString(1, mobile);
        stmt->executeUpdate();
        conn->terminateStatement(stmt);
        
        // Insert new OTP
        stmt = conn->createStatement(
            "INSERT INTO otps (mobile_no, otp_code, expiry_time, created_at) "
            "VALUES (:1, :2, :3, SYSTIMESTAMP)"
        );
        stmt->setString(1, mobile);
        stmt->setString(2, otp);
        
        // Convert time_t to Oracle TIMESTAMP
        std::ostringstream oss;
        oss << "TIMESTAMP '" << std::put_time(std::gmtime(&time_t_expiry), "%Y-%m-%d %H:%M:%S") << "'";
        
        stmt = conn->createStatement(
            "INSERT INTO otps (mobile_no, otp_code, expiry_time, created_at) "
            "VALUES ('" + mobile + "', '" + otp + "', SYSTIMESTAMP + INTERVAL '1' MINUTE, SYSTIMESTAMP)"
        );
        
        stmt->executeUpdate();
        conn->terminateStatement(stmt);
        conn->commit();
        
        return true;
    } catch (SQLException& ex) {
        std::cerr << "Error storing OTP: " << ex.getMessage() << std::endl;
        return false;
    }
}

bool DatabaseManager::verifyOTP(const std::string& mobile, const std::string& otp) {
    try {
        Statement* stmt = conn->createStatement(
            "SELECT COUNT(*) FROM otps WHERE mobile_no = :1 AND otp_code = :2 AND expiry_time > SYSTIMESTAMP"
        );
        stmt->setString(1, mobile);
        stmt->setString(2, otp);
        
        ResultSet* rs = stmt->executeQuery();
        bool isValid = false;
        
        if (rs->next()) {
            int count = rs->getInt(1);
            isValid = (count > 0);
        }
        
        stmt->closeResultSet(rs);
        conn->terminateStatement(stmt);
        
        // If OTP is valid, delete it (one-time use)
        if (isValid) {
            stmt = conn->createStatement(
                "DELETE FROM otps WHERE mobile_no = :1 AND otp_code = :2"
            );
            stmt->setString(1, mobile);
            stmt->setString(2, otp);
            stmt->executeUpdate();
            conn->terminateStatement(stmt);
            conn->commit();
        }
        
        return isValid;
    } catch (SQLException& ex) {
        std::cerr << "Error verifying OTP: " << ex.getMessage() << std::endl;
        return false;
    }
}

bool DatabaseManager::cleanExpiredOTPs() {
    try {
        Statement* stmt = conn->createStatement(
            "DELETE FROM otps WHERE expiry_time <= SYSTIMESTAMP"
        );
        stmt->executeUpdate();
        conn->terminateStatement(stmt);
        conn->commit();
        return true;
    } catch (SQLException& ex) {
        std::cerr << "Error cleaning expired OTPs: " << ex.getMessage() << std::endl;
        return false;
    }
}

bool DatabaseManager::createUser(const std::string& mobile, const std::string& name) {
    try {
        Statement* stmt = conn->createStatement(
            "INSERT INTO users (mobile_no, name, created_at) VALUES (:1, :2, SYSTIMESTAMP)"
        );
        stmt->setString(1, mobile);
        stmt->setString(2, name);
        stmt->executeUpdate();
        conn->terminateStatement(stmt);
        conn->commit();
        return true;
    } catch (SQLException& ex) {
        std::cerr << "Error creating user: " << ex.getMessage() << std::endl;
        return false;
    }
}

bool DatabaseManager::userExists(const std::string& mobile) {
    try {
        Statement* stmt = conn->createStatement(
            "SELECT COUNT(*) FROM users WHERE mobile_no = :1"
        );
        stmt->setString(1, mobile);
        
        ResultSet* rs = stmt->executeQuery();
        bool exists = false;
        
        if (rs->next()) {
            int count = rs->getInt(1);
            exists = (count > 0);
        }
        
        stmt->closeResultSet(rs);
        conn->terminateStatement(stmt);
        
        return exists;
    } catch (SQLException& ex) {
        std::cerr << "Error checking user existence: " << ex.getMessage() << std::endl;
        return false;
    }
}

int DatabaseManager::getUserId(const std::string& mobile) {
    try {
        Statement* stmt = conn->createStatement(
            "SELECT user_id FROM users WHERE mobile_no = :1"
        );
        stmt->setString(1, mobile);
        
        ResultSet* rs = stmt->executeQuery();
        int userId = -1;
        
        if (rs->next()) {
            userId = rs->getInt(1);
        }
        
        stmt->closeResultSet(rs);
        conn->terminateStatement(stmt);
        
        return userId;
    } catch (SQLException& ex) {
        std::cerr << "Error getting user ID: " << ex.getMessage() << std::endl;
        return -1;
    }
}

bool DatabaseManager::saveScore(int userId, int score, const std::string& gameName) {
    try {
        Statement* stmt = conn->createStatement(
            "INSERT INTO scores (user_id, score, game_name, created_at) VALUES (:1, :2, :3, SYSTIMESTAMP)"
        );
        stmt->setInt(1, userId);
        stmt->setInt(2, score);
        stmt->setString(3, gameName);
        stmt->executeUpdate();
        conn->terminateStatement(stmt);
        conn->commit();
        return true;
    } catch (SQLException& ex) {
        std::cerr << "Error saving score: " << ex.getMessage() << std::endl;
        return false;
    }
}
#include <iostream>
#include <memory>
#include <signal.h>
#include "database.h"
#include "api_server.h"

// Global server instance for signal handling
APIServer* g_server = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "Starting Gaming API Server..." << std::endl;
    
    // Database configuration
    std::string connectionString = "localhost:1521/XE";  // Adjust as needed
    std::string username = "gaming_user";               // Adjust as needed
    std::string password = "gaming_password";           // Adjust as needed
    
    // Create database manager
    auto dbManager = std::make_unique<DatabaseManager>(connectionString, username, password);
    
    // Try to connect to database
    if (!dbManager->connect()) {
        std::cerr << "Failed to connect to database. Please check your Oracle configuration." << std::endl;
        std::cerr << "Make sure Oracle is running and the connection details are correct." << std::endl;
        return 1;
    }
    
    // Clean expired OTPs on startup
    dbManager->cleanExpiredOTPs();
    
    // Create and start API server
    int port = 8080;
    APIServer server(port, std::move(dbManager));
    g_server = &server;
    
    if (!server.start()) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }
    
    std::cout << "Gaming API Server is running on http://localhost:" << port << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  POST /api/send-otp" << std::endl;
    std::cout << "  POST /api/verify-otp" << std::endl;
    std::cout << "  POST /api/register" << std::endl;
    std::cout << "  POST /api/save-score" << std::endl;
    std::cout << "  GET  /api/dashboard/overall" << std::endl;
    std::cout << "  GET  /api/dashboard/weekly" << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server." << std::endl;
    
    // Run the server
    server.run();
    
    return 0;
}
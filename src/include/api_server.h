#ifndef API_SERVER_H
#define API_SERVER_H

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <thread>
#include <atomic>
#include "database.h"

struct HttpRequest {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> params;
};

struct HttpResponse {
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse() : status_code(200) {
        headers["Content-Type"] = "application/json";
        headers["Access-Control-Allow-Origin"] = "*";
        headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    }
};

class APIServer {
private:
    std::unique_ptr<DatabaseManager> dbManager;
    std::atomic<bool> running;
    int port;
    int server_socket;
    
    // Route handlers
    std::map<std::string, std::function<HttpResponse(const HttpRequest&)>> routes;
    
    // Helper methods
    HttpRequest parseRequest(const std::string& request);
    std::string buildResponse(const HttpResponse& response);
    std::map<std::string, std::string> parseJsonBody(const std::string& body);
    std::string createJsonResponse(const std::map<std::string, std::string>& data);
    
    // Route handler methods
    HttpResponse sendOTP(const HttpRequest& request);
    HttpResponse verifyOTP(const HttpRequest& request);
    HttpResponse registerUser(const HttpRequest& request);
    HttpResponse saveScore(const HttpRequest& request);
    HttpResponse getOverallDashboard(const HttpRequest& request);
    HttpResponse getWeeklyDashboard(const HttpRequest& request);
    HttpResponse handleOptions(const HttpRequest& request);
    
    void setupRoutes();
    void handleClient(int client_socket);

public:
    APIServer(int port, std::unique_ptr<DatabaseManager> db);
    ~APIServer();
    
    bool start();
    void stop();
    void run();
};

#endif // API_SERVER_H
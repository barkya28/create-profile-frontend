#include "api_server.h"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <regex>
#include <chrono>
#include <iomanip>

APIServer::APIServer(int port, std::unique_ptr<DatabaseManager> db) 
    : dbManager(std::move(db)), running(false), port(port), server_socket(-1) {
    setupRoutes();
}

APIServer::~APIServer() {
    stop();
}

void APIServer::setupRoutes() {
    routes["POST /api/send-otp"] = [this](const HttpRequest& req) { return sendOTP(req); };
    routes["POST /api/verify-otp"] = [this](const HttpRequest& req) { return verifyOTP(req); };
    routes["POST /api/register"] = [this](const HttpRequest& req) { return registerUser(req); };
    routes["POST /api/save-score"] = [this](const HttpRequest& req) { return saveScore(req); };
    routes["GET /api/dashboard/overall"] = [this](const HttpRequest& req) { return getOverallDashboard(req); };
    routes["GET /api/dashboard/weekly"] = [this](const HttpRequest& req) { return getWeeklyDashboard(req); };
    routes["OPTIONS"] = [this](const HttpRequest& req) { return handleOptions(req); };
}

HttpRequest APIServer::parseRequest(const std::string& request) {
    HttpRequest req;
    std::istringstream stream(request);
    std::string line;
    
    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        lineStream >> req.method >> req.path;
        
        // Extract query parameters
        size_t queryPos = req.path.find('?');
        if (queryPos != std::string::npos) {
            std::string queryString = req.path.substr(queryPos + 1);
            req.path = req.path.substr(0, queryPos);
            
            // Parse query parameters
            std::regex paramRegex("([^&=]+)=([^&=]*)");
            std::sregex_iterator iter(queryString.begin(), queryString.end(), paramRegex);
            std::sregex_iterator end;
            
            for (; iter != end; ++iter) {
                req.params[(*iter)[1]] = (*iter)[2];
            }
        }
    }
    
    // Parse headers
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            // Trim whitespace
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            req.headers[key] = value;
        }
    }
    
    // Parse body
    std::string bodyLine;
    while (std::getline(stream, bodyLine)) {
        req.body += bodyLine + "\n";
    }
    
    return req;
}

std::string APIServer::buildResponse(const HttpResponse& response) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.status_code;
    
    switch (response.status_code) {
        case 200: oss << " OK"; break;
        case 400: oss << " Bad Request"; break;
        case 404: oss << " Not Found"; break;
        case 500: oss << " Internal Server Error"; break;
        default: oss << " Unknown"; break;
    }
    
    oss << "\r\n";
    
    for (const auto& header : response.headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "Content-Length: " << response.body.length() << "\r\n";
    oss << "\r\n";
    oss << response.body;
    
    return oss.str();
}

std::map<std::string, std::string> APIServer::parseJsonBody(const std::string& body) {
    std::map<std::string, std::string> result;
    
    // Simple JSON parser for key-value pairs
    std::regex jsonRegex("\"([^\"]+)\"\\s*:\\s*\"([^\"]+)\"");
    std::sregex_iterator iter(body.begin(), body.end(), jsonRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        result[(*iter)[1]] = (*iter)[2];
    }
    
    return result;
}

std::string APIServer::createJsonResponse(const std::map<std::string, std::string>& data) {
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& pair : data) {
        if (!first) oss << ",";
        oss << "\"" << pair.first << "\":\"" << pair.second << "\"";
        first = false;
    }
    
    oss << "}";
    return oss.str();
}

HttpResponse APIServer::sendOTP(const HttpRequest& request) {
    HttpResponse response;
    
    try {
        auto params = parseJsonBody(request.body);
        std::string mobile = params["mobile"];
        
        if (mobile.empty()) {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Mobile number is required"}});
            return response;
        }
        
        // Validate mobile number (basic validation)
        if (mobile.length() != 10 || !std::all_of(mobile.begin(), mobile.end(), ::isdigit)) {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Invalid mobile number format"}});
            return response;
        }
        
        // Hardcoded OTP as requested
        std::string otp = "1234";
        
        // Calculate expiry time (1 minute from now)
        auto expiry = std::chrono::system_clock::now() + std::chrono::minutes(1);
        
        // Store OTP in database
        if (dbManager->storeOTP(mobile, otp, expiry)) {
            response.body = createJsonResponse({{"status", "success"}, {"message", "OTP sent successfully"}});
            std::cout << "OTP sent to mobile: " << mobile << " (OTP: " << otp << ")" << std::endl;
        } else {
            response.status_code = 500;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Failed to send OTP"}});
        }
        
    } catch (const std::exception& e) {
        response.status_code = 500;
        response.body = createJsonResponse({{"status", "error"}, {"message", "Internal server error"}});
        std::cerr << "Error in sendOTP: " << e.what() << std::endl;
    }
    
    return response;
}

HttpResponse APIServer::verifyOTP(const HttpRequest& request) {
    HttpResponse response;
    
    try {
        auto params = parseJsonBody(request.body);
        std::string mobile = params["mobile"];
        std::string otp = params["otp"];
        
        if (mobile.empty() || otp.empty()) {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Mobile number and OTP are required"}});
            return response;
        }
        
        if (dbManager->verifyOTP(mobile, otp)) {
            response.body = createJsonResponse({{"status", "success"}, {"message", "OTP verified successfully"}});
        } else {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Invalid or expired OTP"}});
        }
        
    } catch (const std::exception& e) {
        response.status_code = 500;
        response.body = createJsonResponse({{"status", "error"}, {"message", "Internal server error"}});
        std::cerr << "Error in verifyOTP: " << e.what() << std::endl;
    }
    
    return response;
}

HttpResponse APIServer::registerUser(const HttpRequest& request) {
    HttpResponse response;
    
    try {
        auto params = parseJsonBody(request.body);
        std::string mobile = params["mobile"];
        std::string name = params["name"];
        std::string otp = params["otp"];
        
        if (mobile.empty() || name.empty() || otp.empty()) {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Mobile, name, and OTP are required"}});
            return response;
        }
        
        // Verify OTP first
        if (!dbManager->verifyOTP(mobile, otp)) {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Invalid or expired OTP"}});
            return response;
        }
        
        // Check if user already exists
        if (dbManager->userExists(mobile)) {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "User already exists"}});
            return response;
        }
        
        // Create user
        if (dbManager->createUser(mobile, name)) {
            response.body = createJsonResponse({{"status", "success"}, {"message", "User registered successfully"}});
        } else {
            response.status_code = 500;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Failed to register user"}});
        }
        
    } catch (const std::exception& e) {
        response.status_code = 500;
        response.body = createJsonResponse({{"status", "error"}, {"message", "Internal server error"}});
        std::cerr << "Error in registerUser: " << e.what() << std::endl;
    }
    
    return response;
}

HttpResponse APIServer::saveScore(const HttpRequest& request) {
    HttpResponse response;
    
    try {
        auto params = parseJsonBody(request.body);
        std::string mobile = params["mobile"];
        std::string scoreStr = params["score"];
        std::string gameName = params["game_name"];
        
        if (mobile.empty() || scoreStr.empty() || gameName.empty()) {
            response.status_code = 400;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Mobile, score, and game name are required"}});
            return response;
        }
        
        int score = std::stoi(scoreStr);
        int userId = dbManager->getUserId(mobile);
        
        if (userId == -1) {
            response.status_code = 404;
            response.body = createJsonResponse({{"status", "error"}, {"message", "User not found"}});
            return response;
        }
        
        if (dbManager->saveScore(userId, score, gameName)) {
            response.body = createJsonResponse({{"status", "success"}, {"message", "Score saved successfully"}});
        } else {
            response.status_code = 500;
            response.body = createJsonResponse({{"status", "error"}, {"message", "Failed to save score"}});
        }
        
    } catch (const std::exception& e) {
        response.status_code = 500;
        response.body = createJsonResponse({{"status", "error"}, {"message", "Internal server error"}});
        std::cerr << "Error in saveScore: " << e.what() << std::endl;
    }
    
    return response;
}

HttpResponse APIServer::getOverallDashboard(const HttpRequest& request) {
    HttpResponse response;
    
    try {
        // This would typically fetch overall statistics from the database
        // For now, returning a placeholder response
        std::map<std::string, std::string> dashboardData = {
            {"status", "success"},
            {"total_users", "0"},
            {"total_games", "0"},
            {"highest_score", "0"},
            {"message", "Overall dashboard data"}
        };
        
        response.body = createJsonResponse(dashboardData);
        
    } catch (const std::exception& e) {
        response.status_code = 500;
        response.body = createJsonResponse({{"status", "error"}, {"message", "Internal server error"}});
        std::cerr << "Error in getOverallDashboard: " << e.what() << std::endl;
    }
    
    return response;
}

HttpResponse APIServer::getWeeklyDashboard(const HttpRequest& request) {
    HttpResponse response;
    
    try {
        // This would typically fetch weekly statistics from the database
        // For now, returning a placeholder response
        std::map<std::string, std::string> dashboardData = {
            {"status", "success"},
            {"weekly_users", "0"},
            {"weekly_games", "0"},
            {"weekly_highest_score", "0"},
            {"message", "Weekly dashboard data"}
        };
        
        response.body = createJsonResponse(dashboardData);
        
    } catch (const std::exception& e) {
        response.status_code = 500;
        response.body = createJsonResponse({{"status", "error"}, {"message", "Internal server error"}});
        std::cerr << "Error in getWeeklyDashboard: " << e.what() << std::endl;
    }
    
    return response;
}

HttpResponse APIServer::handleOptions(const HttpRequest& request) {
    HttpResponse response;
    response.status_code = 200;
    response.body = "";
    return response;
}

bool APIServer::start() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        close(server_socket);
        return false;
    }
    
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket to port " << port << std::endl;
        close(server_socket);
        return false;
    }
    
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_socket);
        return false;
    }
    
    running = true;
    std::cout << "Server started on port " << port << std::endl;
    return true;
}

void APIServer::stop() {
    running = false;
    if (server_socket != -1) {
        close(server_socket);
        server_socket = -1;
    }
}

void APIServer::run() {
    while (running) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running) {
                std::cerr << "Failed to accept client connection" << std::endl;
            }
            continue;
        }
        
        // Handle client in a separate thread
        std::thread clientThread(&APIServer::handleClient, this, client_socket);
        clientThread.detach();
    }
}

void APIServer::handleClient(int client_socket) {
    char buffer[4096];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        close(client_socket);
        return;
    }
    
    buffer[bytes_received] = '\0';
    std::string request_str(buffer);
    
    HttpRequest request = parseRequest(request_str);
    HttpResponse response;
    
    // Find matching route
    std::string route_key = request.method + " " + request.path;
    auto route_it = routes.find(route_key);
    
    if (route_it != routes.end()) {
        response = route_it->second(request);
    } else if (request.method == "OPTIONS") {
        response = handleOptions(request);
    } else {
        response.status_code = 404;
        response.body = createJsonResponse({{"status", "error"}, {"message", "Endpoint not found"}});
    }
    
    std::string response_str = buildResponse(response);
    send(client_socket, response_str.c_str(), response_str.length(), 0);
    
    close(client_socket);
}
#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "Blockchain.hpp"
#include <string>
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>

/**
 * ============================================================================
 * OOP CONCEPT: FAÇADE & SUBSYSTEM ORCHESTRATION
 * ============================================================================
 * HttpServer embeds a lightweight, high-performance POSIX HTTP/1.1 server
 * directly inside the C++ application. It routes REST API requests to the
 * Blockchain engine and serves the static HTML/CSS/JS frontend to any browser.
 */
class HttpServer {
public:
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string body;
        std::unordered_map<std::string, std::string> headers;
        std::unordered_map<std::string, std::string> queryParams;
    };

    struct HttpResponse {
        int statusCode = 200;
        std::string contentType = "text/plain";
        std::string body;
    };

    using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

private:
    int port;
    std::string webRoot;
    int serverFd;
    std::atomic<bool> isRunning;
    std::shared_ptr<Blockchain> blockchain;
    std::unordered_map<std::string, RouteHandler> getRoutes;
    std::unordered_map<std::string, RouteHandler> postRoutes;

    void setupRoutes();
    void handleClient(int clientSocket);
    HttpRequest parseRequest(const std::string& rawRequest);
    std::string buildResponseString(const HttpResponse& res);
    HttpResponse serveStaticFile(const std::string& path);
    std::string getMimeType(const std::string& path);

public:
    HttpServer(int port, const std::string& webRoot, std::shared_ptr<Blockchain> blockchain);
    ~HttpServer();

    void start();
    void stop();
};

#endif // HTTPSERVER_HPP

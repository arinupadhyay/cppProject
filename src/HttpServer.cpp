#include "../include/HttpServer.hpp"
#include "../include/SHA256.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

static std::string extractJsonValue(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";

    pos += needle.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

    if (pos < json.length() && json[pos] == '"') {
        pos++;
        size_t end = json.find('"', pos);
        if (end != std::string::npos) {
            return json.substr(pos, end - pos);
        }
    } else {
        size_t end = json.find_first_of(",}\n\r", pos);
        if (end != std::string::npos) {
            std::string val = json.substr(pos, end - pos);
            while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();
            return val;
        }
    }
    return "";
}

HttpServer::HttpServer(int port, const std::string& webRoot, std::shared_ptr<Blockchain> blockchain)
    : port(port), webRoot(webRoot), serverFd(-1), isRunning(false), blockchain(blockchain) {
    setupRoutes();
}

HttpServer::~HttpServer() {
    stop();
}

std::string HttpServer::getMimeType(const std::string& path) {
    if (path.rfind(".html") != std::string::npos) return "text/html; charset=utf-8";
    if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    if (path.rfind(".json") != std::string::npos) return "application/json";
    if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
    if (path.rfind(".png") != std::string::npos) return "image/png";
    return "text/plain";
}

HttpResponse HttpServer::serveStaticFile(const std::string& relPath) {
    std::string filePath = webRoot + (relPath == "/" ? "/index.html" : relPath);
    std::ifstream file(filePath, std::ios::binary);

    HttpResponse res;
    if (!file.is_open()) {
        res.statusCode = 404;
        res.contentType = "text/plain";
        res.body = "404 Not Found: " + relPath;
        return res;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    res.statusCode = 200;
    res.contentType = getMimeType(filePath);
    res.body = ss.str();
    return res;
}

void HttpServer::setupRoutes() {
    // 1. GET /api/blockchain
    getRoutes["/api/blockchain"] = [this](const HttpRequest&) {
        HttpResponse res;
        res.contentType = "application/json";
        res.body = blockchain->toJSON();
        return res;
    };

    // 2. GET /api/mempool
    getRoutes["/api/mempool"] = [this](const HttpRequest&) {
        HttpResponse res;
        res.contentType = "application/json";
        res.body = blockchain->mempoolToJSON();
        return res;
    };

    // 3. GET /api/results
    getRoutes["/api/results"] = [this](const HttpRequest& req) {
        std::string electionId = "ELECTION-2026";
        auto it = req.queryParams.find("electionId");
        if (it != req.queryParams.end() && !it->second.empty()) {
            electionId = it->second;
        }
        HttpResponse res;
        res.contentType = "application/json";
        res.body = blockchain->resultsToJSON(electionId);
        return res;
    };

    // 4. GET /api/verify
    getRoutes["/api/verify"] = [this](const HttpRequest&) {
        std::string audit;
        bool valid = blockchain->isChainValid(audit);
        HttpResponse res;
        res.contentType = "application/json";
        std::stringstream ss;
        ss << "{\"valid\":" << (valid ? "true" : "false")
           << ",\"report\":\"" << audit << "\""
           << ",\"blocks\":" << blockchain->getChainLength()
           << "}";
        res.body = ss.str();
        return res;
    };

    // 5. POST /api/voter/register
    postRoutes["/api/voter/register"] = [this](const HttpRequest& req) {
        HttpResponse res;
        res.contentType = "application/json";

        std::string voterId = extractJsonValue(req.body, "voterId");
        std::string name = extractJsonValue(req.body, "name");
        std::string email = extractJsonValue(req.body, "email");

        if (voterId.empty()) {
            voterId = "VTR-" + SHA256::hash(name + email + std::to_string(rand())).substr(0, 8);
        }
        if (name.empty()) name = "Citizen Voter";
        if (email.empty()) email = "voter@election.org";

        // Cryptographic key derivation
        std::string simulatedPrivKey = SHA256::hash("PRIV_" + voterId + "_" + email);
        std::string pubKey = "PUB_" + SHA256::hash(simulatedPrivKey).substr(0, 32);
        std::string address = "0x" + SHA256::hash(pubKey).substr(0, 40);

        try {
            Voter voter(voterId, name, email, address, pubKey, simulatedPrivKey);
            blockchain->registerVoter(voter);

            std::stringstream ss;
            ss << "{"
               << "\"success\":true,"
               << "\"voterId\":\"" << voterId << "\","
               << "\"name\":\"" << name << "\","
               << "\"address\":\"" << address << "\","
               << "\"publicKey\":\"" << pubKey << "\","
               << "\"privateKey\":\"" << simulatedPrivKey << "\""
               << "}";
            res.body = ss.str();
        } catch (const std::exception& e) {
            res.statusCode = 400;
            res.body = std::string("{\"success\":false,\"error\":\"") + e.what() + "\"}";
        }
        return res;
    };

    // 6. POST /api/vote
    postRoutes["/api/vote"] = [this](const HttpRequest& req) {
        HttpResponse res;
        res.contentType = "application/json";

        std::string electionId = extractJsonValue(req.body, "electionId");
        std::string candidateId = extractJsonValue(req.body, "candidateId");
        std::string candidateName = extractJsonValue(req.body, "candidateName");
        std::string voterAddress = extractJsonValue(req.body, "voterAddress");
        std::string voterPublicKey = extractJsonValue(req.body, "voterPublicKey");
        std::string privateKey = extractJsonValue(req.body, "privateKey");

        if (electionId.empty()) electionId = "ELECTION-2026";
        if (candidateId.empty() || candidateName.empty() || voterAddress.empty()) {
            res.statusCode = 400;
            res.body = "{\"success\":false,\"error\":\"Missing required ballot fields\"}";
            return res;
        }

        // Generate cryptographic signature using voter's private key
        std::string payloadToSign = electionId + ":" + candidateId + ":" + candidateName + ":" + voterAddress;
        std::string keyToUse = privateKey.empty() ? (voterAddress + "_SEC") : privateKey;
        std::string signature = SHA256::hash(keyToUse + "::" + payloadToSign);

        VoteTransaction tx(
            electionId,
            candidateId,
            candidateName,
            voterAddress,
            voterPublicKey.empty() ? ("PUB_" + voterAddress) : voterPublicKey,
            signature
        );

        try {
            blockchain->addVote(tx);
            std::stringstream ss;
            ss << "{"
               << "\"success\":true,"
               << "\"message\":\"Ballot accepted into mempool!\","
               << "\"txHash\":\"" << tx.getTxHash() << "\","
               << "\"signature\":\"" << signature << "\","
               << "\"status\":\"PENDING_IN_MEMPOOL\""
               << "}";
            res.body = ss.str();
        } catch (const std::exception& e) {
            res.statusCode = 400;
            res.body = std::string("{\"success\":false,\"error\":\"") + e.what() + "\"}";
        }
        return res;
    };

    // 7. POST /api/mine
    postRoutes["/api/mine"] = [this](const HttpRequest& req) {
        HttpResponse res;
        res.contentType = "application/json";

        std::string miner = extractJsonValue(req.body, "miner");
        if (miner.empty()) miner = "Validator Node 01";

        try {
            auto block = blockchain->minePendingVotes(miner);
            std::stringstream ss;
            ss << "{"
               << "\"success\":true,"
               << "\"message\":\"Block successfully mined with Proof of Work!\","
               << "\"blockIndex\":" << block->getIndex() << ","
               << "\"blockHash\":\"" << block->getHash() << "\","
               << "\"merkleRoot\":\"" << block->getMerkleRoot() << "\","
               << "\"nonce\":" << block->getNonce() << ","
               << "\"difficulty\":" << block->getDifficulty() << ","
               << "\"txCount\":" << block->getTransactionCount()
               << "}";
            res.body = ss.str();
        } catch (const std::exception& e) {
            res.statusCode = 400;
            res.body = std::string("{\"success\":false,\"error\":\"") + e.what() + "\"}";
        }
        return res;
    };

    // 8. POST /api/tamper (Security Lab)
    postRoutes["/api/tamper"] = [this](const HttpRequest& req) {
        HttpResponse res;
        res.contentType = "application/json";

        std::string blockIdxStr = extractJsonValue(req.body, "blockIndex");
        std::string fakeName = extractJsonValue(req.body, "fakeCandidateName");
        if (fakeName.empty()) fakeName = "MALICIOUS_TAMPERED_CANDIDATE";

        size_t idx = blockIdxStr.empty() ? 1 : std::stoul(blockIdxStr);
        bool success = blockchain->tamperBlock(idx, fakeName);

        std::stringstream ss;
        ss << "{"
           << "\"success\":" << (success ? "true" : "false") << ","
           << "\"blockIndex\":" << idx << ","
           << "\"message\":\"Block #" << idx << " vote was maliciously tampered with! Run blockchain verification to observe detection.\""
           << "}";
        res.body = ss.str();
        return res;
    };
}

HttpRequest HttpServer::parseRequest(const std::string& raw) {
    HttpRequest req;
    std::istringstream stream(raw);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        lineStream >> req.method >> req.path;
    }

    // Extract query parameters
    size_t qPos = req.path.find('?');
    if (qPos != std::string::npos) {
        std::string queryStr = req.path.substr(qPos + 1);
        req.path = req.path.substr(0, qPos);

        std::istringstream queryStream(queryStr);
        std::string pair;
        while (std::getline(queryStream, pair, '&')) {
            size_t eqPos = pair.find('=');
            if (eqPos != std::string::npos) {
                req.queryParams[pair.substr(0, eqPos)] = pair.substr(eqPos + 1);
            }
        }
    }

    // Parse headers
    size_t contentLength = 0;
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string headerName = line.substr(0, colon);
            std::string headerVal = line.substr(colon + 1);
            if (headerName == "Content-Length" || headerName == "content-length") {
                contentLength = std::stoul(headerVal);
            }
            req.headers[headerName] = headerVal;
        }
    }

    // Parse body
    if (contentLength > 0) {
        size_t bodyStart = raw.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            req.body = raw.substr(bodyStart + 4, contentLength);
        }
    }

    return req;
}

std::string HttpServer::buildResponseString(const HttpResponse& res) {
    std::stringstream ss;
    ss << "HTTP/1.1 " << res.statusCode << " ";
    if (res.statusCode == 200) ss << "OK";
    else if (res.statusCode == 400) ss << "Bad Request";
    else if (res.statusCode == 404) ss << "Not Found";
    else ss << "Status";
    ss << "\r\n";

    ss << "Content-Type: " << res.contentType << "\r\n";
    ss << "Content-Length: " << res.body.length() << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    ss << "Access-Control-Allow-Headers: Content-Type\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << res.body;

    return ss.str();
}

void HttpServer::handleClient(int clientSocket) {
    char buffer[16384];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);

    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }

    HttpRequest req = parseRequest(std::string(buffer, bytesRead));
    HttpResponse res;

    // Handle CORS preflight
    if (req.method == "OPTIONS") {
        res.statusCode = 200;
        res.contentType = "text/plain";
        res.body = "OK";
    } else if (req.method == "GET") {
        auto it = getRoutes.find(req.path);
        if (it != getRoutes.end()) {
            res = it->second(req);
        } else {
            res = serveStaticFile(req.path);
        }
    } else if (req.method == "POST") {
        auto it = postRoutes.find(req.path);
        if (it != postRoutes.end()) {
            res = it->second(req);
        } else {
            res.statusCode = 404;
            res.contentType = "application/json";
            res.body = "{\"error\":\"Route not found\"}";
        }
    }

    std::string responseStr = buildResponseString(res);
    write(clientSocket, responseStr.c_str(), responseStr.length());
    close(clientSocket);
}

void HttpServer::start() {
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(serverFd);
        throw std::runtime_error("Failed to bind socket on port " + std::to_string(port));
    }

    if (listen(serverFd, 30) < 0) {
        close(serverFd);
        throw std::runtime_error("Failed to listen on socket");
    }

    isRunning = true;
    std::cout << "==========================================================" << std::endl;
    std::cout << " BLOCKCHAIN VOTING WEB APPLICATION STARTED" << std::endl;
    std::cout << " Native C++ OOP Core & POSIX Web Server Listening" << std::endl;
    std::cout << " Open your browser at: http://localhost:" << port << std::endl;
    std::cout << " Serving Web UI from:  " << webRoot << std::endl;
    std::cout << "==========================================================" << std::endl;

    while (isRunning) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSocket >= 0) {
            std::thread(&HttpServer::handleClient, this, clientSocket).detach();
        }
    }
}

void HttpServer::stop() {
    isRunning = false;
    if (serverFd >= 0) {
        close(serverFd);
        serverFd = -1;
    }
}

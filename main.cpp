/**
 * ============================================================================
 * BLOCKCHAIN VOTING SYSTEM - MAIN ENTRY POINT
 * ============================================================================
 * Demonstrates:
 * 1. Abstraction (Pure virtual interfaces IVerifiable, ICryptoHashable)
 * 2. Encapsulation (Private fields, invariant enforcement, const correctness)
 * 3. Inheritance (User -> Voter, User -> Candidate, AbstractBlock -> Block)
 * 4. Polymorphism (Runtime dynamic dispatch with smart pointers, template MerkleTree)
 * 5. Operator Overloading (==, !=, <<)
 * 6. Native POSIX Socket Web Server serving HTML/CSS/JS frontend + JSON API
 * ============================================================================
 */

#include "include/Blockchain.hpp"
#include "include/Voter.hpp"
#include "include/Candidate.hpp"
#include "include/HttpServer.hpp"
#include "include/SHA256.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

void printBanner() {
    std::cout << "\n";
    std::cout << "==================================================================\n";
    std::cout << "   _     _            _        _           _       \n";
    std::cout << "  | |   | |          | |      | |         (_)      \n";
    std::cout << "  | |__ | | ___   ___| | _____| |__   __ _ _ _ __  \n";
    std::cout << "  | '_ \\| |/ _ \\ / __| |/ / __| '_ \\ / _` | | '_ \\ \n";
    std::cout << "  | |_) | | (_) | (__|   < (__| | | | (_| | | | | |\n";
    std::cout << "  |_.__/|_|\\___/ \\___|_|\\_\\___|_| |_|\\__,_|_|_| |_|\n";
    std::cout << "             V O T I N G   S Y S T E M             \n";
    std::cout << "==================================================================\n";
    std::cout << " Built with C++17 OOP Architecture & Native POSIX Web Server\n";
    std::cout << " Encapsulation * Abstraction * Inheritance * Polymorphism * Templates\n";
    std::cout << "==================================================================\n\n";
}

void seedSampleData(std::shared_ptr<Blockchain> blockchain) {
    std::cout << "[INIT] Seeding initial election candidates and registered voters...\n";

    // 1. Create candidates (Demonstrating Candidate : User inheritance)
    Candidate cand1("CAND-01", "Alice Vance", "alice@tech.org", "Decentralized Alliance", "Zero-Knowledge Privacy for all", "ELECTION-2026", "ALICE");
    Candidate cand2("CAND-02", "Bob Thorne", "bob@cyber.io", "Cyber Security Front", "Quantum-resistant blockchain auditing", "ELECTION-2026", "BOB");
    Candidate cand3("CAND-03", "Clara Oswald", "clara@future.org", "Open Ledger Coalition", "Transparent public budgets on-chain", "ELECTION-2026", "CLARA");

    std::cout << "  -> Registered candidate: "; cand1.display(); std::cout << "\n";
    std::cout << "  -> Registered candidate: "; cand2.display(); std::cout << "\n";
    std::cout << "  -> Registered candidate: "; cand3.display(); std::cout << "\n";

    // 2. Create voters (Demonstrating Voter : User inheritance)
    Voter voter1("VTR-1001", "David Miller", "david@univ.edu", "0x7a892b1e4590c34512fd45ab1209e8745c1234aa", "PUB_KEY_1001", "PRIV_KEY_1001");
    Voter voter2("VTR-1002", "Elena Rostova", "elena@tech.io", "0x3f56bc78129034ade1289fe409a87123bc456789", "PUB_KEY_1002", "PRIV_KEY_1002");
    Voter voter3("VTR-1003", "Farhan Qureshi", "farhan@city.org", "0x98124ef5601278ba90123ef6712398ab76123450", "PUB_KEY_1003", "PRIV_KEY_1003");

    blockchain->registerVoter(voter1);
    blockchain->registerVoter(voter2);
    blockchain->registerVoter(voter3);

    // 3. Cast initial demo votes
    std::string sig1 = voter1.signPayload("ELECTION-2026:CAND-01:Alice Vance:" + voter1.getVoterAddress());
    VoteTransaction tx1("ELECTION-2026", "CAND-01", "Alice Vance", voter1.getVoterAddress(), voter1.getPublicKey(), sig1);
    blockchain->addVote(tx1);

    std::string sig2 = voter2.signPayload("ELECTION-2026:CAND-02:Bob Thorne:" + voter2.getVoterAddress());
    VoteTransaction tx2("ELECTION-2026", "CAND-02", "Bob Thorne", voter2.getVoterAddress(), voter2.getPublicKey(), sig2);
    blockchain->addVote(tx2);

    std::cout << "[MINING] Mining initial Block #1 with Proof-of-Work (Difficulty: " << blockchain->getDifficulty() << ")...\n";
    auto block1 = blockchain->minePendingVotes("Validator-Alpha");
    std::cout << "  -> Mined Block #1 Hash: " << block1->getHash() << " (Nonce: " << block1->getNonce() << ")\n";

    // Cast a vote for Block #2
    std::string sig3 = voter3.signPayload("ELECTION-2026:CAND-01:Alice Vance:" + voter3.getVoterAddress());
    VoteTransaction tx3("ELECTION-2026", "CAND-01", "Alice Vance", voter3.getVoterAddress(), voter3.getPublicKey(), sig3);
    blockchain->addVote(tx3);

    std::cout << "[MINING] Mining Block #2...\n";
    auto block2 = blockchain->minePendingVotes("Validator-Beta");
    std::cout << "  -> Mined Block #2 Hash: " << block2->getHash() << " (Nonce: " << block2->getNonce() << ")\n";

    std::string audit;
    bool isValid = blockchain->isChainValid(audit);
    std::cout << "[AUDIT] Initial Blockchain Status: " << (isValid ? "VALID (100% Verified)" : "COMPROMISED") << "\n";
    std::cout << "        " << audit << "\n\n";
}

int main(int argc, char* argv[]) {
    printBanner();

    int port = 8080;
    std::string webRoot = "./public";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--webroot" && i + 1 < argc) {
            webRoot = argv[++i];
        }
    }

    // Instantiate Blockchain engine
    auto blockchain = std::make_shared<Blockchain>(2);
    seedSampleData(blockchain);

    // Launch Native C++ POSIX Web Server
    HttpServer server(port, webRoot, blockchain);

    try {
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

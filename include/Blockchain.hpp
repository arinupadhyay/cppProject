#ifndef BLOCKCHAIN_HPP
#define BLOCKCHAIN_HPP

#include "Block.hpp"
#include "Voter.hpp"
#include "Candidate.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

/**
 * ============================================================================
 * OOP CONCEPT: AGGREGATION, SMART POINTERS & POLYMORPHIC CONTAINERS
 * ============================================================================
 * Blockchain orchestrates the entire decentralized voting ledger.
 * Demonstrates:
 * - Smart pointer memory management (std::shared_ptr<Block>)
 * - Encapsulation of ledger state (chain, mempool, voter registry, spent state)
 * - Custom Exception handling
 * - Operator Overloading (<<)
 */
class Blockchain {
private:
    std::vector<std::shared_ptr<Block>> chain;
    std::vector<VoteTransaction> mempool;
    std::unordered_map<std::string, Voter> voterRegistry;
    std::unordered_set<std::string> spentBallots; // "electionId:voterAddress"
    uint32_t difficulty;

    void createGenesisBlock();

public:
    explicit Blockchain(uint32_t difficulty = 2);

    // Getters (Encapsulation)
    uint32_t getDifficulty() const { return difficulty; }
    void setDifficulty(uint32_t diff) { difficulty = diff; }
    size_t getChainLength() const { return chain.size(); }
    const std::vector<std::shared_ptr<Block>>& getChain() const { return chain; }
    const std::vector<VoteTransaction>& getMempool() const { return mempool; }
    const std::unordered_map<std::string, Voter>& getVoters() const { return voterRegistry; }

    std::shared_ptr<Block> getLatestBlock() const;
    std::shared_ptr<Block> getBlock(size_t index) const;

    // Voter Registration & Authentication
    void registerVoter(const Voter& voter);
    bool isVoterRegistered(const std::string& voterId) const;
    const Voter* getVoter(const std::string& voterId) const;

    // Voting Operations
    void addVote(const VoteTransaction& vote);
    std::shared_ptr<Block> minePendingVotes(const std::string& miner = "Consensus Validator");

    // Cryptographic Chain Validation
    bool isChainValid(std::string& auditDetails) const;

    // Results Tallying directly from the audited blockchain
    struct TallyResult {
        std::string candidateId;
        std::string candidateName;
        uint32_t voteCount;
        double percentage;
    };
    std::vector<TallyResult> getResults(const std::string& electionId) const;

    // Educational Tamper Simulation
    bool tamperBlock(size_t blockIndex, const std::string& fakeCandidateName);

    // Receipt verification lookup
    struct ReceiptInfo {
        bool found;
        bool isPending;
        uint32_t blockIndex;
        std::string blockHash;
        VoteTransaction transaction;
        std::vector<MerkleTree<VoteTransaction>::AuditStep> merkleProof;
        bool isProofValid;
    };
    ReceiptInfo findReceipt(const std::string& txHash) const;

    // JSON Serialization
    std::string toJSON() const;
    std::string mempoolToJSON() const;
    std::string resultsToJSON(const std::string& electionId) const;

    friend std::ostream& operator<<(std::ostream& os, const Blockchain& bc);
};

#endif // BLOCKCHAIN_HPP

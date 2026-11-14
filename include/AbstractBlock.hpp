#ifndef ABSTRACTBLOCK_HPP
#define ABSTRACTBLOCK_HPP

#include "ICryptoHashable.hpp"
#include "IVerifiable.hpp"
#include <string>
#include <cstdint>
#include <iostream>

/**
 * ============================================================================
 * OOP CONCEPT: ABSTRACTION & BASE CLASS HIERARCHY
 * ============================================================================
 * AbstractBlock defines the fundamental blueprint of any block on the ledger.
 * Demonstrates:
 * - Pure virtual methods enforcing consensus algorithm implementation
 * - Encapsulation of core header fields
 * - Multiple interface inheritance (ICryptoHashable, IVerifiable)
 */
class AbstractBlock : public ICryptoHashable, public IVerifiable {
protected:
    uint32_t index;
    uint64_t timestamp;
    std::string previousHash;
    std::string merkleRoot;
    std::string hash;
    uint64_t nonce;
    uint32_t difficulty;
    std::string minedBy;

public:
    AbstractBlock(uint32_t index, uint64_t timestamp, const std::string& previousHash,
                  uint32_t difficulty = 2, const std::string& minedBy = "Validator-01")
        : index(index), timestamp(timestamp), previousHash(previousHash),
          hash(""), nonce(0), difficulty(difficulty), minedBy(minedBy) {}

    virtual ~AbstractBlock() = default;

    // Getters (Encapsulation)
    uint32_t getIndex() const { return index; }
    uint64_t getTimestamp() const { return timestamp; }
    std::string getPreviousHash() const { return previousHash; }
    std::string getMerkleRoot() const { return merkleRoot; }
    std::string getHash() const { return hash; }
    uint64_t getNonce() const { return nonce; }
    uint32_t getDifficulty() const { return difficulty; }
    std::string getMinedBy() const { return minedBy; }

    // Pure virtual method: Derived classes must define their consensus/mining strategy
    virtual void mine(uint32_t targetDifficulty) = 0;

    // Pure virtual method: Get transaction count
    virtual size_t getTransactionCount() const = 0;

    // Serialization
    virtual std::string toJSON() const = 0;

    virtual void display(std::ostream& os = std::cout) const {
        os << "Block #" << index << " [Hash: " << hash.substr(0, 10) << "... | Prev: "
           << previousHash.substr(0, 10) << "... | Nonce: " << nonce << "]";
    }
};

#endif // ABSTRACTBLOCK_HPP

#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "AbstractBlock.hpp"
#include "VoteTransaction.hpp"
#include "MerkleTree.hpp"
#include <vector>

/**
 * ============================================================================
 * OOP CONCEPT: CONCRETE DERIVED CLASS & OPERATOR OVERLOADING
 * ============================================================================
 * Concrete Block implementation holding actual VoteTransactions.
 * Demonstrates:
 * - Specialization of AbstractBlock
 * - Composition: Block contains a collection of VoteTransaction objects
 * - Integration with Templated MerkleTree
 * - Operator Overloading (==, !=, <<)
 */
class Block : public AbstractBlock {
private:
    std::vector<VoteTransaction> transactions;

public:
    Block(uint32_t index, uint64_t timestamp, const std::vector<VoteTransaction>& transactions,
          const std::string& previousHash, uint32_t difficulty = 2,
          const std::string& minedBy = "Consensus Validator");

    // Accessors
    const std::vector<VoteTransaction>& getTransactions() const { return transactions; }
    size_t getTransactionCount() const override { return transactions.size(); }

    // Merkle Root calculation
    std::string calculateMerkleRoot();

    // AbstractBlock & ICryptoHashable overrides
    std::string computeHash() const override;
    void mine(uint32_t targetDifficulty) override;

    // IVerifiable overrides
    bool verify() const override;
    std::string getVerificationDetails() const override;

    // Tamper simulation for educational security testing
    bool tamperVote(size_t txIndex, const std::string& fakeCandidateName);

    // Serialization
    std::string toJSON() const override;

    // Operator Overloading
    bool operator==(const Block& other) const;
    bool operator!=(const Block& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Block& block);
};

#endif // BLOCK_HPP

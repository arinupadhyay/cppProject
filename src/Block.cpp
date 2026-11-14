#include "../include/Block.hpp"
#include "../include/SHA256.hpp"
#include <sstream>

Block::Block(uint32_t index, uint64_t timestamp, const std::vector<VoteTransaction>& transactions,
             const std::string& previousHash, uint32_t difficulty, const std::string& minedBy)
    : AbstractBlock(index, timestamp, previousHash, difficulty, minedBy),
      transactions(transactions) {
    this->merkleRoot = calculateMerkleRoot();
    this->hash = computeHash();
}

std::string Block::calculateMerkleRoot() {
    std::vector<std::string> txHashes;
    for (const auto& tx : transactions) {
        txHashes.push_back(tx.getTxHash());
    }
    MerkleTree<VoteTransaction> tree(txHashes);
    return tree.getRootHash();
}

std::string Block::computeHash() const {
    std::stringstream ss;
    ss << index << previousHash << timestamp << merkleRoot << nonce << difficulty << minedBy;
    return SHA256::hash(ss.str());
}

void Block::mine(uint32_t targetDifficulty) {
    this->difficulty = targetDifficulty;
    this->merkleRoot = calculateMerkleRoot();

    std::string target(targetDifficulty, '0');
    this->nonce = 0;
    this->hash = computeHash();

    while (this->hash.substr(0, targetDifficulty) != target) {
        this->nonce++;
        this->hash = computeHash();
    }
}

bool Block::verify() const {
    // 1. Recalculate hash and verify match
    if (computeHash() != this->hash) {
        return false;
    }

    // 2. Check Proof-of-Work difficulty target
    std::string target(difficulty, '0');
    if (this->hash.substr(0, difficulty) != target) {
        return false;
    }

    // 3. Recalculate and verify Merkle Root
    std::vector<std::string> txHashes;
    for (const auto& tx : transactions) {
        txHashes.push_back(tx.getTxHash());
    }
    MerkleTree<VoteTransaction> tree(txHashes);
    if (tree.getRootHash() != this->merkleRoot) {
        return false;
    }

    // 4. Verify all transactions inside this block
    for (const auto& tx : transactions) {
        if (!tx.verify()) {
            return false;
        }
    }

    return true;
}

std::string Block::getVerificationDetails() const {
    if (verify()) {
        return "Block #" + std::to_string(index) + " is cryptographically VALID.";
    } else {
        return "Block #" + std::to_string(index) + " is INVALID or TAMPERED.";
    }
}

bool Block::tamperVote(size_t txIndex, const std::string& fakeCandidateName) {
    if (txIndex >= transactions.size()) {
        return false;
    }
    transactions[txIndex].tamperCandidate(fakeCandidateName);
    // Intentionally DO NOT recalculate hash or Merkle root to test detection
    return true;
}

std::string Block::toJSON() const {
    std::stringstream ss;
    ss << "{"
       << "\"index\":" << index << ","
       << "\"timestamp\":" << timestamp << ","
       << "\"previousHash\":\"" << previousHash << "\","
       << "\"merkleRoot\":\"" << merkleRoot << "\","
       << "\"hash\":\"" << hash << "\","
       << "\"nonce\":" << nonce << ","
       << "\"difficulty\":" << difficulty << ","
       << "\"minedBy\":\"" << minedBy << "\","
       << "\"transactions\":[";

    for (size_t i = 0; i < transactions.size(); ++i) {
        ss << transactions[i].toJSON();
        if (i + 1 < transactions.size()) {
            ss << ",";
        }
    }

    ss << "]}";
    return ss.str();
}

bool Block::operator==(const Block& other) const {
    return this->hash == other.hash;
}

bool Block::operator!=(const Block& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Block& block) {
    os << "Block #" << block.index << " [Hash: " << block.hash.substr(0, 10)
       << "... | Merkle: " << block.merkleRoot.substr(0, 8)
       << "... | Txs: " << block.transactions.size()
       << " | Nonce: " << block.nonce << "]";
    return os;
}

#include "../include/Blockchain.hpp"
#include "../include/SHA256.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>

Blockchain::Blockchain(uint32_t difficulty) : difficulty(difficulty) {
    createGenesisBlock();
}

void Blockchain::createGenesisBlock() {
    VoteTransaction genesisTx(
        "GENESIS_SYSTEM",
        "INIT",
        "Genesis Block Ballot",
        "0x0000000000000000000000000000000000000000",
        "GENESIS_SYSTEM_AUTHORITY",
        "GENESIS_SIG_VALID",
        1704067200000,
        "0000000000000000"
    );

    std::vector<VoteTransaction> txs = { genesisTx };
    auto genesisBlock = std::make_shared<Block>(
        0,
        1704067200000,
        txs,
        std::string(64, '0'),
        1,
        "Genesis Node"
    );
    // Mine with difficulty 1 for fast deterministic genesis
    genesisBlock->mine(1);
    chain.push_back(genesisBlock);
}

std::shared_ptr<Block> Blockchain::getLatestBlock() const {
    if (chain.empty()) {
        throw std::runtime_error("Blockchain is empty");
    }
    return chain.back();
}

std::shared_ptr<Block> Blockchain::getBlock(size_t index) const {
    if (index >= chain.size()) {
        return nullptr;
    }
    return chain[index];
}

void Blockchain::registerVoter(const Voter& voter) {
    if (voterRegistry.find(voter.getId()) != voterRegistry.end()) {
        throw std::runtime_error("Voter ID already registered: " + voter.getId());
    }
    // Check address collision
    for (const auto& pair : voterRegistry) {
        if (pair.second.getVoterAddress() == voter.getVoterAddress()) {
            throw std::runtime_error("Voter address already registered: " + voter.getVoterAddress());
        }
    }
    voterRegistry.insert({voter.getId(), voter});
}

bool Blockchain::isVoterRegistered(const std::string& voterId) const {
    return voterRegistry.find(voterId) != voterRegistry.end();
}

const Voter* Blockchain::getVoter(const std::string& voterId) const {
    auto it = voterRegistry.find(voterId);
    if (it != voterRegistry.end()) {
        return &(it->second);
    }
    return nullptr;
}

void Blockchain::addVote(const VoteTransaction& vote) {
    // 1. Cryptographic validation of transaction
    if (!vote.verify()) {
        throw std::runtime_error("Cryptographic signature or hash verification failed for vote");
    }

    // 2. Double-voting check against mined ledger
    std::string ballotKey = vote.getElectionId() + ":" + vote.getVoterAddress();
    if (spentBallots.find(ballotKey) != spentBallots.end()) {
        throw std::runtime_error("Double voting rejected: Voter address already cast a ballot for election " + vote.getElectionId());
    }

    // 3. Double-voting check against current mempool
    for (const auto& pending : mempool) {
        if (pending.getElectionId() == vote.getElectionId() &&
            pending.getVoterAddress() == vote.getVoterAddress()) {
            throw std::runtime_error("Double voting rejected: Ballot already waiting in mempool");
        }
    }

    mempool.push_back(vote);
}

std::shared_ptr<Block> Blockchain::minePendingVotes(const std::string& miner) {
    if (mempool.empty()) {
        throw std::runtime_error("Mempool is empty. No pending votes to mine into a block.");
    }

    auto prevBlock = getLatestBlock();
    uint32_t newIndex = prevBlock->getIndex() + 1;
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    auto newBlock = std::make_shared<Block>(
        newIndex,
        now,
        mempool,
        prevBlock->getHash(),
        difficulty,
        miner
    );

    // Proof-of-Work
    newBlock->mine(difficulty);

    // Update spent voter registry for all mined transactions
    for (const auto& tx : newBlock->getTransactions()) {
        if (tx.getVoterAddress() != "0x0000000000000000000000000000000000000000") {
            spentBallots.insert(tx.getElectionId() + ":" + tx.getVoterAddress());
        }
    }

    chain.push_back(newBlock);
    mempool.clear();

    return newBlock;
}

bool Blockchain::isChainValid(std::string& auditDetails) const {
    std::stringstream report;

    // Validate Genesis Block
    if (chain.empty()) {
        auditDetails = "ERROR: Blockchain contains zero blocks";
        return false;
    }

    auto genesis = chain[0];
    if (genesis->getIndex() != 0 || genesis->getPreviousHash() != std::string(64, '0')) {
        auditDetails = "ERROR: Genesis block header corrupted";
        return false;
    }
    if (!genesis->verify()) {
        auditDetails = "ERROR: Genesis block cryptographic validation failed";
        return false;
    }

    std::unordered_set<std::string> auditSpent;
    size_t totalVotesAudited = 0;

    for (size_t i = 1; i < chain.size(); ++i) {
        auto current = chain[i];
        auto previous = chain[i - 1];

        // 1. Verify previous hash chain linkage
        if (current->getPreviousHash() != previous->getHash()) {
            report << "TAMPER DETECTED at Block #" << current->getIndex()
                   << ": previousHash (" << current->getPreviousHash().substr(0, 10)
                   << "...) does not match Block #" << previous->getIndex()
                   << " hash (" << previous->getHash().substr(0, 10) << "...)";
            auditDetails = report.str();
            return false;
        }

        // 2. Verify current block internal cryptography (hash, merkle root, PoW, transactions)
        if (!current->verify()) {
            report << "TAMPER DETECTED at Block #" << current->getIndex()
                   << ": Internal cryptographic verification failed (Merkle Root or Hash altered)";
            auditDetails = report.str();
            return false;
        }

        // 3. Audit double voting across chain history
        for (const auto& tx : current->getTransactions()) {
            if (tx.getVoterAddress() != "0x0000000000000000000000000000000000000000") {
                std::string key = tx.getElectionId() + ":" + tx.getVoterAddress();
                if (auditSpent.find(key) != auditSpent.end()) {
                    report << "DOUBLE VOTING DETECTED at Block #" << current->getIndex()
                           << " for voter " << tx.getVoterAddress()
                           << " in election " << tx.getElectionId();
                    auditDetails = report.str();
                    return false;
                }
                auditSpent.insert(key);
                totalVotesAudited++;
            }
        }
    }

    report << "SUCCESS: All " << chain.size() << " blocks and "
           << totalVotesAudited << " cast votes verified. Cryptographic integrity 100% intact.";
    auditDetails = report.str();
    return true;
}

std::vector<Blockchain::TallyResult> Blockchain::getResults(const std::string& electionId) const {
    std::unordered_map<std::string, TallyResult> tallyMap;
    uint32_t totalVotes = 0;

    for (size_t i = 1; i < chain.size(); ++i) {
        for (const auto& tx : chain[i]->getTransactions()) {
            if (tx.getElectionId() == electionId) {
                totalVotes++;
                if (tallyMap.find(tx.getCandidateId()) == tallyMap.end()) {
                    TallyResult tr;
                    tr.candidateId = tx.getCandidateId();
                    tr.candidateName = tx.getCandidateName();
                    tr.voteCount = 0;
                    tr.percentage = 0.0;
                    tallyMap[tx.getCandidateId()] = tr;
                }
                tallyMap[tx.getCandidateId()].voteCount++;
            }
        }
    }

    std::vector<TallyResult> results;
    for (auto& pair : tallyMap) {
        if (totalVotes > 0) {
            pair.second.percentage = (static_cast<double>(pair.second.voteCount) / totalVotes) * 100.0;
        }
        results.push_back(pair.second);
    }

    std::sort(results.begin(), results.end(), [](const TallyResult& a, const TallyResult& b) {
        return a.voteCount > b.voteCount;
    });

    return results;
}

bool Blockchain::tamperBlock(size_t blockIndex, const std::string& fakeCandidateName) {
    if (blockIndex <= 0 || blockIndex >= chain.size()) {
        return false;
    }
    return chain[blockIndex]->tamperVote(0, fakeCandidateName);
}

Blockchain::ReceiptInfo Blockchain::findReceipt(const std::string& txHash) const {
    ReceiptInfo info;
    info.found = false;
    info.isPending = false;
    info.isProofValid = false;

    // Search mined blocks
    for (const auto& block : chain) {
        const auto& txs = block->getTransactions();
        for (size_t i = 0; i < txs.size(); ++i) {
            if (txs[i].getTxHash() == txHash) {
                info.found = true;
                info.isPending = false;
                info.blockIndex = block->getIndex();
                info.blockHash = block->getHash();
                info.transaction = txs[i];

                std::vector<std::string> hashes;
                for (const auto& t : txs) hashes.push_back(t.getTxHash());
                MerkleTree<VoteTransaction> tree(hashes);
                info.merkleProof = tree.generateProof(i);
                info.isProofValid = MerkleTree<VoteTransaction>::verifyProof(
                    txHash, info.merkleProof, block->getMerkleRoot()
                );
                return info;
            }
        }
    }

    // Search mempool
    for (const auto& tx : mempool) {
        if (tx.getTxHash() == txHash) {
            info.found = true;
            info.isPending = true;
            info.blockIndex = 0;
            info.blockHash = "UNMINED_IN_MEMPOOL";
            info.transaction = tx;
            return info;
        }
    }

    return info;
}

std::string Blockchain::toJSON() const {
    std::stringstream ss;
    ss << "{\"chain\":[";
    for (size_t i = 0; i < chain.size(); ++i) {
        ss << chain[i]->toJSON();
        if (i + 1 < chain.size()) ss << ",";
    }
    ss << "],\"difficulty\":" << difficulty
       << ",\"mempoolSize\":" << mempool.size()
       << ",\"totalBlocks\":" << chain.size() << "}";
    return ss.str();
}

std::string Blockchain::mempoolToJSON() const {
    std::stringstream ss;
    ss << "{\"pendingVotes\":[";
    for (size_t i = 0; i < mempool.size(); ++i) {
        ss << mempool[i].toJSON();
        if (i + 1 < mempool.size()) ss << ",";
    }
    ss << "],\"count\":" << mempool.size() << "}";
    return ss.str();
}

std::string Blockchain::resultsToJSON(const std::string& electionId) const {
    auto results = getResults(electionId);
    std::stringstream ss;
    ss << "{\"electionId\":\"" << electionId << "\",\"results\":[";
    for (size_t i = 0; i < results.size(); ++i) {
        ss << "{"
           << "\"candidateId\":\"" << results[i].candidateId << "\","
           << "\"candidateName\":\"" << results[i].candidateName << "\","
           << "\"voteCount\":" << results[i].voteCount << ","
           << "\"percentage\":" << std::fixed << std::setprecision(2) << results[i].percentage
           << "}";
        if (i + 1 < results.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Blockchain& bc) {
    os << "=== BLOCKCHAIN VOTING LEDGER ===" << std::endl;
    os << "Blocks: " << bc.chain.size() << " | Difficulty: " << bc.difficulty
       << " | Mempool Pending: " << bc.mempool.size() << std::endl;
    for (const auto& block : bc.chain) {
        os << "  " << *block << std::endl;
    }
    return os;
}

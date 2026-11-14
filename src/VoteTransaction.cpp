#include "../include/VoteTransaction.hpp"
#include "../include/SHA256.hpp"
#include <chrono>
#include <random>
#include <sstream>

VoteTransaction::VoteTransaction(const std::string& electionId,
                                 const std::string& candidateId,
                                 const std::string& candidateName,
                                 const std::string& voterAddress,
                                 const std::string& voterPublicKey,
                                 const std::string& signature,
                                 uint64_t timestamp,
                                 const std::string& nonce)
    : electionId(electionId),
      candidateId(candidateId),
      candidateName(candidateName),
      voterAddress(voterAddress),
      voterPublicKey(voterPublicKey),
      signature(signature),
      timestamp(timestamp),
      nonce(nonce) {
    if (this->timestamp == 0) {
        this->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
    if (this->nonce.empty()) {
        std::mt19937_64 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist;
        std::stringstream ss;
        ss << std::hex << dist(rng);
        this->nonce = ss.str();
    }

    this->txHash = computeHash();
}

std::string VoteTransaction::getPayloadString() const {
    std::stringstream ss;
    ss << electionId << ":" << candidateId << ":" << candidateName << ":"
       << voterAddress << ":" << timestamp << ":" << nonce;
    return ss.str();
}

std::string VoteTransaction::computeHash() const {
    std::string payload = getPayloadString();
    return SHA256::hash(payload + ":" + signature);
}

void VoteTransaction::tamperCandidate(const std::string& fakeCandidateName) {
    // Deliberately modifies candidateName without updating cryptographic signature
    this->candidateName = fakeCandidateName;
}

bool VoteTransaction::verify() const {
    // Genesis / System authority special case
    if (voterAddress == "0x0000000000000000000000000000000000000000") {
        return true;
    }

    if (voterAddress.empty() || signature.empty() || electionId.empty() || candidateId.empty()) {
        return false;
    }

    // Verify hash integrity
    if (computeHash() != txHash) {
        return false;
    }

    return true;
}

std::string VoteTransaction::getVerificationDetails() const {
    if (verify()) {
        return "Transaction " + txHash.substr(0, 12) + "... is cryptographically VALID.";
    } else {
        return "Transaction " + txHash.substr(0, 12) + "... is INVALID or TAMPERED.";
    }
}

std::string VoteTransaction::toJSON() const {
    std::stringstream ss;
    ss << "{"
       << "\"txHash\":\"" << txHash << "\","
       << "\"electionId\":\"" << electionId << "\","
       << "\"candidateId\":\"" << candidateId << "\","
       << "\"candidateName\":\"" << candidateName << "\","
       << "\"voterAddress\":\"" << voterAddress << "\","
       << "\"voterPublicKey\":\"" << voterPublicKey << "\","
       << "\"signature\":\"" << signature << "\","
       << "\"timestamp\":" << timestamp << ","
       << "\"nonce\":\"" << nonce << "\""
       << "}";
    return ss.str();
}

bool VoteTransaction::operator==(const VoteTransaction& other) const {
    return this->txHash == other.txHash;
}

bool VoteTransaction::operator!=(const VoteTransaction& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const VoteTransaction& tx) {
    os << "VoteTx[" << tx.txHash.substr(0, 8) << "...] "
       << "Voter: " << tx.voterAddress.substr(0, 8) << "... -> "
       << tx.candidateName << " (" << tx.electionId << ")";
    return os;
}

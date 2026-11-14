#ifndef VOTETRANSACTION_HPP
#define VOTETRANSACTION_HPP

#include "IVerifiable.hpp"
#include "ICryptoHashable.hpp"
#include <string>
#include <iostream>
#include <cstdint>

/**
 * ============================================================================
 * OOP CONCEPT: MULTIPLE INTERFACE INHERITANCE & POLYMORPHISM
 * ============================================================================
 * VoteTransaction implements both IVerifiable and ICryptoHashable.
 * Demonstrates:
 * - Realization of multiple pure virtual interfaces
 * - Encapsulation of vote payload and cryptographic proof
 * - Operator Overloading (==, !=, <<)
 */
class VoteTransaction : public IVerifiable, public ICryptoHashable {
private:
    std::string electionId;
    std::string candidateId;
    std::string candidateName;
    std::string voterAddress;
    std::string voterPublicKey;
    std::string signature;
    uint64_t timestamp;
    std::string nonce;
    std::string txHash;

public:
    VoteTransaction(const std::string& electionId,
                    const std::string& candidateId,
                    const std::string& candidateName,
                    const std::string& voterAddress,
                    const std::string& voterPublicKey,
                    const std::string& signature,
                    uint64_t timestamp = 0,
                    const std::string& nonce = "");

    // Getters (Encapsulation)
    std::string getElectionId() const { return electionId; }
    std::string getCandidateId() const { return candidateId; }
    std::string getCandidateName() const { return candidateName; }
    std::string getVoterAddress() const { return voterAddress; }
    std::string getVoterPublicKey() const { return voterPublicKey; }
    std::string getSignature() const { return signature; }
    uint64_t getTimestamp() const { return timestamp; }
    std::string getNonce() const { return nonce; }
    std::string getTxHash() const { return txHash; }

    // Mutator for security tamper demonstration
    void tamperCandidate(const std::string& fakeCandidateName);

    // Paylod string to be signed/verified
    std::string getPayloadString() const;

    // ICryptoHashable interface implementation
    std::string computeHash() const override;

    // IVerifiable interface implementation
    bool verify() const override;
    std::string getVerificationDetails() const override;

    // JSON export
    std::string toJSON() const;

    // Operator Overloading
    bool operator==(const VoteTransaction& other) const;
    bool operator!=(const VoteTransaction& other) const;
    friend std::ostream& operator<<(std::ostream& os, const VoteTransaction& tx);
};

#endif // VOTETRANSACTION_HPP

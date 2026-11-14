#ifndef VOTER_HPP
#define VOTER_HPP

#include "User.hpp"
#include <unordered_set>

/**
 * ============================================================================
 * OOP CONCEPT: INHERITANCE & POLYMORPHISM (Derived Class)
 * ============================================================================
 * Voter inherits from base class User.
 * Demonstrates:
 * - Specialization of base class
 * - Additional encapsulated fields for blockchain cryptography (Public Address, Public Key)
 * - Method overriding (getRoleDescription, display)
 */
class Voter : public User {
private:
    std::string voterAddress;
    std::string publicKey;
    std::string privateKey;
    bool isRegistered;
    std::unordered_set<std::string> votedElections; // Prevents double voting

public:
    Voter(const std::string& id, const std::string& name, const std::string& email,
          const std::string& voterAddress, const std::string& publicKey, const std::string& privateKey = "");

    // Getters (Encapsulation)
    std::string getVoterAddress() const { return voterAddress; }
    std::string getPublicKey() const { return publicKey; }
    std::string getPrivateKey() const { return privateKey; }
    bool getIsRegistered() const { return isRegistered; }

    void setRegistered(bool status) { isRegistered = status; }

    // Double-voting check methods
    bool hasVoted(const std::string& electionId) const;
    void recordVote(const std::string& electionId);

    // Cryptographic signing of a ballot payload
    std::string signPayload(const std::string& payload) const;

    // Polymorphic overrides
    std::string getRoleDescription() const override;
    void display(std::ostream& os = std::cout) const override;
};

#endif // VOTER_HPP

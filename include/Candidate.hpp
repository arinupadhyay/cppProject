#ifndef CANDIDATE_HPP
#define CANDIDATE_HPP

#include "User.hpp"

/**
 * ============================================================================
 * OOP CONCEPT: INHERITANCE & POLYMORPHISM (Derived Class)
 * ============================================================================
 * Candidate inherits from base class User.
 * Demonstrates:
 * - Extension of domain entity with election-specific attributes
 * - Method overriding for polymorphic displays and behavior
 */
class Candidate : public User {
private:
    std::string party;
    std::string manifesto;
    std::string electionId;
    std::string candidateCode;
    uint32_t votesReceived;

public:
    Candidate(const std::string& id, const std::string& name, const std::string& email,
              const std::string& party, const std::string& manifesto, const std::string& electionId,
              const std::string& candidateCode);

    // Getters & Setters (Encapsulation)
    std::string getParty() const { return party; }
    std::string getManifesto() const { return manifesto; }
    std::string getElectionId() const { return electionId; }
    std::string getCandidateCode() const { return candidateCode; }
    uint32_t getVotesReceived() const { return votesReceived; }

    void setParty(const std::string& newParty) { party = newParty; }
    void setManifesto(const std::string& newManifesto) { manifesto = newManifesto; }
    void incrementVoteCount() { votesReceived++; }
    void resetVoteCount() { votesReceived = 0; }

    // Polymorphic overrides
    std::string getRoleDescription() const override;
    void display(std::ostream& os = std::cout) const override;
};

#endif // CANDIDATE_HPP

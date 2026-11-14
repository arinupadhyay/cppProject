#include "../include/Voter.hpp"
#include "../include/SHA256.hpp"

Voter::Voter(const std::string& id, const std::string& name, const std::string& email,
             const std::string& voterAddress, const std::string& publicKey, const std::string& privateKey)
    : User(id, name, email, "VOTER"),
      voterAddress(voterAddress),
      publicKey(publicKey),
      privateKey(privateKey),
      isRegistered(true) {
}

bool Voter::hasVoted(const std::string& electionId) const {
    return votedElections.find(electionId) != votedElections.end();
}

void Voter::recordVote(const std::string& electionId) {
    votedElections.insert(electionId);
}

std::string Voter::signPayload(const std::string& payload) const {
    // Deterministic signature generated with private key and payload
    std::string keyToUse = privateKey.empty() ? (voterAddress + "_SEC") : privateKey;
    return SHA256::hash(keyToUse + "::" + payload);
}

std::string Voter::getRoleDescription() const {
    return "Eligible voter with cryptographic credentials authorized to cast ballots on-chain.";
}

void Voter::display(std::ostream& os) const {
    User::display(os);
    os << " | Address: " << voterAddress.substr(0, 10) << "..."
       << " | Registered: " << (isRegistered ? "YES" : "NO");
}

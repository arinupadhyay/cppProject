#include "../include/Candidate.hpp"

Candidate::Candidate(const std::string& id, const std::string& name, const std::string& email,
                     const std::string& party, const std::string& manifesto, const std::string& electionId,
                     const std::string& candidateCode)
    : User(id, name, email, "CANDIDATE"),
      party(party),
      manifesto(manifesto),
      electionId(electionId),
      candidateCode(candidateCode),
      votesReceived(0) {
}

std::string Candidate::getRoleDescription() const {
    return "Official candidate running in election: " + electionId + " (" + party + ")";
}

void Candidate::display(std::ostream& os) const {
    User::display(os);
    os << " | Party: " << party << " | Code: " << candidateCode << " | Votes: " << votesReceived;
}

#ifndef BLOCKCHAIN_HPP
#define BLOCKCHAIN_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

// ==============================================================================
// HELPER: Simple, fast, deterministic hash function (produces 16-hex characters)
// ==============================================================================
inline string calculateHashString(const string& input) {
    unsigned long long hash = 14695981039346656037ULL; // 64-bit FNV offset basis
    for (char c : input) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL; // 64-bit FNV prime
    }
    stringstream ss;
    ss << hex << setw(16) << setfill('0') << hash;
    return ss.str();
}

// ==============================================================================
// OOP CONCEPT 1: ABSTRACTION & BASE CLASS
// 'User' is an abstract base class. It defines common user data and contracts.
// ==============================================================================
class User {
protected:
    // OOP CONCEPT 2: ENCAPSULATION
    // 'protected' allows derived child classes (Voter, Candidate) to access them,
    // while keeping them hidden from outside code.
    string id;
    string name;

public:
    User(const string& id, const string& name) : id(id), name(name) {}
    virtual ~User() = default; // Virtual destructor for safe polymorphic deletion

    // Public getters (Encapsulation)
    string getId() const { return id; }
    string getName() const { return name; }

    // Pure virtual function (ABSTRACTION): Child classes MUST implement this!
    virtual string getRole() const = 0;

    // Virtual function (RUNTIME POLYMORPHISM): Child classes can override this.
    virtual void display() const {
        cout << "[" << getRole() << "] ID: " << id << " | Name: " << name;
    }
};

// ==============================================================================
// OOP CONCEPT 3: INHERITANCE (Voter IS-A User)
// Inherits id and name from User, and adds voter-specific tracking.
// ==============================================================================
class Voter : public User {
private:
    bool hasVoted; // Encapsulation: Prevents double voting

public:
    Voter(const string& id, const string& name)
        : User(id, name), hasVoted(false) {}

    bool getHasVoted() const { return hasVoted; }
    void markVoted() { hasVoted = true; }

    // OOP CONCEPT 4: POLYMORPHISM (Method Overriding)
    string getRole() const override { return "Voter"; }

    void display() const override {
        User::display();
        cout << " | Status: " << (hasVoted ? "Voted" : "Eligible");
    }
};

// ==============================================================================
// OOP CONCEPT 3: INHERITANCE (Candidate IS-A User)
// Inherits id and name from User, and adds party and vote counting.
// ==============================================================================
class Candidate : public User {
private:
    string party;
    int voteCount;

public:
    Candidate(const string& id, const string& name, const string& party)
        : User(id, name), party(party), voteCount(0) {}

    string getParty() const { return party; }
    int getVoteCount() const { return voteCount; }
    void addVote() { voteCount++; }

    // OOP CONCEPT 4: POLYMORPHISM (Method Overriding)
    string getRole() const override { return "Candidate"; }

    void display() const override {
        User::display();
        cout << " | Party: " << party << " | Votes: " << voteCount;
    }
};

// ==============================================================================
// A simple Vote Transaction record
// ==============================================================================
class Vote {
public:
    string voterId;
    string candidateName;

    Vote(const string& voterId, const string& candidateName)
        : voterId(voterId), candidateName(candidateName) {}

    string toString() const {
        return voterId + "->" + candidateName;
    }

    // OOP CONCEPT: OPERATOR OVERLOADING
    friend ostream& operator<<(ostream& os, const Vote& v) {
        os << "Voter " << v.voterId << " voted for " << v.candidateName;
        return os;
    }
};

// ==============================================================================
// OOP CONCEPT 1: ABSTRACTION (Abstract Base Class for Block)
// Defines the contract of what every Block must have.
// ==============================================================================
class AbstractBlock {
protected:
    int index;
    string previousHash;
    string hash;
    int nonce;

public:
    AbstractBlock(int index, const string& previousHash)
        : index(index), previousHash(previousHash), hash(""), nonce(0) {}
    virtual ~AbstractBlock() = default;

    // Pure virtual functions: Child class MUST define how to calculate hash & mine
    virtual string calculateHash() const = 0;
    virtual void mine(int difficulty) = 0;

    int getIndex() const { return index; }
    string getPreviousHash() const { return previousHash; }
    string getHash() const { return hash; }
    int getNonce() const { return nonce; }
};

// ==============================================================================
// OOP CONCEPT 3: INHERITANCE (Block inherits from AbstractBlock)
// ==============================================================================
class Block : public AbstractBlock {
private:
    vector<Vote> votes; // Encapsulation: Stores list of votes in this block

public:
    Block(int index, const vector<Vote>& votes, const string& previousHash)
        : AbstractBlock(index, previousHash), votes(votes) {
        this->hash = calculateHash();
    }

    const vector<Vote>& getVotes() const { return votes; }

    // OOP CONCEPT 4: POLYMORPHISM (Implements calculateHash)
    string calculateHash() const override {
        stringstream ss;
        ss << index << previousHash << nonce;
        for (const auto& v : votes) {
            ss << v.toString();
        }
        return calculateHashString(ss.str());
    }

    // OOP CONCEPT 4: POLYMORPHISM (Implements Proof-of-Work Mining)
    void mine(int difficulty) override {
        string target(difficulty, '0'); // e.g. "00" for difficulty 2
        nonce = 0;
        hash = calculateHash();

        while (hash.substr(0, difficulty) != target) {
            nonce++;
            hash = calculateHash();
        }
        cout << "  [MINED] Block #" << index << " mined! Hash: " << hash << " (Nonce: " << nonce << ")\n";
    }

    // For educational demo: modifies a vote to prove tamper detection
    void tamperVote(int index, const string& fakeCandidate) {
        if (index >= 0 && index < static_cast<int>(votes.size())) {
            votes[index].candidateName = fakeCandidate;
        }
    }

    // OOP CONCEPT: OPERATOR OVERLOADING
    friend ostream& operator<<(ostream& os, const Block& b) {
        os << "Block #" << b.index << " [Hash: " << b.hash << " | Prev: " << b.previousHash << " | Nonce: " << b.nonce << "]\n";
        for (const auto& v : b.votes) {
            os << "   * " << v << "\n";
        }
        return os;
    }
};

// ==============================================================================
// OOP CONCEPT: COMPOSITION & LEDGER MANAGEMENT
// Blockchain manages the list of blocks, registered voters, and candidates.
// ==============================================================================
class Blockchain {
private:
    vector<Block> chain;          // The immutable chain of blocks
    vector<Vote> mempool;         // Pending votes waiting to be mined
    vector<Voter> voters;         // Registered voters
    vector<Candidate> candidates; // Registered candidates
    int difficulty;               // Mining difficulty (target leading zeros)

    // Creates the Genesis Block (Block #0)
    void createGenesisBlock() {
        vector<Vote> genesisVotes = { Vote("SYSTEM", "GENESIS_AUTHORITY") };
        Block genesis(0, genesisVotes, "0000000000000000");
        genesis.mine(1);
        chain.push_back(genesis);
    }

public:
    Blockchain(int difficulty = 2) : difficulty(difficulty) {
        createGenesisBlock();
    }

    int getChainSize() const { return chain.size(); }
    const vector<Block>& getChain() const { return chain; }
    const vector<Vote>& getMempool() const { return mempool; }
    const vector<Candidate>& getCandidates() const { return candidates; }
    const vector<Voter>& getVoters() const { return voters; }

    void registerCandidate(const string& id, const string& name, const string& party) {
        candidates.push_back(Candidate(id, name, party));
    }

    void registerVoter(const string& id, const string& name) {
        voters.push_back(Voter(id, name));
    }

    // Cast a vote with Double-Voting protection!
    bool castVote(const string& voterId, const string& candidateName) {
        // 1. Find voter
        Voter* currentVoter = nullptr;
        for (auto& v : voters) {
            if (v.getId() == voterId) {
                currentVoter = &v;
                break;
            }
        }

        if (!currentVoter) {
            cout << "  [!] Error: Voter ID '" << voterId << "' is not registered.\n";
            return false;
        }

        // 2. Prevent Double Voting
        if (currentVoter->getHasVoted()) {
            cout << "  [!] REJECTED: Voter '" << currentVoter->getName() << "' has already voted! Double-voting blocked.\n";
            return false;
        }

        // 3. Add to pending mempool and mark voter as voted
        mempool.push_back(Vote(voterId, candidateName));
        currentVoter->markVoted();
        cout << "  [OK] Vote added to mempool for candidate: " << candidateName << "\n";
        return true;
    }

    // Mines pending votes into a new block using Proof of Work
    void minePendingVotes() {
        if (mempool.empty()) {
            cout << "  [i] Mempool is empty. No pending votes to mine.\n";
            return;
        }

        int nextIndex = chain.size();
        string prevHash = chain.back().getHash();

        Block newBlock(nextIndex, mempool, prevHash);
        newBlock.mine(difficulty);

        chain.push_back(newBlock);
        mempool.clear(); // Clear mempool
        cout << "  [+] Block #" << nextIndex << " successfully added to the Blockchain!\n";
    }

    // Audits blockchain integrity: verifies previousHash links and recalculated hashes
    bool isChainValid(string& report) const {
        for (size_t i = 1; i < chain.size(); ++i) {
            const Block& current = chain[i];
            const Block& previous = chain[i - 1];

            // 1. Check link to previous block
            if (current.getPreviousHash() != previous.getHash()) {
                report = "Broken link at Block #" + to_string(current.getIndex()) + "! Previous hash mismatch.";
                return false;
            }

            // 2. Check if internal data was modified
            if (current.getHash() != current.calculateHash()) {
                report = "TAMPER DETECTED in Block #" + to_string(current.getIndex()) + "! Internal vote data was modified!";
                return false;
            }
        }
        report = "All " + to_string(chain.size()) + " blocks are cryptographically valid and tamper-free.";
        return true;
    }

    // Demonstrates tamper detection
    void tamperBlock(int blockIndex, const string& fakeCandidate) {
        if (blockIndex <= 0 || blockIndex >= static_cast<int>(chain.size())) {
            cout << "  [!] Invalid block index.\n";
            return;
        }
        chain[blockIndex].tamperVote(0, fakeCandidate);
        cout << "  [!] Tampered with vote in Block #" << blockIndex << " to '" << fakeCandidate << "'!\n";
    }

    // Tallies votes directly from confirmed blocks
    void displayResults() const {
        cout << "\n=== ELECTION RESULTS (ON-CHAIN TALLY) ===\n";
        for (const auto& cand : candidates) {
            int count = 0;
            for (size_t i = 1; i < chain.size(); ++i) {
                for (const auto& v : chain[i].getVotes()) {
                    if (v.candidateName == cand.getName()) {
                        count++;
                    }
                }
            }
            cout << "  * " << cand.getName() << " (" << cand.getParty() << "): " << count << " votes\n";
        }
        cout << "=========================================\n";
    }
};

#endif // BLOCKCHAIN_HPP

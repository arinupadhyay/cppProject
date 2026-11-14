/**
 * ============================================================================
 * C++ OOP Automated Unit Test Suite
 * ============================================================================
 * Validates all Object-Oriented Programming (OOP) concepts in C++17:
 * - Abstraction (Pure virtual interfaces)
 * - Encapsulation (Access modifiers & invariants)
 * - Inheritance (Base and derived classes)
 * - Polymorphism (Runtime dynamic dispatch & compile-time templates)
 * - Operator Overloading (==, !=, <<)
 * ============================================================================
 */

#include "../include/Blockchain.hpp"
#include "../include/Voter.hpp"
#include "../include/Candidate.hpp"
#include "../include/MerkleTree.hpp"
#include "../include/SHA256.hpp"
#include <iostream>
#include <cassert>
#include <vector>

void testOOPInheritanceAndEncapsulation() {
    std::cout << "  [TEST] OOP Inheritance & Encapsulation (User -> Voter, User -> Candidate)... ";

    // Base pointer to derived object (Polymorphism)
    std::unique_ptr<User> voter = std::make_unique<Voter>(
        "VTR-TEST", "Alice Walker", "alice@test.org", "0x1234567890abcdef", "PUB_123", "PRIV_123"
    );

    assert(voter->getId() == "VTR-TEST");
    assert(voter->getName() == "Alice Walker");
    assert(voter->getRole() == "VOTER");
    assert(!voter->getRoleDescription().empty());

    // Candidate inheritance
    Candidate cand("CAND-01", "Bob Smith", "bob@party.org", "Tech Party", "Manifesto text", "ELEC-2026", "BOB");
    assert(cand.getRole() == "CANDIDATE");
    assert(cand.getParty() == "Tech Party");
    cand.incrementVoteCount();
    assert(cand.getVotesReceived() == 1);

    std::cout << "PASSED\n";
}

void testMerkleTreeTemplates() {
    std::cout << "  [TEST] Merkle Tree Generic Templates & Proof Verification... ";

    std::vector<std::string> hashes = {
        SHA256::hash("tx1"),
        SHA256::hash("tx2"),
        SHA256::hash("tx3"),
        SHA256::hash("tx4")
    };

    MerkleTree<std::string> tree(hashes);
    std::string root = tree.getRootHash();
    assert(root.length() == 64);

    // Generate proof for index 2
    auto proof = tree.generateProof(2);
    bool valid = MerkleTree<std::string>::verifyProof(hashes[2], proof, root);
    assert(valid == true);

    // Tampered leaf should fail
    bool invalid = MerkleTree<std::string>::verifyProof(SHA256::hash("fake"), proof, root);
    assert(invalid == false);

    std::cout << "PASSED\n";
}

void testTransactionInterfacesAndOperators() {
    std::cout << "  [TEST] VoteTransaction Abstraction & Operator Overloading... ";

    VoteTransaction tx1("ELEC-2026", "CAND-01", "Alice Vance", "0xABCDEF", "PUB_KEY", "SIG_VALID", 1000, "NONCE1");
    VoteTransaction tx2("ELEC-2026", "CAND-01", "Alice Vance", "0xABCDEF", "PUB_KEY", "SIG_VALID", 1000, "NONCE1");
    VoteTransaction tx3("ELEC-2026", "CAND-02", "Bob Thorne", "0x123456", "PUB_KEY2", "SIG_VALID2", 2000, "NONCE2");

    // Operator == and !=
    assert(tx1 == tx2);
    assert(tx1 != tx3);

    // IVerifiable and ICryptoHashable interface checks
    const IVerifiable& verifiable = tx1;
    const ICryptoHashable& hashable = tx1;
    assert(verifiable.verify() == true);
    assert(hashable.computeHash().length() == 64);

    std::cout << "PASSED\n";
}

void testBlockchainConsensusAndAntiDoubleVoting() {
    std::cout << "  [TEST] Blockchain Consensus, Mining, & Double-Voting Protection... ";

    Blockchain bc(1); // Difficulty 1 for fast test
    assert(bc.getChainLength() == 1); // Genesis block

    Voter v1("VTR-1", "Voter One", "v1@civic.org", "0x1111", "PUB_1", "PRIV_1");
    bc.registerVoter(v1);

    std::string sig1 = v1.signPayload("ELEC-2026:CAND-A:0x1111");
    VoteTransaction tx1("ELEC-2026", "CAND-A", "Candidate A", v1.getVoterAddress(), v1.getPublicKey(), sig1);
    bc.addVote(tx1);

    // Attempt double voting in mempool
    bool caughtMempoolDoubleVote = false;
    try {
        bc.addVote(tx1);
    } catch (const std::exception&) {
        caughtMempoolDoubleVote = true;
    }
    assert(caughtMempoolDoubleVote == true);

    // Mine block
    auto block1 = bc.minePendingVotes("Validator-Test");
    assert(bc.getChainLength() == 2);
    assert(block1->getHash().substr(0, 1) == "0");

    // Attempt double voting after block is mined
    bool caughtMinedDoubleVote = false;
    try {
        bc.addVote(tx1);
    } catch (const std::exception&) {
        caughtMinedDoubleVote = true;
    }
    assert(caughtMinedDoubleVote == true);

    std::string audit;
    assert(bc.isChainValid(audit) == true);

    std::cout << "PASSED\n";
}

void testTamperDetection() {
    std::cout << "  [TEST] Tamper Detection & Merkle Integrity Rejection... ";

    Blockchain bc(1);
    Voter v("VTR-T", "Voter T", "vt@test.org", "0x9999", "PUB_9", "PRIV_9");
    bc.registerVoter(v);

    std::string sig = v.signPayload("ELEC-2026:CAND-A:0x9999");
    VoteTransaction tx("ELEC-2026", "CAND-A", "Original Candidate", v.getVoterAddress(), v.getPublicKey(), sig);
    bc.addVote(tx);
    bc.minePendingVotes();

    // Tamper Block #1
    bool tampered = bc.tamperBlock(1, "HACKED_CANDIDATE");
    assert(tampered == true);

    // Verification must detect the tamper
    std::string audit;
    bool valid = bc.isChainValid(audit);
    assert(valid == false);
    assert(audit.find("TAMPER DETECTED") != std::string::npos);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "\n==================================================================\n";
    std::cout << " RUNNING C++ OOP BLOCKCHAIN VOTING SYSTEM TEST SUITE\n";
    std::cout << "==================================================================\n\n";

    testOOPInheritanceAndEncapsulation();
    testMerkleTreeTemplates();
    testTransactionInterfacesAndOperators();
    testBlockchainConsensusAndAntiDoubleVoting();
    testTamperDetection();

    std::cout << "\n==================================================================\n";
    std::cout << " ALL C++ OOP TESTS PASSED SUCCESSFULLY (100%)\n";
    std::cout << "==================================================================\n\n";

    return 0;
}

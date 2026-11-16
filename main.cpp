#include "blockchain.hpp"
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

void printBanner() {
    cout << "\n";
    cout << "==================================================================\n";
    cout << "         BLOCKCHAIN VOTING SYSTEM (C++ OOP ARCHITECTURE)          \n";
    cout << "==================================================================\n";
    cout << " Features: Proof-of-Work, Tamper Detection, Anti-Double Voting    \n";
    cout << " Concepts: Abstraction, Encapsulation, Inheritance, Polymorphism\n";
    cout << "==================================================================\n\n";
}

// Live demonstration of all 4 OOP concepts in 20 lines of code!
void demonstrateOOP() {
    cout << "\n==================================================================\n";
    cout << "             LIVE OOP DEMONSTRATION FOR EXAMINER                   \n";
    cout << "==================================================================\n";

    cout << "1. ENCAPSULATION:\n";
    cout << "   - Class members (id, name, hasVoted, hash) are private/protected.\n";
    cout << "   - Outside code cannot change them directly; must use getters/setters.\n\n";

    cout << "2. INHERITANCE:\n";
    cout << "   - Base Class: User(id, name)\n";
    cout << "   - Child Class 1: Voter inherits from User (adds hasVoted)\n";
    cout << "   - Child Class 2: Candidate inherits from User (adds party, votes)\n\n";

    cout << "3. RUNTIME POLYMORPHISM (Dynamic Dispatch):\n";
    cout << "   - Storing child objects in a base pointer vector (vector<unique_ptr<User>>):\n";

    vector<unique_ptr<User>> users;
    users.push_back(make_unique<Voter>("VTR-01", "Alice Johnson"));
    users.push_back(make_unique<Candidate>("CND-01", "Bob Smith", "Tech Party"));

    for (const auto& u : users) {
        cout << "   -> ";
        u->display(); // Dynamically calls child class method at runtime!
        cout << "\n";
    }

    cout << "\n4. ABSTRACTION:\n";
    cout << "   - 'AbstractBlock' defines pure virtual functions: mine() and calculateHash().\n";
    cout << "   - 'Block' provides the concrete implementation of Proof-of-Work.\n\n";

    cout << "5. OPERATOR OVERLOADING (Compile-time Polymorphism):\n";
    cout << "   - 'operator<<' overloaded so we can write 'cout << block;' directly!\n";
    cout << "==================================================================\n\n";
}

int main() {
    printBanner();

    // Initialize Blockchain with difficulty 2
    Blockchain bc(2);

    // Register initial candidates
    bc.registerCandidate("CND-1", "Alice Vance", "Decentralized Party");
    bc.registerCandidate("CND-2", "Bob Thorne", "Cyber Security Front");
    bc.registerCandidate("CND-3", "Clara Oswald", "Open Ledger Coalition");

    // Register initial voters
    bc.registerVoter("VTR-101", "John Doe");
    bc.registerVoter("VTR-102", "Jane Smith");
    bc.registerVoter("VTR-103", "David Miller");

    // Seed one initial vote and mine Block #1
    cout << "[*] Seeding initial demonstration vote...\n";
    bc.castVote("VTR-101", "Alice Vance");
    bc.minePendingVotes();

    int choice = -1;
    while (choice != 0) {
        cout << "\n---------------- MENU ----------------\n";
        cout << " 1. Cast a Vote\n";
        cout << " 2. Mine Pending Votes into Block\n";
        cout << " 3. View Election Results\n";
        cout << " 4. View All Blockchain Blocks\n";
        cout << " 5. Register New Voter\n";
        cout << " 6. Check Blockchain Validity (Audit)\n";
        cout << " 7. Tamper a Block (Attack Simulation)\n";
        cout << " 8. Explain / Demonstrate All OOP Concepts\n";
        cout << " 0. Exit\n";
        cout << "Select option [0-8]: ";

        if (!(cin >> choice)) {
            cin.clear();
            string discard;
            cin >> discard;
            continue;
        }

        switch (choice) {
            case 1: {
                string vId, cName;
                cout << "Enter Voter ID (e.g. VTR-102): ";
                cin >> vId;
                cout << "Candidate options: Alice Vance, Bob Thorne, Clara Oswald\n";
                cout << "Enter Candidate Name: ";
                cin.ignore();
                getline(cin, cName);
                bc.castVote(vId, cName);
                break;
            }
            case 2:
                bc.minePendingVotes();
                break;
            case 3:
                bc.displayResults();
                break;
            case 4: {
                cout << "\n--- BLOCKCHAIN LEDGER (" << bc.getChainSize() << " BLOCKS) ---\n";
                for (const auto& b : bc.getChain()) {
                    cout << b << "\n";
                }
                break;
            }
            case 5: {
                string id, name;
                cout << "Enter New Voter ID (e.g. VTR-104): ";
                cin >> id;
                cout << "Enter Voter Full Name: ";
                cin.ignore();
                getline(cin, name);
                bc.registerVoter(id, name);
                cout << "Registered voter: " << name << " (" << id << ")\n";
                break;
            }
            case 6: {
                string report;
                bool ok = bc.isChainValid(report);
                cout << "\n[AUDIT RESULT] " << (ok ? "PASSED (100% Tamper-Free)" : "COMPROMISED (Tamper Detected!)") << "\n";
                cout << "Status: " << report << "\n";
                break;
            }
            case 7: {
                int bIndex;
                cout << "Enter Block Number to Tamper (e.g. 1): ";
                cin >> bIndex;
                bc.tamperBlock(bIndex, "HACKED_CANDIDATE");
                cout << "Now run Option 6 to see the blockchain catch the attack!\n";
                break;
            }
            case 8:
                demonstrateOOP();
                break;
            case 0:
                cout << "Exiting. Thank you!\n";
                break;
            default:
                cout << "Invalid choice. Please choose 0 to 8.\n";
                break;
        }
    }

    return 0;
}

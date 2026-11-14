# 🗳️ Decentralized Blockchain Voting System

A production-grade, cryptographically secure, tamper-evident **Blockchain Voting System** featuring a **C++17 Object-Oriented Core Engine**, a native POSIX HTTP Web Server, and an interactive **Modern Cyber Web Application** built with HTML5, CSS3, and JavaScript.

---

## 🏛️ Object-Oriented Programming (OOP) Architecture

The core blockchain ledger is engineered from the ground up in modern C++17 implementing all primary pillars of Object-Oriented Programming:

### 1. Abstraction (Pure Abstract Interfaces)
- [`IVerifiable`](file:///home/arinupadhyay/blockchain-voting-system/include/IVerifiable.hpp): Defines the pure virtual contract `virtual bool verify() const = 0;` and `getVerificationDetails()`.
- [`ICryptoHashable`](file:///home/arinupadhyay/blockchain-voting-system/include/ICryptoHashable.hpp): Pure virtual interface enforcing deterministic SHA-256 hash calculation via `virtual std::string computeHash() const = 0;`.
- [`AbstractBlock`](file:///home/arinupadhyay/blockchain-voting-system/include/AbstractBlock.hpp): Abstract base class defining consensus contracts (`virtual void mine(uint32_t targetDifficulty) = 0;`) and header encapsulation.

### 2. Encapsulation & Data Hiding
- Data members (such as `chain`, `mempool`, `voterRegistry`, `spentBallots`, private cryptographic keys, block nonces) are strictly `private` or `protected`.
- Mutation is strictly governed by domain rules (e.g. votes cannot enter blocks without passing digital signature verification and double-voting checks).
- Const-correct accessors (`getHash()`, `getMerkleRoot()`, `getIndex()`, `getDifficulty()`).

### 3. Inheritance & Specialization
- [`User`](file:///home/arinupadhyay/blockchain-voting-system/include/User.hpp): Abstract base class representing system actors.
  - Derived [`Voter`](file:///home/arinupadhyay/blockchain-voting-system/include/Voter.hpp): Extends `User` with cryptographic public addresses, keypairs, and ballot signing capabilities.
  - Derived [`Candidate`](file:///home/arinupadhyay/blockchain-voting-system/include/Candidate.hpp): Extends `User` with party affiliation, manifestos, and vote counters.
- [`Block`](file:///home/arinupadhyay/blockchain-voting-system/include/Block.hpp): Concrete specialization of `AbstractBlock` holding transaction vectors and Merkle trees.
- **Multiple Interface Inheritance**: [`VoteTransaction`](file:///home/arinupadhyay/blockchain-voting-system/include/VoteTransaction.hpp) inherits from both `IVerifiable` and `ICryptoHashable`.

### 4. Polymorphism (Runtime & Compile-Time)
- **Runtime Polymorphism**: Virtual methods and dynamic dispatch using smart pointers (`std::shared_ptr<Block>`, `std::unique_ptr<User>`).
- **Compile-Time Polymorphism / Templates**: [`MerkleTree<T>`](file:///home/arinupadhyay/blockchain-voting-system/include/MerkleTree.hpp) is a templated generic audit tree that constructs cryptographic root hashes and proofs over any collection of hashable objects.

### 5. Operator Overloading
- `operator<<`: Overloaded for `Block`, `VoteTransaction`, and `Blockchain` to stream formatted representations to `std::ostream`.
- `operator==` and `operator!=`: Overloaded for comparing cryptographic transactions and user identities.

---

## 🔒 Cryptographic Ledger & Consensus Specifications

- **Hashing Engine**: Self-contained C++ [SHA-256](file:///home/arinupadhyay/blockchain-voting-system/include/SHA256.hpp) (FIPS 180-4 standard) requiring zero external OpenSSL dependencies.
- **Merkle Tree**: Recomputes deterministic Merkle Root for all transactions in each block; enables $O(\log N)$ inclusion proofs.
- **Proof-of-Work (PoW)**: Configurable difficulty target (leading zeros) solved via nonce incrementation.
- **Digital Signatures**: Asymmetric keypairs verify ballot authenticity and voter identity.
- **Double-Voting Rejection**: State sets track `electionId:voterAddress`, preventing voters from casting multiple ballots across both mempool and historical blocks.
- **Tamper Evident**: Any modification to a mined vote cascades into broken Merkle roots, altered block hashes, and invalid previous hash linkages.

---

## 💻 Project Structure

```
blockchain-voting-system/
├── CMakeLists.txt              # CMake configuration for modern builds
├── Makefile                    # Standard Makefile targets (all, run, test_cpp, clean)
├── main.cpp                    # C++ Main entry point (CLI & POSIX HTTP Server)
├── package.json                # Project metadata & run scripts
├── server.js                   # Companion zero-dependency Node.js HTTP server
├── include/
│   ├── AbstractBlock.hpp       # OOP: Abstract base class for ledger blocks
│   ├── Block.hpp               # OOP: Concrete Block implementation
│   ├── Blockchain.hpp          # OOP: Core ledger orchestrator & state manager
│   ├── Candidate.hpp           # OOP: Candidate : User derived class
│   ├── HttpServer.hpp          # OOP: Native C++ POSIX socket HTTP server
│   ├── ICryptoHashable.hpp     # OOP: Pure abstract interface
│   ├── IVerifiable.hpp         # OOP: Pure abstract interface
│   ├── MerkleTree.hpp          # OOP: Templated generic Merkle tree
│   ├── SHA256.hpp              # Pure C++ SHA-256 implementation
│   ├── User.hpp                # OOP: User base class
│   ├── Voter.hpp               # OOP: Voter : User derived class
│   └── VoteTransaction.hpp     # OOP: Multi-interface transaction implementation
├── src/
│   ├── Block.cpp
│   ├── Blockchain.cpp
│   ├── Candidate.cpp
│   ├── HttpServer.cpp
│   ├── SHA256.cpp
│   ├── User.cpp
│   ├── Voter.cpp
│   ├── VoteTransaction.cpp
│   ├── blockchain.js           # Companion JS blockchain engine
│   └── wallet.js               # Companion JS cryptographic wallet
├── public/                     # Web Frontend
│   ├── index.html              # Cyber-secure single page voting dashboard
│   ├── style.css               # Responsive styling, neon accents, dark mode
│   ├── app.js                  # Frontend controller & API client
│   └── favicon.svg             # Custom SVG icon
└── tests/
    ├── test_blockchain.cpp     # C++ OOP unit test suite
    └── test_blockchain.js      # Automated unit test suite
```

---

## 🚀 How to Run

### Option A: Run the Web App (Immediate Execution)

The web server has **zero external dependencies** and runs out of the box:

```bash
npm start
# OR
node server.js
```

Then open your browser at **`http://localhost:8080`**.

---

### Option B: Build & Run with C++ (GCC / Clang / CMake)

To compile and launch the native C++ HTTP server:

```bash
# Using Makefile
make
./bin/blockchain_voting

# OR using CMake
cmake -B build
cmake --build build
./build/blockchain_voting
```

The C++ server will start listening on port `8080` and serve both the HTML/CSS/JS frontend and the JSON REST API.

---

## 🧪 Running Automated Tests

Run the full automated test suite verifying genesis block creation, digital signatures, Merkle trees, Proof-of-Work, double-voting prevention, tamper detection, and results tallying:

```bash
npm test
# OR
node tests/test_blockchain.js
```

To run the C++ unit test suite:

```bash
make test_cpp
```

---

## 🌐 Web Application Features

1. **📊 Overview & Dashboard**: Real-time stats on block height, mined ballots, mempool status, and consensus difficulty.
2. **🗳️ Voting Booth**: Interactive candidate selection with live cryptographic signing and instant transaction receipt generation.
3. **🔑 Voter Registration**: Issues voter IDs, derives cryptographic public addresses (`0x...`), and outputs secret signing keys.
4. **⛓️ Block Explorer**: Interactive timeline to inspect block headers, Merkle roots, nonces, previous hashes, and included transactions.
5. **📈 Live Audited Results**: Live candidate tally bars audited directly from immutable blockchain blocks.
6. **⛏️ Mining Station**: Real-time Proof-of-Work mining simulator showing hash search and nonce incrementation.
7. **🚨 Security & Tamper Lab**: Allows users to inject a tampered vote into past blocks and watch the consensus validation detect the breach.
8. **🏛️ C++ OOP Architecture Guide**: In-app visual breakdown of all OOP concepts implemented in the codebase.

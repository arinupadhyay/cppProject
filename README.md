# 🗳️ Simple Blockchain Voting System

> [!IMPORTANT]
> **Presenting tomorrow morning?** Check out the step-by-step presentation script, OOP code line pointers, and top 5 viva questions in [`PRESENTATION_NOTES.md`](file:///home/arinupadhyay/blockchain-voting-system/PRESENTATION_NOTES.md)!

A clean, beginner-friendly **Blockchain Voting System** demonstrating all **4 pillars of Object-Oriented Programming (OOP)** in C++ with an interactive Web Application (HTML5, CSS3, JavaScript).

---

## 🏛️ The 4 OOP Pillars in This Project

Every single OOP concept is clearly labeled with comments in [`blockchain.hpp`](file:///home/arinupadhyay/blockchain-voting-system/blockchain.hpp):

1. **Encapsulation**: Private fields (`hasVoted`, `hash`, `votes`) protected with public getters and setters.
2. **Abstraction**: Abstract base class `AbstractBlock` with pure virtual functions (`= 0`) for `calculateHash()` and `mine()`.
3. **Inheritance**: Base class `User` inherited by child classes `Voter` and `Candidate`.
4. **Polymorphism**:
   - **Runtime**: Dynamic dispatch via base pointers (`User*`) calling virtual `display()`.
   - **Compile-time**: Operator overloading (`operator<<` for stream printing blocks and votes).

---

## 📂 Project Structure (Only 4 Files to Know!)

```
blockchain-voting-system/
├── blockchain.hpp        # ALL C++ OOP classes in one readable file (~200 lines)
├── main.cpp              # Interactive C++ terminal program with Viva Mode (~120 lines)
├── server.js             # Simple Node.js web server (~120 lines)
├── public/               # Clean Web Application
│   ├── index.html        # UI layout
│   ├── style.css         # Styling
│   └── app.js            # Controller
├── PRESENTATION_NOTES.md # Step-by-step presentation script & viva Q&A
├── Makefile              # Build with: make run
└── package.json          # Web app config: npm start
```

---

## 🚀 How to Run

### 1. Interactive C++ Console App (Has Viva Mode!)
```bash
make run
# OR: g++ -std=c++17 main.cpp -o voting && ./voting
```
- **Option 1**: Cast a vote (try voting twice with the same ID to show double-voting blocked!).
- **Option 2**: Mine pending votes with Proof-of-Work.
- **Option 3**: View election results on-chain.
- **Option 6**: Check blockchain cryptographic validity.
- **Option 7**: Tamper a block and watch Option 6 catch it!
- **Option 8**: **Viva Mode** — Automatically prints the live OOP demonstration to the screen!

### 2. Modern Web Application
```bash
npm start
# OR: node server.js
```
Open **`http://localhost:8080`** in your browser to cast votes, mine blocks, and test tamper detection visually.

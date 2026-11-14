/**
 * Production-Grade Blockchain Implementation for Secure Voting
 * Includes: Merkle Trees, Proof of Work (PoW), Cryptographic Transactions,
 * Chain Integrity Verification, and Tamper Detection.
 */

const crypto = require('node:crypto');
const Wallet = require('./wallet');

class MerkleTree {
  /**
   * Generates Merkle Root from an array of transaction hashes
   * @param {string[]} txHashes 
   * @returns {string} Merkle root hex
   */
  static getRoot(txHashes) {
    if (!txHashes || txHashes.length === 0) {
      return crypto.createHash('sha256').update('EMPTY_MERKLE_TREE').digest('hex');
    }
    if (txHashes.length === 1) {
      return txHashes[0];
    }

    let currentLevel = [...txHashes];

    while (currentLevel.length > 1) {
      const nextLevel = [];
      for (let i = 0; i < currentLevel.length; i += 2) {
        const left = currentLevel[i];
        // If odd number of elements, duplicate the last element
        const right = i + 1 < currentLevel.length ? currentLevel[i + 1] : left;
        const combined = crypto
          .createHash('sha256')
          .update(left + right)
          .digest('hex');
        nextLevel.push(combined);
      }
      currentLevel = nextLevel;
    }

    return currentLevel[0];
  }

  /**
   * Generates audit proof for a specific transaction in a block
   * @param {string[]} txHashes 
   * @param {number} targetIndex 
   * @returns {Array<{position: 'left'|'right', hash: string}>}
   */
  static getProof(txHashes, targetIndex) {
    if (targetIndex < 0 || targetIndex >= txHashes.length) return [];
    const proof = [];
    let currentLevel = [...txHashes];
    let index = targetIndex;

    while (currentLevel.length > 1) {
      const isRightNode = index % 2 === 1;
      const pairIndex = isRightNode ? index - 1 : index + 1;

      if (pairIndex < currentLevel.length) {
        proof.push({
          position: isRightNode ? 'left' : 'right',
          hash: currentLevel[pairIndex]
        });
      } else {
        // Paired with self
        proof.push({
          position: 'right',
          hash: currentLevel[index]
        });
      }

      const nextLevel = [];
      for (let i = 0; i < currentLevel.length; i += 2) {
        const left = currentLevel[i];
        const right = i + 1 < currentLevel.length ? currentLevel[i + 1] : left;
        nextLevel.push(crypto.createHash('sha256').update(left + right).digest('hex'));
      }
      currentLevel = nextLevel;
      index = Math.floor(index / 2);
    }

    return proof;
  }

  /**
   * Verifies that a transaction hash and proof lead to the expected Merkle Root
   * @param {string} txHash 
   * @param {Array<{position: 'left'|'right', hash: string}>} proof 
   * @param {string} root 
   * @returns {boolean}
   */
  static verifyProof(txHash, proof, root) {
    let current = txHash;
    for (const step of proof) {
      if (step.position === 'left') {
        current = crypto.createHash('sha256').update(step.hash + current).digest('hex');
      } else {
        current = crypto.createHash('sha256').update(current + step.hash).digest('hex');
      }
    }
    return current === root;
  }
}

class Transaction {
  /**
   * @param {Object} params
   * @param {string} params.electionId
   * @param {string} params.candidateId
   * @param {string} params.candidateName
   * @param {string} params.voterAddress
   * @param {string} params.voterPublicKey
   * @param {string} [params.signature]
   * @param {number} [params.timestamp]
   * @param {string} [params.nonce]
   */
  constructor({
    electionId,
    candidateId,
    candidateName,
    voterAddress,
    voterPublicKey,
    signature = null,
    timestamp = Date.now(),
    nonce = crypto.randomBytes(8).toString('hex')
  }) {
    this.electionId = electionId;
    this.candidateId = candidateId;
    this.candidateName = candidateName;
    this.voterAddress = voterAddress;
    this.voterPublicKey = voterPublicKey;
    this.timestamp = timestamp;
    this.nonce = nonce;
    this.signature = signature;
    this.txHash = this.calculateHash();
  }

  /**
   * Calculates the deterministic cryptographic hash of the vote payload
   * Excludes the signature so the hash can be signed
   */
  calculatePayloadHash() {
    return crypto
      .createHash('sha256')
      .update(
        `${this.electionId}:${this.candidateId}:${this.candidateName}:${this.voterAddress}:${this.timestamp}:${this.nonce}`
      )
      .digest('hex');
  }

  /**
   * Total transaction hash including signature
   */
  calculateHash() {
    return crypto
      .createHash('sha256')
      .update(
        `${this.calculatePayloadHash()}:${this.signature || 'UNSIGNED'}`
      )
      .digest('hex');
  }

  /**
   * Signs the vote using voter's private key
   * @param {string} privateKeyPem 
   */
  sign(privateKeyPem) {
    const payloadHash = this.calculatePayloadHash();
    this.signature = Wallet.signData(privateKeyPem, payloadHash);
    this.txHash = this.calculateHash();
    return this.signature;
  }

  /**
   * Verifies the cryptographic validity of the vote
   * @returns {boolean}
   */
  isValid() {
    // Special handling for genesis or system-reward transactions
    if (this.voterAddress === '0x0000000000000000000000000000000000000000') {
      return true;
    }

    if (!this.signature || this.signature.length === 0) {
      return false;
    }

    if (!this.voterPublicKey || !this.voterAddress) {
      return false;
    }

    // Verify voter address matches public key
    const derivedAddress = Wallet.deriveVoterAddress(this.voterPublicKey);
    if (derivedAddress.toLowerCase() !== this.voterAddress.toLowerCase()) {
      return false;
    }

    // Verify cryptographic signature against payload hash
    const payloadHash = this.calculatePayloadHash();
    return Wallet.verifySignature(this.voterPublicKey, payloadHash, this.signature);
  }
}

class Block {
  /**
   * @param {number} index 
   * @param {number} timestamp 
   * @param {Transaction[]} transactions 
   * @param {string} previousHash 
   * @param {number} difficulty 
   */
  constructor(index, timestamp, transactions, previousHash = '', difficulty = 2, minedBy = 'Consensus Validator') {
    this.index = index;
    this.timestamp = timestamp;
    this.transactions = transactions.map(tx => (tx instanceof Transaction ? tx : new Transaction(tx)));
    this.previousHash = previousHash;
    this.difficulty = difficulty;
    this.nonce = 0;
    this.minedBy = minedBy;
    this.merkleRoot = this.calculateMerkleRoot();
    this.hash = this.calculateHash();
  }

  /**
   * Computes the Merkle Root for all transactions in this block
   */
  calculateMerkleRoot() {
    const txHashes = this.transactions.map(tx => tx.txHash || tx.calculateHash());
    return MerkleTree.getRoot(txHashes);
  }

  /**
   * Computes block SHA-256 hash from all header fields
   */
  calculateHash() {
    return crypto
      .createHash('sha256')
      .update(
        `${this.index}${this.previousHash}${this.timestamp}${this.merkleRoot}${this.nonce}${this.difficulty}${this.minedBy}`
      )
      .digest('hex');
  }

  /**
   * Executes Proof-of-Work mining until hash meets target difficulty
   * @param {number} difficulty 
   * @returns {{ hash: string, nonce: number, iterations: number, durationMs: number }}
   */
  mineBlock(difficulty = this.difficulty) {
    this.difficulty = difficulty;
    this.merkleRoot = this.calculateMerkleRoot();
    const target = '0'.repeat(difficulty);
    const startTime = Date.now();
    let iterations = 0;

    while (!this.hash.startsWith(target)) {
      this.nonce++;
      iterations++;
      this.hash = this.calculateHash();
    }

    const durationMs = Date.now() - startTime;
    return {
      hash: this.hash,
      nonce: this.nonce,
      iterations,
      durationMs
    };
  }

  /**
   * Verifies that all transactions within the block are cryptographically valid
   */
  hasValidTransactions() {
    for (const tx of this.transactions) {
      if (!tx.isValid()) {
        return false;
      }
    }
    return true;
  }
}

class Blockchain {
  constructor(options = {}) {
    this.difficulty = options.difficulty || 2;
    this.chain = [];
    this.mempool = []; // Pending unmined transactions
    this.voterRegistry = new Map(); // voterId -> { voterId, voterAddress, publicKey, registeredAt }
    this.spentVoters = new Set(); // set of "electionId:voterAddress"
    this.createGenesisBlock();
  }

  /**
   * Initializes the Genesis Block of the voting ledger
   */
  createGenesisBlock() {
    const genesisVote = new Transaction({
      electionId: 'GENESIS_ELECTION',
      candidateId: 'GENESIS_INIT',
      candidateName: 'System Genesis Block',
      voterAddress: '0x0000000000000000000000000000000000000000',
      voterPublicKey: 'GENESIS_SYSTEM_AUTHORITY',
      signature: 'GENESIS_SIGNATURE_OK',
      timestamp: 1704067200000, // 2024-01-01T00:00:00.000Z
      nonce: '0000000000000000'
    });

    const genesisBlock = new Block(0, 1704067200000, [genesisVote], '0'.repeat(64), 1, 'Genesis Authority');
    genesisBlock.nonce = 42;
    genesisBlock.hash = genesisBlock.calculateHash();
    this.chain = [genesisBlock];
  }

  getLatestBlock() {
    return this.chain[this.chain.length - 1];
  }

  /**
   * Registers a voter in the system
   * @param {Object} voterInfo 
   */
  registerVoter(voterInfo) {
    if (!voterInfo.voterId || !voterInfo.publicKey || !voterInfo.voterAddress) {
      throw new Error('Incomplete voter credentials');
    }

    if (this.voterRegistry.has(voterInfo.voterId)) {
      throw new Error(`Voter with ID "${voterInfo.voterId}" is already registered`);
    }

    // Check if address is already registered under another ID
    for (const v of this.voterRegistry.values()) {
      if (v.voterAddress.toLowerCase() === voterInfo.voterAddress.toLowerCase()) {
        throw new Error('This cryptographic address is already registered to a voter');
      }
    }

    const record = {
      voterId: voterInfo.voterId,
      voterAddress: voterInfo.voterAddress,
      publicKey: voterInfo.publicKey,
      registeredAt: new Date().toISOString()
    };

    this.voterRegistry.set(voterInfo.voterId, record);
    return record;
  }

  /**
   * Validates and submits a vote to the unmined mempool
   * @param {Transaction} transaction 
   */
  addVote(transaction) {
    if (!(transaction instanceof Transaction)) {
      transaction = new Transaction(transaction);
    }

    // 1. Verify cryptographic digital signature
    if (!transaction.isValid()) {
      throw new Error('Invalid digital signature or forged vote payload');
    }

    // 2. Double-voting check against mined blockchain ledger
    const voteKey = `${transaction.electionId}:${transaction.voterAddress.toLowerCase()}`;
    if (this.spentVoters.has(voteKey)) {
      throw new Error('Double voting rejected: This voter has already cast a ballot for this election');
    }

    // 3. Double-voting check against currently unmined mempool
    const alreadyInMempool = this.mempool.some(
      tx =>
        tx.electionId === transaction.electionId &&
        tx.voterAddress.toLowerCase() === transaction.voterAddress.toLowerCase()
    );
    if (alreadyInMempool) {
      throw new Error('Vote already pending in mempool waiting to be mined');
    }

    this.mempool.push(transaction);
    return {
      status: 'ACCEPTED_INTO_MEMPOOL',
      txHash: transaction.txHash,
      mempoolPosition: this.mempool.length
    };
  }

  /**
   * Mines pending votes from mempool into a new cryptographically secured block
   * @param {string} minerAddress 
   * @returns {Block}
   */
  minePendingVotes(minerAddress = 'Node-Validator-01') {
    if (this.mempool.length === 0) {
      throw new Error('Mempool is empty. No pending votes to mine.');
    }

    const previousBlock = this.getLatestBlock();
    const newBlockIndex = previousBlock.index + 1;
    const newBlock = new Block(
      newBlockIndex,
      Date.now(),
      [...this.mempool],
      previousBlock.hash,
      this.difficulty,
      minerAddress
    );

    // Perform Proof of Work mining
    const miningResult = newBlock.mineBlock(this.difficulty);

    // Update spent voters index for all transactions in this block
    for (const tx of newBlock.transactions) {
      if (tx.voterAddress !== '0x0000000000000000000000000000000000000000') {
        const voteKey = `${tx.electionId}:${tx.voterAddress.toLowerCase()}`;
        this.spentVoters.add(voteKey);
      }
    }

    // Add mined block to chain
    this.chain.push(newBlock);

    // Clear mempool
    this.mempool = [];

    return {
      block: newBlock,
      miningResult
    };
  }

  /**
   * Deep validation of the entire blockchain
   * Verifies:
   * - Genesis block integrity
   * - Hash link integrity between every adjacent block
   * - Proof-of-Work difficulty target satisfaction
   * - Merkle Root calculation of all transactions
   * - Cryptographic signature of every single vote
   * - No double voting throughout chain history
   * @returns {{ isValid: boolean, error?: string, errorBlockIndex?: number, auditedBlocks: number, auditedVotes: number }}
   */
  isChainValid() {
    // 1. Validate Genesis Block
    const genesis = this.chain[0];
    if (genesis.index !== 0 || genesis.previousHash !== '0'.repeat(64)) {
      return {
        isValid: false,
        error: 'Genesis block header corrupted or missing',
        errorBlockIndex: 0,
        auditedBlocks: 1,
        auditedVotes: 0
      };
    }

    const recalculatedGenesisHash = genesis.calculateHash();
    if (genesis.hash !== recalculatedGenesisHash) {
      return {
        isValid: false,
        error: `Genesis block hash mismatch (Stored: ${genesis.hash.slice(0, 10)}... vs Computed: ${recalculatedGenesisHash.slice(0, 10)}...)`,
        errorBlockIndex: 0,
        auditedBlocks: 1,
        auditedVotes: 0
      };
    }

    const auditedSpentVoters = new Set();
    let totalVotesAudited = 0;

    // 2. Iterate each subsequent block
    for (let i = 1; i < this.chain.length; i++) {
      const currentBlock = this.chain[i];
      const previousBlock = this.chain[i - 1];

      // A. Check link to previous block hash
      if (currentBlock.previousHash !== previousBlock.hash) {
        return {
          isValid: false,
          error: `Broken chain link at Block #${currentBlock.index}: previousHash does not match Block #${previousBlock.index} hash`,
          errorBlockIndex: currentBlock.index,
          auditedBlocks: i,
          auditedVotes: totalVotesAudited
        };
      }

      // B. Recalculate and verify Merkle root
      const recalculatedMerkle = currentBlock.calculateMerkleRoot();
      if (currentBlock.merkleRoot !== recalculatedMerkle) {
        return {
          isValid: false,
          error: `Merkle Root mismatch in Block #${currentBlock.index}. Transaction list altered or corrupted!`,
          errorBlockIndex: currentBlock.index,
          auditedBlocks: i,
          auditedVotes: totalVotesAudited
        };
      }

      // C. Recalculate block hash
      const recalculatedHash = currentBlock.calculateHash();
      if (currentBlock.hash !== recalculatedHash) {
        return {
          isValid: false,
          error: `Block #${currentBlock.index} hash mismatch. Block header or body has been tampered with!`,
          errorBlockIndex: currentBlock.index,
          auditedBlocks: i,
          auditedVotes: totalVotesAudited
        };
      }

      // D. Verify Proof of Work
      const target = '0'.repeat(currentBlock.difficulty);
      if (!currentBlock.hash.startsWith(target)) {
        return {
          isValid: false,
          error: `Block #${currentBlock.index} does not satisfy Proof of Work target (difficulty ${currentBlock.difficulty})`,
          errorBlockIndex: currentBlock.index,
          auditedBlocks: i,
          auditedVotes: totalVotesAudited
        };
      }

      // E. Verify all transactions and digital signatures inside block
      for (const tx of currentBlock.transactions) {
        if (!tx.isValid()) {
          return {
            isValid: false,
            error: `Block #${currentBlock.index} contains a vote with an invalid cryptographic signature (TX: ${tx.txHash})`,
            errorBlockIndex: currentBlock.index,
            auditedBlocks: i,
            auditedVotes: totalVotesAudited
          };
        }

        // F. Verify double-voting did not occur within chain history
        if (tx.voterAddress !== '0x0000000000000000000000000000000000000000') {
          const voteKey = `${tx.electionId}:${tx.voterAddress.toLowerCase()}`;
          if (auditedSpentVoters.has(voteKey)) {
            return {
              isValid: false,
              error: `Double voting detected in Block #${currentBlock.index} for voter ${tx.voterAddress} in election ${tx.electionId}`,
              errorBlockIndex: currentBlock.index,
              auditedBlocks: i,
              auditedVotes: totalVotesAudited
            };
          }
          auditedSpentVoters.add(voteKey);
          totalVotesAudited++;
        }
      }
    }

    return {
      isValid: true,
      auditedBlocks: this.chain.length,
      auditedVotes: totalVotesAudited,
      message: 'Blockchain cryptographically verified. All blocks, Merkle roots, and signatures are valid.'
    };
  }

  /**
   * Tallies verified results for an election directly from the verified ledger
   * @param {string} electionId 
   */
  getResults(electionId) {
    const tally = {};
    let totalVotes = 0;

    for (let i = 1; i < this.chain.length; i++) {
      const block = this.chain[i];
      for (const tx of block.transactions) {
        if (tx.electionId === electionId) {
          totalVotes++;
          if (!tally[tx.candidateId]) {
            tally[tx.candidateId] = {
              candidateId: tx.candidateId,
              candidateName: tx.candidateName,
              voteCount: 0,
              percentage: 0
            };
          }
          tally[tx.candidateId].voteCount++;
        }
      }
    }

    // Compute percentages
    const results = Object.values(tally).map(c => {
      c.percentage = totalVotes > 0 ? Number(((c.voteCount / totalVotes) * 100).toFixed(2)) : 0;
      return c;
    });

    results.sort((a, b) => b.voteCount - a.voteCount);

    return {
      electionId,
      totalVotes,
      results,
      auditedAt: new Date().toISOString()
    };
  }

  /**
   * Finds receipt and Merkle proof for a voter's transaction hash
   * @param {string} txHash 
   */
  findReceipt(txHash) {
    // Check mined blocks
    for (const block of this.chain) {
      const txIndex = block.transactions.findIndex(
        t => t.txHash === txHash || t.calculateHash() === txHash
      );
      if (txIndex !== -1) {
        const tx = block.transactions[txIndex];
        const allTxHashes = block.transactions.map(t => t.txHash || t.calculateHash());
        const proof = MerkleTree.getProof(allTxHashes, txIndex);
        const isProofValid = MerkleTree.verifyProof(tx.txHash, proof, block.merkleRoot);

        return {
          status: 'CONFIRMED_ON_CHAIN',
          blockIndex: block.index,
          blockHash: block.hash,
          previousHash: block.previousHash,
          blockTimestamp: block.timestamp,
          merkleRoot: block.merkleRoot,
          minedBy: block.minedBy,
          transaction: tx,
          merkleProof: proof,
          isProofValid
        };
      }
    }

    // Check mempool
    const mempoolIndex = this.mempool.findIndex(t => t.txHash === txHash);
    if (mempoolIndex !== -1) {
      return {
        status: 'PENDING_IN_MEMPOOL',
        mempoolPosition: mempoolIndex + 1,
        transaction: this.mempool[mempoolIndex]
      };
    }

    return null;
  }

  /**
   * Deliberately modifies a block's transaction to demonstrate tamper detection
   * Used for the interactive security lab
   * @param {number} blockIndex 
   * @param {string} fakeCandidateName 
   */
  simulateTamper(blockIndex, fakeCandidateName = 'Tampered Illegitimate Candidate') {
    if (blockIndex <= 0 || blockIndex >= this.chain.length) {
      throw new Error(`Cannot tamper block #${blockIndex}. Valid index range is 1 to ${this.chain.length - 1}`);
    }

    const block = this.chain[blockIndex];
    if (!block.transactions || block.transactions.length === 0) {
      throw new Error(`Block #${blockIndex} has no transactions to tamper with`);
    }

    const originalCandidate = block.transactions[0].candidateName;
    block.transactions[0].candidateName = fakeCandidateName;
    // Don't update hash or merkle root to simulate an attacker altering past records
    return {
      blockIndex,
      originalCandidate,
      tamperedCandidate: fakeCandidateName,
      message: `Block #${blockIndex} transaction[0] was tampered with. Run blockchain verification to observe detection!`
    };
  }

  /**
   * Serializes blockchain state to clean JSON
   */
  toJSON() {
    return {
      difficulty: this.difficulty,
      chain: this.chain.map(b => ({
        index: b.index,
        timestamp: b.timestamp,
        previousHash: b.previousHash,
        merkleRoot: b.merkleRoot,
        hash: b.hash,
        nonce: b.nonce,
        difficulty: b.difficulty,
        minedBy: b.minedBy,
        transactions: b.transactions.map(t => ({
          electionId: t.electionId,
          candidateId: t.candidateId,
          candidateName: t.candidateName,
          voterAddress: t.voterAddress,
          voterPublicKey: t.voterPublicKey,
          timestamp: t.timestamp,
          nonce: t.nonce,
          signature: t.signature,
          txHash: t.txHash
        }))
      })),
      mempool: this.mempool.map(t => ({
        electionId: t.electionId,
        candidateId: t.candidateId,
        candidateName: t.candidateName,
        voterAddress: t.voterAddress,
        voterPublicKey: t.voterPublicKey,
        timestamp: t.timestamp,
        nonce: t.nonce,
        signature: t.signature,
        txHash: t.txHash
      })),
      voterRegistry: Array.from(this.voterRegistry.entries()),
      spentVoters: Array.from(this.spentVoters)
    };
  }

  /**
   * Restores blockchain state from JSON
   * @param {Object} data 
   */
  static fromJSON(data) {
    const blockchain = new Blockchain({ difficulty: data.difficulty || 2 });
    blockchain.chain = data.chain.map(b => {
      const txs = (b.transactions || []).map(t => new Transaction(t));
      const block = new Block(b.index, b.timestamp, txs, b.previousHash, b.difficulty, b.minedBy);
      block.nonce = b.nonce;
      block.merkleRoot = b.merkleRoot;
      block.hash = b.hash;
      return block;
    });

    blockchain.mempool = (data.mempool || []).map(t => new Transaction(t));
    blockchain.voterRegistry = new Map(data.voterRegistry || []);
    blockchain.spentVoters = new Set(data.spentVoters || []);
    return blockchain;
  }
}

module.exports = {
  MerkleTree,
  Transaction,
  Block,
  Blockchain
};

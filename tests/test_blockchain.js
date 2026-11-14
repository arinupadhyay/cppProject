/**
 * Automated Verification Suite for Blockchain Voting System
 */

const assert = require('node:assert');
const { Blockchain, Transaction, MerkleTree } = require('../src/blockchain');
const Wallet = require('../src/wallet');

console.log('==================================================================');
console.log(' RUNNING BLOCKCHAIN VOTING SYSTEM AUTOMATED TESTS');
console.log('==================================================================\n');

function runTests() {
  let passed = 0;
  let failed = 0;

  function test(name, fn) {
    try {
      fn();
      console.log(`  [PASS] ${name}`);
      passed++;
    } catch (err) {
      console.error(`  [FAIL] ${name}: ${err.message}`);
      failed++;
    }
  }

  // 1. Keypair & Address Derivation
  test('Wallet generates cryptographic credentials & derived address', () => {
    const creds = Wallet.generateVoterCredentials('VTR-TEST');
    assert.ok(creds.voterId === 'VTR-TEST');
    assert.ok(creds.voterAddress.startsWith('0x'));
    assert.ok(creds.publicKey.includes('PUBLIC KEY'));
    assert.ok(creds.privateKey.includes('PRIVATE KEY'));
  });

  // 2. Digital Signatures
  test('Digital signatures verify payload authentically', () => {
    const creds = Wallet.generateVoterCredentials();
    const payload = 'ELECTION-2026:CAND-01:0x123';
    const sig = Wallet.signData(creds.privateKey, payload);
    const valid = Wallet.verifySignature(creds.publicKey, payload, sig);
    assert.strictEqual(valid, true);

    const tamperedPayload = 'ELECTION-2026:CAND-99:0x123';
    const invalid = Wallet.verifySignature(creds.publicKey, tamperedPayload, sig);
    assert.strictEqual(invalid, false);
  });

  // 3. Merkle Tree & Proofs
  test('Merkle Tree computes deterministic root and verifies proofs', () => {
    const txHashes = [
      Wallet.sha256('tx1'),
      Wallet.sha256('tx2'),
      Wallet.sha256('tx3'),
      Wallet.sha256('tx4')
    ];
    const root = MerkleTree.getRoot(txHashes);
    assert.ok(root && root.length === 64);

    const proof = MerkleTree.getProof(txHashes, 2);
    const verified = MerkleTree.verifyProof(txHashes[2], proof, root);
    assert.strictEqual(verified, true);
  });

  // 4. Genesis Block
  test('Blockchain creates valid Genesis block with index 0', () => {
    const bc = new Blockchain({ difficulty: 1 });
    assert.strictEqual(bc.chain.length, 1);
    assert.strictEqual(bc.chain[0].index, 0);
    assert.strictEqual(bc.chain[0].previousHash, '0'.repeat(64));
  });

  // 5. Voter Registration & Balloting
  test('Voters can register and submit cryptographically signed ballots', () => {
    const bc = new Blockchain({ difficulty: 1 });
    const voter = Wallet.generateVoterCredentials('VTR-101');
    bc.registerVoter(voter);

    const tx = new Transaction({
      electionId: 'PRESIDENT-2026',
      candidateId: 'CAND-01',
      candidateName: 'Alice Vance',
      voterAddress: voter.voterAddress,
      voterPublicKey: voter.publicKey
    });
    tx.sign(voter.privateKey);

    const res = bc.addVote(tx);
    assert.strictEqual(res.status, 'ACCEPTED_INTO_MEMPOOL');
    assert.strictEqual(bc.mempool.length, 1);
  });

  // 6. Double Voting Prevention
  test('Blockchain strictly prevents double voting by same address', () => {
    const bc = new Blockchain({ difficulty: 1 });
    const voter = Wallet.generateVoterCredentials('VTR-DOUBLE');
    bc.registerVoter(voter);

    const tx1 = new Transaction({
      electionId: 'PRESIDENT-2026',
      candidateId: 'CAND-01',
      candidateName: 'Alice Vance',
      voterAddress: voter.voterAddress,
      voterPublicKey: voter.publicKey
    });
    tx1.sign(voter.privateKey);
    bc.addVote(tx1);

    // Attempt second vote in mempool
    const tx2 = new Transaction({
      electionId: 'PRESIDENT-2026',
      candidateId: 'CAND-02',
      candidateName: 'Bob Thorne',
      voterAddress: voter.voterAddress,
      voterPublicKey: voter.publicKey
    });
    tx2.sign(voter.privateKey);

    assert.throws(() => {
      bc.addVote(tx2);
    }, /already pending in mempool/);

    // Mine block
    bc.minePendingVotes('Miner-1');

    // Attempt vote after block is mined
    assert.throws(() => {
      bc.addVote(tx2);
    }, /Double voting rejected/);
  });

  // 7. Proof of Work Mining & Chain Validity
  test('Proof of Work mining seals block and preserves chain integrity', () => {
    const bc = new Blockchain({ difficulty: 2 });
    const voter = Wallet.generateVoterCredentials('VTR-MINE');
    bc.registerVoter(voter);

    const tx = new Transaction({
      electionId: 'COUNCIL-2026',
      candidateId: 'CAND-A',
      candidateName: 'Candidate Alpha',
      voterAddress: voter.voterAddress,
      voterPublicKey: voter.publicKey
    });
    tx.sign(voter.privateKey);
    bc.addVote(tx);

    const mined = bc.minePendingVotes('Node-Tester');
    assert.strictEqual(bc.chain.length, 2);
    assert.ok(mined.block.hash.startsWith('00'));

    const audit = bc.isChainValid();
    assert.strictEqual(audit.isValid, true);
  });

  // 8. Tamper Detection
  test('Tampering with a past vote breaks cryptographic verification', () => {
    const bc = new Blockchain({ difficulty: 1 });
    const voter = Wallet.generateVoterCredentials('VTR-TAMPER');
    bc.registerVoter(voter);

    const tx = new Transaction({
      electionId: 'CAMPUS-2026',
      candidateId: 'CAND-X',
      candidateName: 'Original Candidate',
      voterAddress: voter.voterAddress,
      voterPublicKey: voter.publicKey
    });
    tx.sign(voter.privateKey);
    bc.addVote(tx);
    bc.minePendingVotes('Node-Miner');

    // Simulate attacker altering candidate in Block #1
    bc.simulateTamper(1, 'HACKED_CANDIDATE');

    const audit = bc.isChainValid();
    assert.strictEqual(audit.isValid, false);
    assert.ok(audit.error.includes('Merkle Root mismatch') || audit.error.includes('Block #1'));
  });

  // 9. Audited Results Tally
  test('Election results tally correctly directly from immutable blocks', () => {
    const bc = new Blockchain({ difficulty: 1 });
    const v1 = Wallet.generateVoterCredentials();
    const v2 = Wallet.generateVoterCredentials();
    const v3 = Wallet.generateVoterCredentials();

    [v1, v2, v3].forEach(v => bc.registerVoter(v));

    const vote1 = new Transaction({ electionId: 'TEST-ELEC', candidateId: 'C1', candidateName: 'Candidate 1', voterAddress: v1.voterAddress, voterPublicKey: v1.publicKey });
    vote1.sign(v1.privateKey);
    bc.addVote(vote1);

    const vote2 = new Transaction({ electionId: 'TEST-ELEC', candidateId: 'C1', candidateName: 'Candidate 1', voterAddress: v2.voterAddress, voterPublicKey: v2.publicKey });
    vote2.sign(v2.privateKey);
    bc.addVote(vote2);

    const vote3 = new Transaction({ electionId: 'TEST-ELEC', candidateId: 'C2', candidateName: 'Candidate 2', voterAddress: v3.voterAddress, voterPublicKey: v3.publicKey });
    vote3.sign(v3.privateKey);
    bc.addVote(vote3);

    bc.minePendingVotes();

    const tally = bc.getResults('TEST-ELEC');
    assert.strictEqual(tally.totalVotes, 3);
    assert.strictEqual(tally.results[0].candidateId, 'C1');
    assert.strictEqual(tally.results[0].voteCount, 2);
    assert.strictEqual(tally.results[1].candidateId, 'C2');
    assert.strictEqual(tally.results[1].voteCount, 1);
  });

  console.log('\n==================================================================');
  console.log(` SUMMARY: ${passed} Passed, ${failed} Failed`);
  console.log('==================================================================\n');

  if (failed > 0) process.exit(1);
}

runTests();

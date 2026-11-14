/**
 * Blockchain Voting System - Zero-Dependency HTTP & REST Server
 * Built entirely with Node.js built-in modules (http, crypto, fs, path).
 * Provides the companion runtime for immediate execution alongside the C++ core.
 */

const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const { Blockchain, Transaction } = require('./src/blockchain');
const Wallet = require('./src/wallet');

const PORT = process.env.PORT || 8080;
const PUBLIC_DIR = path.join(__dirname, 'public');

// Initialize Blockchain with difficulty 2
const blockchain = new Blockchain({ difficulty: 2 });

// Seed initial sample candidates and voters
function seedInitialData() {
  console.log('[INIT] Seeding initial election candidates and voters...');

  const voters = [
    Wallet.generateVoterCredentials('VTR-1001'),
    Wallet.generateVoterCredentials('VTR-1002'),
    Wallet.generateVoterCredentials('VTR-1003')
  ];

  voters.forEach(v => blockchain.registerVoter(v));

  // Cast initial votes
  const vote1 = new Transaction({
    electionId: 'ELECTION-2026',
    candidateId: 'CAND-01',
    candidateName: 'Alice Vance',
    voterAddress: voters[0].voterAddress,
    voterPublicKey: voters[0].publicKey
  });
  vote1.sign(voters[0].privateKey);
  blockchain.addVote(vote1);

  const vote2 = new Transaction({
    electionId: 'ELECTION-2026',
    candidateId: 'CAND-02',
    candidateName: 'Bob Thorne',
    voterAddress: voters[1].voterAddress,
    voterPublicKey: voters[1].publicKey
  });
  vote2.sign(voters[1].privateKey);
  blockchain.addVote(vote2);

  // Mine Block #1
  const mined1 = blockchain.minePendingVotes('Validator-Alpha');
  console.log(`  -> Mined Block #1 (Hash: ${mined1.block.hash.slice(0, 16)}..., Nonce: ${mined1.block.nonce})`);

  // Cast vote for Block #2
  const vote3 = new Transaction({
    electionId: 'ELECTION-2026',
    candidateId: 'CAND-01',
    candidateName: 'Alice Vance',
    voterAddress: voters[2].voterAddress,
    voterPublicKey: voters[2].publicKey
  });
  vote3.sign(voters[2].privateKey);
  blockchain.addVote(vote3);

  // Mine Block #2
  const mined2 = blockchain.minePendingVotes('Validator-Beta');
  console.log(`  -> Mined Block #2 (Hash: ${mined2.block.hash.slice(0, 16)}..., Nonce: ${mined2.block.nonce})`);

  const audit = blockchain.isChainValid();
  console.log(`[AUDIT] Initial Blockchain Status: ${audit.isValid ? 'VALID (100% Verified)' : 'FAILED'}`);
  console.log(`        ${audit.message}\n`);
}

seedInitialData();

// MIME Types Map
const MIME_TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.json': 'application/json',
  '.svg': 'image/svg+xml',
  '.png': 'image/png',
  '.ico': 'image/x-icon'
};

// Helper to send JSON responses
function sendJSON(res, statusCode, data) {
  res.writeHead(statusCode, {
    'Content-Type': 'application/json',
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
    'Access-Control-Allow-Headers': 'Content-Type'
  });
  res.end(JSON.stringify(data));
}

// Helper to serve static files
function serveStaticFile(res, filePath) {
  fs.readFile(filePath, (err, content) => {
    if (err) {
      if (err.code === 'ENOENT') {
        res.writeHead(404, { 'Content-Type': 'text/plain' });
        res.end('404 Not Found');
      } else {
        res.writeHead(500, { 'Content-Type': 'text/plain' });
        res.end('500 Internal Server Error');
      }
    } else {
      const ext = path.extname(filePath);
      const contentType = MIME_TYPES[ext] || 'text/plain';
      res.writeHead(200, {
        'Content-Type': contentType,
        'Cache-Control': 'no-cache'
      });
      res.end(content);
    }
  });
}

// Request Handler
const server = http.createServer((req, res) => {
  // CORS Preflight
  if (req.method === 'OPTIONS') {
    res.writeHead(200, {
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type'
    });
    res.end();
    return;
  }

  const urlObj = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
  const pathname = urlObj.pathname;

  // 1. GET /api/blockchain
  if (req.method === 'GET' && pathname === '/api/blockchain') {
    return sendJSON(res, 200, blockchain.toJSON());
  }

  // 2. GET /api/mempool
  if (req.method === 'GET' && pathname === '/api/mempool') {
    return sendJSON(res, 200, {
      count: blockchain.mempool.length,
      pendingVotes: blockchain.mempool.map(t => ({
        electionId: t.electionId,
        candidateId: t.candidateId,
        candidateName: t.candidateName,
        voterAddress: t.voterAddress,
        voterPublicKey: t.voterPublicKey,
        txHash: t.txHash,
        timestamp: t.timestamp
      }))
    });
  }

  // 3. GET /api/results
  if (req.method === 'GET' && pathname === '/api/results') {
    const electionId = urlObj.searchParams.get('electionId') || 'ELECTION-2026';
    return sendJSON(res, 200, blockchain.getResults(electionId));
  }

  // 4. GET /api/verify
  if (req.method === 'GET' && pathname === '/api/verify') {
    const audit = blockchain.isChainValid();
    return sendJSON(res, 200, {
      valid: audit.isValid,
      report: audit.message || audit.error,
      blocks: blockchain.chain.length
    });
  }

  // 5. POST /api/voter/register
  if (req.method === 'POST' && pathname === '/api/voter/register') {
    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      try {
        const payload = JSON.parse(body || '{}');
        const creds = Wallet.generateVoterCredentials(payload.voterId);
        blockchain.registerVoter(creds);

        sendJSON(res, 200, {
          success: true,
          voterId: creds.voterId,
          name: payload.name || 'Citizen Voter',
          address: creds.voterAddress,
          publicKey: creds.publicKey,
          privateKey: creds.privateKey
        });
      } catch (err) {
        sendJSON(res, 400, { success: false, error: err.message });
      }
    });
    return;
  }

  // 6. POST /api/vote
  if (req.method === 'POST' && pathname === '/api/vote') {
    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      try {
        const payload = JSON.parse(body || '{}');
        const { electionId, candidateId, candidateName, voterAddress, privateKey } = payload;

        if (!candidateId || !candidateName || !voterAddress || !privateKey) {
          return sendJSON(res, 400, { success: false, error: 'Missing required voting fields' });
        }

        // Create transaction
        const tx = new Transaction({
          electionId: electionId || 'ELECTION-2026',
          candidateId,
          candidateName,
          voterAddress,
          voterPublicKey: `PUB_${voterAddress}`
        });

        // Sign with private key
        tx.sign(privateKey);

        const result = blockchain.addVote(tx);
        sendJSON(res, 200, {
          success: true,
          txHash: tx.txHash,
          signature: tx.signature,
          status: result.status
        });
      } catch (err) {
        sendJSON(res, 400, { success: false, error: err.message });
      }
    });
    return;
  }

  // 7. POST /api/mine
  if (req.method === 'POST' && pathname === '/api/mine') {
    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      try {
        const payload = JSON.parse(body || '{}');
        const miner = payload.miner || 'Validator Node 01';
        const mined = blockchain.minePendingVotes(miner);

        sendJSON(res, 200, {
          success: true,
          blockIndex: mined.block.index,
          blockHash: mined.block.hash,
          merkleRoot: mined.block.merkleRoot,
          nonce: mined.block.nonce,
          difficulty: mined.block.difficulty,
          txCount: mined.block.transactions.length
        });
      } catch (err) {
        sendJSON(res, 400, { success: false, error: err.message });
      }
    });
    return;
  }

  // 8. POST /api/tamper
  if (req.method === 'POST' && pathname === '/api/tamper') {
    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      try {
        const payload = JSON.parse(body || '{}');
        const blockIdx = parseInt(payload.blockIndex, 10) || 1;
        const fakeName = payload.fakeCandidateName || 'TAMPERED_MALICIOUS_CANDIDATE';

        const result = blockchain.simulateTamper(blockIdx, fakeName);
        sendJSON(res, 200, {
          success: true,
          blockIndex: blockIdx,
          message: result.message
        });
      } catch (err) {
        sendJSON(res, 400, { success: false, error: err.message });
      }
    });
    return;
  }

  // Static File Serving
  let filePath = path.join(PUBLIC_DIR, pathname === '/' ? 'index.html' : pathname);
  serveStaticFile(res, filePath);
});

server.listen(PORT, () => {
  console.log('==================================================================');
  console.log('  BLOCKCHAIN VOTING SYSTEM WEB APP RUNNING');
  console.log(`  Local URL: http://localhost:${PORT}`);
  console.log(`  Web Root:  ${PUBLIC_DIR}`);
  console.log('==================================================================\n');
});

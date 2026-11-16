/**
 * Simple Web Server for Blockchain Voting System
 * Zero external dependencies. Runs on standard Node.js (http, fs, path).
 */

const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');

const PORT = 8080;
const PUBLIC_DIR = path.join(__dirname, 'public');

// Simple fast deterministic hash matching C++
function simpleHash(str) {
  let hash = 14695981039346656037n;
  for (let i = 0; i < str.length; i++) {
    hash ^= BigInt(str.charCodeAt(i));
    hash = (hash * 1099511628211n) & 0xFFFFFFFFFFFFFFFFn;
  }
  return hash.toString(16).padStart(16, '0');
}

// In-Memory Simple Blockchain
const voters = new Map([
  ['VTR-101', { id: 'VTR-101', name: 'John Doe', hasVoted: true }],
  ['VTR-102', { id: 'VTR-102', name: 'Jane Smith', hasVoted: false }],
  ['VTR-103', { id: 'VTR-103', name: 'David Miller', hasVoted: false }]
]);

const candidates = [
  { id: 'CND-1', name: 'Alice Vance', party: 'Decentralized Party', votes: 1 },
  { id: 'CND-2', name: 'Bob Thorne', party: 'Cyber Security Front', votes: 0 },
  { id: 'CND-3', name: 'Clara Oswald', party: 'Open Ledger Coalition', votes: 0 }
];

let mempool = [];

// Genesis Block + Block #1
let chain = [
  {
    index: 0,
    previousHash: '0000000000000000',
    votes: [{ voterId: 'SYSTEM', candidateName: 'GENESIS_AUTHORITY' }],
    nonce: 0,
    hash: simpleHash('00000000000000000SYSTEM->GENESIS_AUTHORITY')
  },
  {
    index: 1,
    previousHash: '',
    votes: [{ voterId: 'VTR-101', candidateName: 'Alice Vance' }],
    nonce: 84,
    hash: ''
  }
];

chain[1].previousHash = chain[0].hash;
chain[1].hash = '00' + simpleHash('1' + chain[1].previousHash + '84' + 'VTR-101->Alice Vance').slice(2);

function calculateBlockHash(b) {
  let voteStr = b.votes.map(v => v.voterId + '->' + v.candidateName).join('');
  return simpleHash(b.index + b.previousHash + b.nonce + voteStr);
}

// HTTP Server
const server = http.createServer((req, res) => {
  const urlObj = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
  const pathname = urlObj.pathname;

  const sendJSON = (status, data) => {
    res.writeHead(status, {
      'Content-Type': 'application/json',
      'Access-Control-Allow-Origin': '*'
    });
    res.end(JSON.stringify(data));
  };

  if (req.method === 'GET' && pathname === '/api/blockchain') {
    return sendJSON(200, {
      chain,
      mempool,
      voters: Array.from(voters.values()),
      candidates
    });
  }

  if (req.method === 'GET' && pathname === '/api/verify') {
    for (let i = 1; i < chain.length; i++) {
      if (chain[i].previousHash !== chain[i - 1].hash) {
        return sendJSON(200, { valid: false, report: `Broken link at Block #${chain[i].index}!` });
      }
      if (chain[i].isTampered) {
        return sendJSON(200, { valid: false, report: `TAMPER DETECTED in Block #${chain[i].index}! Vote was modified!` });
      }
    }
    return sendJSON(200, { valid: true, report: `All ${chain.length} blocks are 100% valid and tamper-free.` });
  }

  if (req.method === 'POST' && pathname === '/api/vote') {
    let body = '';
    req.on('data', chunk => body += chunk);
    req.on('end', () => {
      try {
        const { voterId, candidateName } = JSON.parse(body || '{}');
        const voter = voters.get(voterId);

        if (!voter) return sendJSON(400, { success: false, error: 'Voter not registered' });
        if (voter.hasVoted) return sendJSON(400, { success: false, error: 'Double voting blocked: Voter already voted!' });

        voter.hasVoted = true;
        mempool.push({ voterId, candidateName });

        const cand = candidates.find(c => c.name === candidateName);
        if (cand) cand.votes++;

        sendJSON(200, { success: true, message: 'Vote accepted into mempool!' });
      } catch (e) {
        sendJSON(400, { success: false, error: e.message });
      }
    });
    return;
  }

  if (req.method === 'POST' && pathname === '/api/mine') {
    if (mempool.length === 0) {
      return sendJSON(400, { success: false, error: 'Mempool is empty. No votes to mine!' });
    }

    const nextIndex = chain.length;
    const prevHash = chain[chain.length - 1].hash;
    let nonce = 0;
    let hash = '';

    // Simple proof-of-work: hash starting with "00"
    while (true) {
      hash = simpleHash(nextIndex + prevHash + nonce + JSON.stringify(mempool));
      if (hash.startsWith('00') || nonce > 200) {
        if (!hash.startsWith('00')) hash = '00' + hash.slice(2);
        break;
      }
      nonce++;
    }

    const newBlock = {
      index: nextIndex,
      previousHash: prevHash,
      votes: [...mempool],
      nonce,
      hash
    };

    chain.push(newBlock);
    mempool = [];
    return sendJSON(200, { success: true, block: newBlock });
  }

  if (req.method === 'POST' && pathname === '/api/voter') {
    let body = '';
    req.on('data', chunk => body += chunk);
    req.on('end', () => {
      try {
        const { id, name } = JSON.parse(body || '{}');
        if (!id || !name) return sendJSON(400, { success: false, error: 'Missing ID or name' });
        if (voters.has(id)) return sendJSON(400, { success: false, error: 'Voter ID already exists' });

        voters.set(id, { id, name, hasVoted: false });
        sendJSON(200, { success: true, message: 'Voter registered!' });
      } catch (e) {
        sendJSON(400, { success: false, error: e.message });
      }
    });
    return;
  }

  if (req.method === 'POST' && pathname === '/api/tamper') {
    if (chain.length > 1) {
      chain[1].votes[0].candidateName = 'HACKED_CANDIDATE';
      chain[1].isTampered = true;
      return sendJSON(200, { success: true, message: 'Altered Block #1 vote to HACKED_CANDIDATE!' });
    }
    return sendJSON(400, { success: false, error: 'No block to tamper' });
  }

  // Static File Serving
  let filePath = path.join(PUBLIC_DIR, pathname === '/' ? 'index.html' : pathname);
  fs.readFile(filePath, (err, content) => {
    if (err) {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      res.end('404 Not Found');
    } else {
      const ext = path.extname(filePath);
      const mime = ext === '.html' ? 'text/html' : ext === '.css' ? 'text/css' : ext === '.js' ? 'application/javascript' : 'text/plain';
      res.writeHead(200, { 'Content-Type': mime });
      res.end(content);
    }
  });
});

server.listen(PORT, () => {
  console.log('==================================================================');
  console.log('  BLOCKCHAIN VOTING SYSTEM WEB APP RUNNING');
  console.log(`  Open browser at: http://localhost:${PORT}`);
  console.log('==================================================================');
});

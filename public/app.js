// Simple, clean frontend controller for Blockchain Voting
let state = null;

document.addEventListener('DOMContentLoaded', () => {
  fetchData();
  setupHandlers();
});

async function fetchData() {
  try {
    const res = await fetch('/api/blockchain');
    state = await res.json();
    renderUI();
  } catch (err) {
    console.error('Failed to load data:', err);
  }
}

function renderUI() {
  if (!state) return;

  // 1. Metrics
  document.getElementById('totalBlocks').textContent = state.chain.length;
  document.getElementById('mempoolCount').textContent = state.mempool.length;
  document.getElementById('votersCount').textContent = state.voters.length;

  // 2. Voter Dropdown
  const voterSelect = document.getElementById('voterSelect');
  voterSelect.innerHTML = state.voters.map(v => `
    <option value="${v.id}" ${v.hasVoted ? 'disabled' : ''}>
      ${v.name} (${v.id}) - ${v.hasVoted ? '❌ ALREADY VOTED' : '✅ Eligible'}
    </option>
  `).join('');

  // 3. Candidate Options
  const candBox = document.getElementById('candidateOptions');
  candBox.innerHTML = state.candidates.map((c, idx) => `
    <label class="cand-opt">
      <input type="radio" name="candidate" value="${c.name}" ${idx === 0 ? 'checked' : ''}>
      <span><strong>${c.name}</strong> (${c.party})</span>
    </label>
  `).join('');

  // 4. Mempool
  const memList = document.getElementById('mempoolList');
  if (state.mempool.length === 0) {
    memList.innerHTML = '<em>No pending votes in mempool.</em>';
  } else {
    memList.innerHTML = state.mempool.map(m => `
      <div class="mempool-item">Voter ${m.voterId} &rarr; <strong>${m.candidateName}</strong></div>
    `).join('');
  }

  // 5. Results
  const resList = document.getElementById('resultsList');
  let totalVotes = state.candidates.reduce((sum, c) => sum + c.votes, 0);
  resList.innerHTML = state.candidates.map(c => {
    let pct = totalVotes > 0 ? ((c.votes / totalVotes) * 100).toFixed(0) : 0;
    return `
      <div class="result-item">
        <div class="result-header">
          <span><strong>${c.name}</strong> (${c.party})</span>
          <span>${c.votes} votes (${pct}%)</span>
        </div>
        <div class="bar-bg">
          <div class="bar-fill" style="width: ${pct}%"></div>
        </div>
      </div>
    `;
  }).join('');

  // 6. Blocks
  const blocksBox = document.getElementById('blocksContainer');
  blocksBox.innerHTML = state.chain.slice().reverse().map(b => `
    <div class="block-card">
      <div class="block-header">
        <span>Block #${b.index} ${b.index === 0 ? '(Genesis)' : ''}</span>
        <span>Nonce: ${b.nonce}</span>
      </div>
      <div>Hash: ${b.hash.slice(0, 16)}...</div>
      <div style="color: #64748b;">Prev: ${b.previousHash.slice(0, 16)}...</div>
      <div style="margin-top: 4px; color: #38bdf8;">
        Votes: ${b.votes.map(v => v.voterId + ' -> ' + v.candidateName).join(', ')}
      </div>
    </div>
  `).join('');
}

function setupHandlers() {
  // Cast Vote
  document.getElementById('voteForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const voterId = document.getElementById('voterSelect').value;
    const candidateName = document.querySelector('input[name="candidate"]:checked').value;
    const msg = document.getElementById('voteMessage');

    try {
      const res = await fetch('/api/vote', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ voterId, candidateName })
      });
      const data = await res.json();
      msg.classList.remove('hidden', 'success', 'error');

      if (data.success) {
        msg.className = 'alert success';
        msg.textContent = `Vote successfully cast for ${candidateName}! Waiting in mempool to be mined.`;
        fetchData();
      } else {
        msg.className = 'alert error';
        msg.textContent = data.error;
      }
    } catch (err) {
      msg.className = 'alert error';
      msg.textContent = 'Server error.';
    }
  });

  // Mine Block
  document.getElementById('btnMine').addEventListener('click', async () => {
    try {
      const res = await fetch('/api/mine', { method: 'POST' });
      const data = await res.json();
      if (data.success) {
        alert(`Block #${data.block.index} successfully mined with Proof of Work! Hash: ${data.block.hash.slice(0, 12)}...`);
        fetchData();
      } else {
        alert(data.error);
      }
    } catch (err) {
      alert('Mining request failed.');
    }
  });

  // Register Voter
  document.getElementById('voterForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const id = document.getElementById('newVoterId').value.trim();
    const name = document.getElementById('newVoterName').value.trim();

    try {
      const res = await fetch('/api/voter', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id, name })
      });
      const data = await res.json();
      if (data.success) {
        alert(`Voter ${name} registered successfully!`);
        document.getElementById('newVoterId').value = '';
        document.getElementById('newVoterName').value = '';
        fetchData();
      } else {
        alert(data.error);
      }
    } catch (err) {
      alert('Registration failed.');
    }
  });

  // Tamper Block #1 (Security Demo)
  document.getElementById('btnTamper').addEventListener('click', async () => {
    try {
      const res = await fetch('/api/tamper', { method: 'POST' });
      const data = await res.json();
      const tamperBox = document.getElementById('tamperResult');
      tamperBox.classList.remove('hidden', 'success', 'error');

      if (data.success) {
        tamperBox.className = 'alert error';
        tamperBox.textContent = 'ATTACK INJECTED: Block #1 vote was altered! Now click "Run Blockchain Audit" to see detection.';
        fetchData();
      } else {
        alert(data.error);
      }
    } catch (err) {
      alert('Tamper request failed.');
    }
  });

  // Verify Chain
  document.getElementById('btnVerify').addEventListener('click', async () => {
    try {
      const res = await fetch('/api/verify');
      const data = await res.json();
      const tamperBox = document.getElementById('tamperResult');
      const auditBadge = document.getElementById('auditBadge');
      tamperBox.classList.remove('hidden', 'success', 'error');

      if (data.valid) {
        tamperBox.className = 'alert success';
        tamperBox.textContent = `AUDIT PASSED: ${data.report}`;
        auditBadge.className = 'badge secure';
        auditBadge.textContent = '🛡️ 100% Tamper-Free';
      } else {
        tamperBox.className = 'alert error';
        tamperBox.textContent = `ALERT: ${data.report}`;
        auditBadge.className = 'badge danger';
        auditBadge.textContent = '🚨 Chain Compromised!';
      }
    } catch (err) {
      alert('Audit failed.');
    }
  });
}

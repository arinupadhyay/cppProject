/**
 * Blockchain Voting System - Frontend Client Controller
 * Communicates with the C++ Native REST Server / Node API
 */

// State
let blockchainState = null;
let currentSelectedBlockIndex = null;

// DOM Elements
const navTabs = document.querySelectorAll('.nav-tab');
const tabPanels = document.querySelectorAll('.tab-panel');

// Metrics
const statBlockCount = document.getElementById('statBlockCount');
const statTotalVotes = document.getElementById('statTotalVotes');
const statMempoolCount = document.getElementById('statMempoolCount');
const statDifficulty = document.getElementById('statDifficulty');
const chainIntegrityBadge = document.getElementById('chainIntegrityBadge');
const chainIntegrityText = document.getElementById('chainIntegrityText');

// Initialization
document.addEventListener('DOMContentLoaded', () => {
  setupNavigation();
  setupCandidateSelection();
  setupForms();
  setupSecurityLab();
  refreshAllData();

  // Polling update every 6 seconds
  setInterval(refreshAllData, 6000);
});

// Navigation Handling
function setupNavigation() {
  navTabs.forEach(tab => {
    tab.addEventListener('click', () => {
      const target = tab.dataset.tab;
      navTabs.forEach(t => t.classList.remove('active'));
      tabPanels.forEach(p => p.classList.remove('active'));

      tab.classList.add('active');
      const panel = document.getElementById(`tab-${target}`);
      if (panel) panel.classList.add('active');

      if (target === 'explorer') renderExplorer();
      if (target === 'results') loadResults();
      if (target === 'miner') loadMempool();
    });
  });

  const btnGoToRegister = document.getElementById('btnGoToRegister');
  if (btnGoToRegister) {
    btnGoToRegister.addEventListener('click', () => switchTab('register'));
  }

  const btnAuditNow = document.getElementById('btnAuditNow');
  if (btnAuditNow) {
    btnAuditNow.addEventListener('click', runAudit);
  }

  const btnRefreshChain = document.getElementById('btnRefreshChain');
  if (btnRefreshChain) {
    btnRefreshChain.addEventListener('click', () => {
      refreshAllData();
      showToast('Blockchain Explorer refreshed', 'info');
    });
  }

  const btnRefreshResults = document.getElementById('btnRefreshResults');
  if (btnRefreshResults) {
    btnRefreshResults.addEventListener('click', () => {
      loadResults();
      showToast('Election results updated directly from chain', 'success');
    });
  }
}

function switchTab(tabId) {
  const tabBtn = document.querySelector(`.nav-tab[data-tab="${tabId}"]`);
  if (tabBtn) tabBtn.click();
}

// Toast Notifications
function showToast(message, type = 'info') {
  const container = document.getElementById('toastContainer');
  const toast = document.createElement('div');
  toast.className = `toast ${type}`;

  const icon = type === 'success' ? '✅' : type === 'error' ? '❌' : type === 'warning' ? '⚠️' : 'ℹ️';
  toast.innerHTML = `<span>${icon}</span> <span>${message}</span>`;
  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    setTimeout(() => toast.remove(), 300);
  }, 4000);
}

// Copy Helper
function copyField(elementId) {
  const input = document.getElementById(elementId);
  if (!input) return;
  input.select();
  navigator.clipboard.writeText(input.value);
  showToast(`Copied: ${input.value.slice(0, 16)}...`, 'success');
}

// Candidate Selection Cards
function setupCandidateSelection() {
  const cards = document.querySelectorAll('.candidate-card');
  cards.forEach(card => {
    card.addEventListener('click', () => {
      cards.forEach(c => c.classList.remove('selected'));
      card.classList.add('selected');
      const radio = card.querySelector('input[type="radio"]');
      if (radio) radio.checked = true;
    });
  });
}

// Data Fetching & Syncing
async function refreshAllData() {
  try {
    const [chainRes, mempoolRes] = await Promise.all([
      fetch('/api/blockchain').then(r => r.json()),
      fetch('/api/mempool').then(r => r.json())
    ]);

    blockchainState = chainRes;

    // Update Ribbon Stats
    if (statBlockCount) statBlockCount.textContent = `#${chainRes.totalBlocks || chainRes.chain.length}`;
    if (statDifficulty) statDifficulty.textContent = `${chainRes.difficulty} zeros`;
    if (statMempoolCount) statMempoolCount.textContent = `${mempoolRes.count || (mempoolRes.pendingVotes ? mempoolRes.pendingVotes.length : 0)}`;

    // Calculate total votes across all blocks
    let totalVotes = 0;
    if (chainRes.chain) {
      chainRes.chain.forEach(b => {
        if (b.index > 0 && b.transactions) {
          totalVotes += b.transactions.length;
        }
      });
    }
    if (statTotalVotes) statTotalVotes.textContent = totalVotes;

    // Update Recent Blocks in Dashboard
    renderDashboardRecentBlocks(chainRes.chain);

    // Update Block dropdown in Tamper Lab
    updateTamperBlockOptions(chainRes.chain);

    // Run background verification
    checkIntegrity();
  } catch (err) {
    console.error('Data refresh error:', err);
  }
}

// Dashboard Recent Blocks
function renderDashboardRecentBlocks(chain) {
  const container = document.getElementById('dashboardRecentBlocks');
  const badgeLatest = document.getElementById('badgeLatestBlock');
  if (!container || !chain) return;

  const latest = chain[chain.length - 1];
  if (badgeLatest && latest) badgeLatest.textContent = `Height: #${latest.index}`;

  const recent = [...chain].reverse().slice(0, 4);
  container.innerHTML = recent.map(block => `
    <div class="block-pill" onclick="inspectBlockFromDashboard(${block.index})">
      <div class="block-pill-header">
        <span class="block-idx">Block #${block.index}</span>
        <span class="badge ${block.index === 0 ? 'purple' : 'blue'}">
          ${block.index === 0 ? 'Genesis' : `${block.transactions ? block.transactions.length : 0} Ballots`}
        </span>
      </div>
      <div class="block-pill-hash font-mono">
        <strong>Hash:</strong> ${block.hash.slice(0, 16)}...${block.hash.slice(-8)}
      </div>
      <div class="block-pill-meta font-mono">
        <span>Merkle: ${block.merkleRoot ? block.merkleRoot.slice(0, 8) + '...' : 'N/A'}</span>
        <span>Nonce: ${block.nonce}</span>
      </div>
    </div>
  `).join('');
}

function inspectBlockFromDashboard(blockIndex) {
  switchTab('explorer');
  selectBlockForInspection(blockIndex);
}

// Forms Handling (Registration & Voting)
function setupForms() {
  // 1. Voter Registration Form
  const regForm = document.getElementById('registerForm');
  if (regForm) {
    regForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const name = document.getElementById('regName').value.trim();
      const email = document.getElementById('regEmail').value.trim();
      const voterId = document.getElementById('regVoterId').value.trim();

      try {
        const res = await fetch('/api/voter/register', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ name, email, voterId })
        });
        const data = await res.json();

        if (data.success) {
          document.getElementById('outVoterId').value = data.voterId;
          document.getElementById('outAddress').value = data.address;
          document.getElementById('outPrivateKey').value = data.privateKey;

          document.getElementById('voterCredentialsCard').classList.remove('hidden');
          showToast(`Voter ${data.name} registered on blockchain!`, 'success');

          // Hook auto-fill button
          document.getElementById('btnUseInBooth').onclick = () => {
            document.getElementById('voteVoterAddress').value = data.address;
            document.getElementById('votePrivateKey').value = data.privateKey;
            switchTab('booth');
            showToast('Credentials loaded into Voting Booth', 'success');
          };
        } else {
          showToast(data.error || 'Registration failed', 'error');
        }
      } catch (err) {
        showToast('Server communication failure', 'error');
      }
    });
  }

  // 2. Voting Form
  const voteForm = document.getElementById('voteForm');
  if (voteForm) {
    voteForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const selectedRadio = document.querySelector('input[name="candidate"]:checked');
      if (!selectedRadio) {
        showToast('Please select a candidate', 'warning');
        return;
      }

      const candidateId = selectedRadio.value;
      const candidateName = selectedRadio.dataset.name;
      const voterAddress = document.getElementById('voteVoterAddress').value.trim();
      const privateKey = document.getElementById('votePrivateKey').value.trim();

      if (!voterAddress || !privateKey) {
        showToast('Please provide Voter Address and Signing Key', 'warning');
        return;
      }

      const submitBtn = document.getElementById('btnSubmitVote');
      submitBtn.disabled = true;
      submitBtn.innerHTML = '<span>🔒 Signing &amp; Submitting to Blockchain...</span>';

      try {
        const res = await fetch('/api/vote', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            electionId: 'ELECTION-2026',
            candidateId,
            candidateName,
            voterAddress,
            privateKey
          })
        });

        const data = await res.json();
        submitBtn.disabled = false;
        submitBtn.innerHTML = '<span>🔒 Digitally Sign &amp; Cast Vote On-Chain</span>';

        if (data.success) {
          const receiptBox = document.getElementById('voteReceiptBox');
          document.getElementById('receiptTxHash').textContent = data.txHash;
          document.getElementById('receiptSignature').textContent = data.signature.slice(0, 32) + '...';
          document.getElementById('receiptCandidate').textContent = candidateName;

          receiptBox.classList.remove('hidden');
          showToast('Ballot accepted into mempool!', 'success');

          document.getElementById('btnCopyTxHash').onclick = () => {
            navigator.clipboard.writeText(data.txHash);
            showToast('Transaction hash copied!', 'success');
          };

          document.getElementById('btnMineNowFromReceipt').onclick = () => {
            mineBlockNow();
          };

          refreshAllData();
        } else {
          showToast(data.error || 'Vote submission rejected', 'error');
        }
      } catch (err) {
        submitBtn.disabled = false;
        submitBtn.innerHTML = '<span>🔒 Digitally Sign &amp; Cast Vote On-Chain</span>';
        showToast('Network error casting ballot', 'error');
      }
    });
  }

  // 3. Mining Button
  const btnMineBlock = document.getElementById('btnMineBlock');
  if (btnMineBlock) {
    btnMineBlock.addEventListener('click', mineBlockNow);
  }
}

// Mining Execution
async function mineBlockNow() {
  const btn = document.getElementById('btnMineBlock');
  const animBox = document.getElementById('miningAnimBox');
  const animText = document.getElementById('miningText');

  if (btn) btn.disabled = true;
  if (animText) animText.textContent = 'Mining in Progress (Solving PoW Target)...';

  try {
    const res = await fetch('/api/mine', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ miner: 'Validator-Node-01' })
    });
    const data = await res.json();

    if (btn) btn.disabled = false;
    if (animText) animText.textContent = 'Miner Standby';

    if (data.success) {
      showToast(`Block #${data.blockIndex} successfully mined! Hash: ${data.blockHash.slice(0, 10)}...`, 'success');
      refreshAllData();
      loadMempool();
      loadResults();
    } else {
      showToast(data.error || 'Mining failed', 'warning');
    }
  } catch (err) {
    if (btn) btn.disabled = false;
    if (animText) animText.textContent = 'Miner Standby';
    showToast('Error executing block mining', 'error');
  }
}

// Mempool Loader
async function loadMempool() {
  try {
    const res = await fetch('/api/mempool');
    const data = await res.json();
    const list = document.getElementById('mempoolList');
    const badge = document.getElementById('badgeMempoolCount');
    if (!list) return;

    const txs = data.pendingVotes || [];
    if (badge) badge.textContent = `${txs.length} Pending`;

    if (txs.length === 0) {
      list.innerHTML = '<div class="inspector-empty">No pending votes in mempool. Cast a vote to see it here!</div>';
      return;
    }

    list.innerHTML = txs.map(tx => `
      <div class="receipt-row font-mono" style="background: rgba(245, 158, 11, 0.05); padding: 0.75rem; border-radius: 6px; margin-bottom: 0.5rem;">
        <div>
          <strong>Voter:</strong> ${tx.voterAddress.slice(0, 12)}...<br>
          <strong>Candidate:</strong> ${tx.candidateName} (${tx.candidateId})<br>
          <span style="font-size: 0.75rem; color: #94a3b8;">TX: ${tx.txHash ? tx.txHash.slice(0, 20) + '...' : 'Pending'}</span>
        </div>
        <span class="badge amber">UNMINED</span>
      </div>
    `).join('');
  } catch (err) {
    console.error('Error loading mempool:', err);
  }
}

// Results Loader
async function loadResults() {
  try {
    const res = await fetch('/api/results?electionId=ELECTION-2026');
    const data = await res.json();
    const list = document.getElementById('resultsList');
    const totalEl = document.getElementById('resTotalVotes');
    const leaderEl = document.getElementById('resLeader');
    if (!list) return;

    const results = data.results || [];
    let total = 0;
    results.forEach(r => total += r.voteCount);

    if (totalEl) totalEl.textContent = total;
    if (leaderEl) {
      leaderEl.textContent = results.length > 0 && results[0].voteCount > 0 ? results[0].candidateName : 'No votes yet';
    }

    if (results.length === 0) {
      list.innerHTML = '<div class="inspector-empty">No votes recorded yet on blockchain.</div>';
      return;
    }

    list.innerHTML = results.map(r => `
      <div class="res-item">
        <div class="res-item-header">
          <strong>${r.candidateName} (${r.candidateId})</strong>
          <span>${r.voteCount} votes (${r.percentage.toFixed(1)}%)</span>
        </div>
        <div class="res-item-bar-bg">
          <div class="res-item-bar-fill" style="width: ${r.percentage}%"></div>
        </div>
      </div>
    `).join('');
  } catch (err) {
    console.error('Error loading results:', err);
  }
}

// Block Explorer
function renderExplorer() {
  if (!blockchainState || !blockchainState.chain) return;
  const container = document.getElementById('chainTimeline');
  if (!container) return;

  container.innerHTML = blockchainState.chain.map(b => `
    <div class="block-pill ${currentSelectedBlockIndex === b.index ? 'active' : ''}" onclick="selectBlockForInspection(${b.index})">
      <div class="block-pill-header">
        <span class="block-idx">#${b.index}</span>
        <span class="badge ${b.index === 0 ? 'purple' : 'blue'}">
          ${b.index === 0 ? 'Genesis' : `${b.transactions.length} Txs`}
        </span>
      </div>
      <div class="block-pill-hash font-mono">
        ${b.hash.slice(0, 16)}...
      </div>
      <div class="block-pill-meta font-mono">
        <span>Prev: ${b.previousHash.slice(0, 8)}...</span>
        <span>Nonce: ${b.nonce}</span>
      </div>
    </div>
  `).join('');

  if (currentSelectedBlockIndex !== null) {
    selectBlockForInspection(currentSelectedBlockIndex);
  } else if (blockchainState.chain.length > 0) {
    selectBlockForInspection(blockchainState.chain[blockchainState.chain.length - 1].index);
  }
}

function selectBlockForInspection(index) {
  currentSelectedBlockIndex = index;
  const inspector = document.getElementById('blockInspector');
  if (!inspector || !blockchainState) return;

  const block = blockchainState.chain.find(b => b.index === index);
  if (!block) return;

  // Highlight pill
  document.querySelectorAll('.chain-timeline .block-pill').forEach((p, idx) => {
    p.classList.toggle('active', idx === index);
  });

  const txRows = (block.transactions || []).map((tx, idx) => `
    <div class="receipt-row font-mono" style="background: rgba(255,255,255,0.02); padding: 0.5rem; margin-top: 0.35rem; border-radius: 4px;">
      <div>
        <strong>[Tx #${idx + 1}]</strong> Candidate: <span style="color: var(--accent-cyan);">${tx.candidateName}</span> (${tx.candidateId})<br>
        <span style="font-size: 0.75rem; color: #94a3b8;">Voter: ${tx.voterAddress}</span><br>
        <span style="font-size: 0.72rem; color: #64748b;">Sig: ${tx.signature ? tx.signature.slice(0, 32) + '...' : 'N/A'}</span>
      </div>
      <span class="badge emerald" style="font-size: 0.68rem;">VERIFIED</span>
    </div>
  `).join('');

  inspector.innerHTML = `
    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 1rem;">
      <h4 style="font-size: 1.15rem; color: var(--accent-cyan);">Block #${block.index} Cryptographic Inspector</h4>
      <span class="badge ${block.index === 0 ? 'purple' : 'emerald'}">${block.index === 0 ? 'Genesis Ledger Block' : 'Mined & Sealed'}</span>
    </div>

    <div class="receipt-details font-mono">
      <div class="receipt-row">
        <span class="label">Block Hash (SHA-256):</span>
        <span class="value" style="color: var(--accent-cyan);">${block.hash}</span>
      </div>
      <div class="receipt-row">
        <span class="label">Previous Block Hash:</span>
        <span class="value">${block.previousHash}</span>
      </div>
      <div class="receipt-row">
        <span class="label">Merkle Root:</span>
        <span class="value" style="color: #a855f7;">${block.merkleRoot}</span>
      </div>
      <div class="receipt-row">
        <span class="label">Proof-of-Work Nonce:</span>
        <span class="value">${block.nonce} (Difficulty: ${block.difficulty})</span>
      </div>
      <div class="receipt-row">
        <span class="label">Validator / Miner:</span>
        <span class="value">${block.minedBy}</span>
      </div>
      <div class="receipt-row">
        <span class="label">Timestamp:</span>
        <span class="value">${new Date(block.timestamp).toLocaleString()}</span>
      </div>
    </div>

    <div style="margin-top: 1rem;">
      <h5 style="color: var(--text-main); margin-bottom: 0.5rem;">Transactions in Block (${block.transactions ? block.transactions.length : 0}):</h5>
      ${txRows || '<div class="text-muted">No transactions in this block</div>'}
    </div>
  `;
}

// Security & Tamper Lab
function setupSecurityLab() {
  const btnTamper = document.getElementById('btnTamperBlock');
  const btnVerify = document.getElementById('btnVerifyTamper');

  if (btnTamper) {
    btnTamper.addEventListener('click', async () => {
      const blockIdx = document.getElementById('tamperBlockSelect').value;
      const fakeName = document.getElementById('tamperFakeName').value.trim() || 'MALICIOUS_TAMPERED_CANDIDATE';

      try {
        const res = await fetch('/api/tamper', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ blockIndex: blockIdx, fakeCandidateName: fakeName })
        });
        const data = await res.json();

        showTamperLog(`[ATTACK INJECTED] Altered transaction in Block #${blockIdx}!\nCandidate name modified to "${fakeName}".\nNotice that the attacker did NOT re-mine the entire chain. Let's see what happens during validation...`, false);
        showToast(`Tampered Block #${blockIdx}`, 'warning');
        refreshAllData();
      } catch (err) {
        showToast('Error injecting tamper', 'error');
      }
    });
  }

  if (btnVerify) {
    btnVerify.addEventListener('click', runAudit);
  }
}

function updateTamperBlockOptions(chain) {
  const select = document.getElementById('tamperBlockSelect');
  if (!select || !chain) return;

  const validBlocks = chain.filter(b => b.index > 0);
  if (validBlocks.length === 0) {
    select.innerHTML = '<option value="">No mined blocks available yet</option>';
    return;
  }

  select.innerHTML = validBlocks.map(b => `
    <option value="${b.index}">Block #${b.index} (${b.transactions ? b.transactions.length : 0} votes)</option>
  `).join('');
}

async function runAudit() {
  try {
    const res = await fetch('/api/verify');
    const data = await res.json();

    const isSuccess = data.valid;
    showTamperLog(`[AUDIT INITIATED] Deep cryptographic scan across ${data.blocks} blocks...\n${data.report}\nResult: ${isSuccess ? 'CRYPTOGRAPHIC INTEGRITY INTACT' : 'TAMPER DETECTED & REJECTED'}`, isSuccess);

    // Update global status badge
    if (chainIntegrityBadge && chainIntegrityText) {
      if (isSuccess) {
        chainIntegrityBadge.className = 'integrity-chip valid';
        chainIntegrityText.textContent = '100% Verified';
      } else {
        chainIntegrityBadge.className = 'integrity-chip invalid';
        chainIntegrityText.textContent = 'Chain Compromised!';
      }
    }

    // Update Dashboard Audit Box
    const dashTitle = document.getElementById('dashAuditTitle');
    const dashMsg = document.getElementById('dashAuditMsg');
    if (dashTitle && dashMsg) {
      if (isSuccess) {
        dashTitle.textContent = 'Cryptographic Chain Verified';
        dashMsg.textContent = 'All blocks, Merkle trees, and voter signatures satisfy consensus validation.';
      } else {
        dashTitle.textContent = 'Security Alert: Tampering Detected';
        dashMsg.textContent = data.report;
      }
    }

    showToast(isSuccess ? 'Audit passed: 100% valid' : 'Security Alert: Tamper caught!', isSuccess ? 'success' : 'error');
  } catch (err) {
    showToast('Failed to run audit', 'error');
  }
}

function showTamperLog(message, isValid) {
  const screen = document.getElementById('tamperAuditScreen');
  const icon = document.getElementById('tamperScreenIcon');
  const title = document.getElementById('tamperScreenTitle');
  const log = document.getElementById('tamperScreenLog');

  if (!screen || !log) return;
  screen.classList.remove('hidden');

  if (icon) icon.textContent = isValid ? '🛡️' : '🚨';
  if (title) title.textContent = isValid ? 'Audit Result: Blockchain Valid' : 'Security Alert: Tamper Detected!';
  if (log) log.textContent = message;
}

// Background Integrity Checker
async function checkIntegrity() {
  try {
    const res = await fetch('/api/verify');
    const data = await res.json();
    if (chainIntegrityBadge && chainIntegrityText) {
      if (data.valid) {
        chainIntegrityBadge.className = 'integrity-chip valid';
        chainIntegrityText.textContent = '100% Verified';
      } else {
        chainIntegrityBadge.className = 'integrity-chip invalid';
        chainIntegrityText.textContent = 'Compromised!';
      }
    }
  } catch (e) {
    // Ignore periodic check errors
  }
}

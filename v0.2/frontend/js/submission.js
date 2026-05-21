const STATUS_LABELS = {
    'accepted': 'Accepted',
    'wrong_answer': 'Wrong Answer',
    'compile_error': 'Compile Error',
    'time_limit': 'Time Limit Exceeded',
    'memory_limit': 'Memory Limit Exceeded',
    'runtime_error': 'Runtime Error',
    'pending': 'Pending',
    'judging': 'Judging',
};

const STATUS_COLORS = {
    'accepted': 'var(--accent-green)',
    'wrong_answer': 'var(--accent-pink)',
    'compile_error': 'var(--accent-orange)',
    'time_limit': 'var(--accent-yellow)',
    'memory_limit': 'var(--accent-purple)',
    'runtime_error': 'var(--accent-pink)',
    'pending': 'var(--text-secondary)',
    'judging': 'var(--accent-blue)',
};

let currentPage = 0;
let currentFilter = '';
const PAGE_SIZE = 20;

// ===== Detail View =====

async function loadSubmissionDetail() {
    const user = await checkAuth();
    if (!user) return;

    const params = new URLSearchParams(window.location.search);
    const id = params.get('id');
    if (!id) {
        document.getElementById('submission-detail').innerHTML =
            '<p style="color:var(--accent-pink);">No submission ID specified.</p>';
        return;
    }

    document.getElementById('submission-status').textContent = 'Loading...';
    hideElement('submission-list-view');
    showElement('submission-detail-view');

    const maxAttempts = 60;
    for (let i = 0; i < maxAttempts; i++) {
        const { status, data } = await apiGet('/api/submission/' + id);
        if (status !== 200) {
            document.getElementById('submission-detail-view').innerHTML =
                '<p style="color:var(--accent-pink);">Failed to load submission.</p>';
            return;
        }

        await renderSubmissionDetail(data);

        if (data.status !== 'pending' && data.status !== 'judging') {
            return;
        }

        await sleep(1000);
    }
}

async function renderSubmissionDetail(data) {
    document.getElementById('submission-id').textContent = '#' + data.id;
    setTextContent('submission-status', STATUS_LABELS[data.status] || data.status);
    setStyle('submission-status', 'color', STATUS_COLORS[data.status] || 'var(--text-secondary)');
    setTextContent('submission-score', data.score + '%');
    setTextContent('submission-language', data.language);
    setTextContent('submission-time', data.submitted_at || '');

    const problemLink = document.getElementById('submission-problem-link');
    if (problemLink) {
        const probData = await fetchProblemTitle(data.problem_id);
        problemLink.textContent = probData || 'Problem #' + data.problem_id;
        problemLink.href = 'problem-detail.html?id=' + data.problem_id;
    }

    const codeEl = document.getElementById('submission-code');
    if (codeEl) {
        codeEl.textContent = data.code || '';
    }

    const resultsEl = document.getElementById('test-results');
    if (!resultsEl || !data.judge_result) return;

    let html = '';
    if (data.judge_result.test_results) {
        html += '<div style="margin-bottom:12px;font-size:14px;font-weight:600;color:var(--text-secondary);">Test Results</div>';
        data.judge_result.test_results.forEach(function(tr, i) {
            const icon = tr.passed ? '✓' : '✗';
            const color = tr.passed ? 'var(--accent-green)' : 'var(--accent-pink)';
            const label = tr.status || (tr.passed ? 'accepted' : 'wrong_answer');
            html += '<div class="test-result-row">';
            html += '<span class="test-result-icon" style="color:' + color + ';">' + icon + '</span>';
            html += '<span style="color:var(--text-secondary);">Case #' + (i + 1) + ':</span>';
            html += '<span style="color:' + color + ';font-weight:500;">' + label + '</span>';
            if (!tr.passed) {
                html += '<div style="width:100%;margin-top:4px;padding-left:24px;font-size:12px;">';
                html += '<div style="color:var(--text-muted);">Expected:</div>';
                html += '<pre style="font-family:monospace;color:var(--accent-green);background:var(--bg-primary);padding:4px 8px;border-radius:4px;margin:2px 0 6px;white-space:pre-wrap;">' + escapeHtml(tr.expected_output) + '</pre>';
                html += '<div style="color:var(--text-muted);">Got:</div>';
                html += '<pre style="font-family:monospace;color:var(--accent-pink);background:var(--bg-primary);padding:4px 8px;border-radius:4px;margin:2px 0 6px;white-space:pre-wrap;">' + escapeHtml(tr.your_output) + '</pre>';
                html += '</div>';
            }
            html += '</div>';
        });
    }

    if (data.judge_result.message) {
        html += '<div class="judge-message">';
        html += '<div style="font-size:12px;color:var(--text-muted);margin-bottom:4px;">Compiler Output:</div>';
        html += '<pre>' + escapeHtml(data.judge_result.message) + '</pre>';
        html += '</div>';
    }

    resultsEl.innerHTML = html || '<p style="color:var(--text-secondary);font-size:14px;">No test results available.</p>';
}

// ===== List View =====

async function loadSubmissionList() {
    const user = await checkAuth();
    if (!user) return;

    const params = new URLSearchParams(window.location.search);
    const problemId = params.get('problem_id');
    const statusFilter = params.get('status');

    if (problemId) {
        const title = await fetchProblemTitle(parseInt(problemId));
        const heading = document.querySelector('.page-header h1');
        if (heading) {
            heading.textContent = 'Submissions: ' + (title || 'Problem #' + problemId);
        }
    }

    const filterSelect = document.getElementById('status-filter');
    if (filterSelect && statusFilter) {
        filterSelect.value = statusFilter;
    }

    currentFilter = params.get('status') || '';
    currentPage = 0;

    hideElement('submission-detail-view');
    showElement('submission-list-view');

    await fetchAndRenderSubmissions();
}

async function fetchAndRenderSubmissions() {
    const container = document.getElementById('submission-list');
    container.innerHTML = '<p style="color:var(--text-secondary);">Loading...</p>';

    const params = new URLSearchParams(window.location.search);
    const problemId = params.get('problem_id');

    let url = '/api/submissions?limit=' + PAGE_SIZE + '&offset=' + (currentPage * PAGE_SIZE);
    if (problemId) url += '&problem_id=' + problemId;

    const { status, data } = await apiGet(url);
    if (status !== 200) {
        container.innerHTML = '<p style="color:var(--accent-pink);">Failed to load submissions.'
            + ' <button class="btn btn-sm" onclick="fetchAndRenderSubmissions()">Retry</button></p>';
        return;
    }

    renderSubmissionList(data);
}

function renderSubmissionList(data) {
    const container = document.getElementById('submission-list');

    if (!data.submissions || data.submissions.length === 0) {
        container.innerHTML = '<div class="empty-state"><p>No submissions yet. Start coding!</p></div>';
        document.getElementById('pagination-controls').innerHTML = '';
        return;
    }

    let html = '<table class="submission-table">';
    html += '<thead><tr>';
    html += '<th>ID</th><th>Problem</th><th>Status</th><th>Score</th><th>Time</th>';
    html += '</tr></thead><tbody>';

    data.submissions.forEach(function(s) {
        const label = STATUS_LABELS[s.status] || s.status;
        const color = STATUS_COLORS[s.status] || 'var(--text-secondary)';
        html += '<tr onclick="window.location=\'submissions.html?id=' + s.id + '\'" style="cursor:pointer;">';
        html += '<td class="sub-id">#' + s.id + '</td>';
        html += '<td><span class="submission-detail-link">Problem ' + s.problem_id + '</span></td>';
        html += '<td class="sub-status" style="color:' + color + ';">' + label + '</td>';
        html += '<td class="sub-score">' + s.score + '%</td>';
        html += '<td class="sub-time">' + (s.submitted_at || '').substring(0, 19) + '</td>';
        html += '</tr>';
    });

    html += '</tbody></table>';
    container.innerHTML = html;

    const total = data.total || 0;
    const totalPages = Math.ceil(total / PAGE_SIZE);
    renderPagination(totalPages);
}

function renderPagination(totalPages) {
    const controls = document.getElementById('pagination-controls');
    if (totalPages <= 1) {
        controls.innerHTML = '';
        return;
    }

    let html = '<div class="pagination">';
    html += '<button onclick="goToPage(0)" ' + (currentPage === 0 ? 'disabled' : '') + '>&laquo; First</button>';
    html += '<button onclick="goToPage(' + (currentPage - 1) + ')" ' + (currentPage === 0 ? 'disabled' : '') + '>&lsaquo; Prev</button>';
    html += '<span class="page-info">Page ' + (currentPage + 1) + ' of ' + totalPages + '</span>';
    html += '<button onclick="goToPage(' + (currentPage + 1) + ')" ' + (currentPage >= totalPages - 1 ? 'disabled' : '') + '>Next &rsaquo;</button>';
    html += '<button onclick="goToPage(' + (totalPages - 1) + ')" ' + (currentPage >= totalPages - 1 ? 'disabled' : '') + '>Last &raquo;</button>';
    html += '</div>';
    controls.innerHTML = html;
}

function goToPage(page) {
    currentPage = page;
    fetchAndRenderSubmissions();
    window.scrollTo({ top: 0, behavior: 'smooth' });
}

function applyStatusFilter() {
    const select = document.getElementById('status-filter');
    const val = select.value;
    const params = new URLSearchParams(window.location.search);
    if (val) {
        params.set('status', val);
    } else {
        params.delete('status');
    }
    window.location.search = params.toString();
}

// ===== Helpers =====

const problemCache = {};

async function fetchProblemTitle(id) {
    if (problemCache[id]) return problemCache[id];
    const { status, data } = await apiGet('/api/problems/' + id);
    if (status === 200 && data.title) {
        problemCache[id] = data.title;
        return data.title;
    }
    return null;
}

function sleep(ms) {
    return new Promise(r => setTimeout(r, ms));
}

function setTextContent(id, text) {
    const el = document.getElementById(id);
    if (el) el.textContent = text;
}

function setStyle(id, prop, value) {
    const el = document.getElementById(id);
    if (el) el.style[prop] = value;
}

function hideElement(id) {
    const el = document.getElementById(id);
    if (el) el.style.display = 'none';
}

function showElement(id) {
    const el = document.getElementById(id);
    if (el) el.style.display = '';
}

function escapeHtml(s) {
    if (!s) return '';
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}

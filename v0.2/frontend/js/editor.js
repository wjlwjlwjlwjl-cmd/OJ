let editor;
let currentProblemId = null;

const DEFAULT_CODE = `#include <iostream>
using namespace std;

int main() {
    // Your code here
    return 0;
}
`;

async function init() {
    const user = await checkAuth();
    if (!user) return;

    const params = new URLSearchParams(window.location.search);
    const id = params.get('id');
    if (!id) {
        document.getElementById('problem-title').textContent = 'No problem specified';
        return;
    }
    currentProblemId = parseInt(id);
    loadProblem(currentProblemId);
    initEditor();
}

function initEditor() {
    editor = ace.edit('editor');
    editor.setTheme('ace/theme/monokai');
    editor.session.setMode('ace/mode/c_cpp');
    editor.setValue(DEFAULT_CODE, -1);
    editor.setOptions({
        fontSize: '14px',
        fontFamily: '"JetBrains Mono", "Fira Code", monospace',
        showPrintMargin: false,
        showGutter: true,
        tabSize: 4,
        useSoftTabs: true,
        enableBasicAutocompletion: false,
        enableLiveAutocompletion: false,
    });
    editor.session.setUseWrapMode(true);
    editor.renderer.setScrollMargin(12, 12);
    editor.renderer.setPadding(16);
}

async function loadProblem(id) {
    const { status, data } = await apiGet('/api/problems/' + id);
    if (status !== 200) {
        document.getElementById('problem-title').textContent = 'Problem not found';
        document.getElementById('problem-description').textContent =
            data.error || 'Failed to load problem';
        return;
    }

    document.getElementById('problem-title').textContent = data.title;

    const badge = document.getElementById('difficulty-badge');
    badge.textContent = data.difficulty;
    badge.className = 'badge badge-' + data.difficulty;

    document.getElementById('time-limit').textContent = data.time_limit + ' ms';
    document.getElementById('memory-limit').textContent = data.memory_limit + ' MB';
    document.getElementById('problem-description').textContent = data.description;
}

async function submitCode() {
    const code = editor.getValue();
    const btn = document.getElementById('submit-btn');
    const msg = document.getElementById('submit-message');

    btn.disabled = true;
    btn.textContent = 'Submitting...';
    msg.textContent = '';
    msg.style.color = '';

    const { status, data } = await apiPost('/api/submit', {
        problem_id: currentProblemId,
        language: 'cpp',
        code: code,
    });

    if (status !== 202) {
        btn.disabled = false;
        btn.textContent = 'Submit';
        msg.style.color = 'var(--accent-pink)';
        msg.textContent = data.error || 'Submission failed';
        return;
    }

    msg.style.color = 'var(--text-secondary)';
    msg.textContent = 'Judging... (ID: ' + data.id + ')';

    pollSubmission(data.id, btn, msg);
}

async function pollSubmission(id, btn, msg) {
    const maxAttempts = 60;
    for (let i = 0; i < maxAttempts; i++) {
        await new Promise(r => setTimeout(r, 1000));

        const { status, data } = await apiGet('/api/submission/' + id);
        if (status !== 200) break;

        if (data.status === 'pending' || data.status === 'judging') {
            const dots = '.'.repeat((i % 3) + 1);
            msg.textContent = 'Judging' + dots;
            continue;
        }

        btn.disabled = false;
        btn.textContent = 'Submit';

        const colorMap = {
            'accepted': 'var(--accent-green)',
            'wrong_answer': 'var(--accent-pink)',
            'compile_error': 'var(--accent-orange)',
            'time_limit': 'var(--accent-yellow)',
            'memory_limit': 'var(--accent-purple)',
            'runtime_error': 'var(--accent-pink)',
        };
        msg.style.color = colorMap[data.status] || 'var(--text-secondary)';

        const labelMap = {
            'accepted': 'Accepted',
            'wrong_answer': 'Wrong Answer',
            'compile_error': 'Compile Error',
            'time_limit': 'Time Limit Exceeded',
            'memory_limit': 'Memory Limit Exceeded',
            'runtime_error': 'Runtime Error',
        };
        const label = labelMap[data.status] || data.status;

        const detailLink = 'submissions.html?id=' + id;
        msg.innerHTML = label + ' (' + data.score + '%)'
            + ' &middot; <a href="' + detailLink + '" style="color:var(--accent-blue);text-decoration:underline;">Details</a>';

        showJudgeResult(data);

        return;
    }

    btn.disabled = false;
    btn.textContent = 'Submit';
    if (msg.textContent.startsWith('Judging')) {
        msg.style.color = 'var(--accent-orange)';
        msg.textContent = 'Judge timed out';
    }
}

function showJudgeResult(data) {
    const existing = document.getElementById('judge-result-panel');
    if (existing) existing.remove();

    if (!data.judge_result) return;

    const panel = document.createElement('div');
    panel.id = 'judge-result-panel';
    panel.style.cssText = 'margin-top:16px;padding:16px;background:var(--bg-tertiary);border-radius:8px;';

    let html = '<div style="font-size:13px;color:var(--text-secondary);margin-bottom:8px;">Test Results:</div>';

    if (data.judge_result.test_results) {
        data.judge_result.test_results.forEach(function(tr, i) {
            const icon = tr.passed ? '✓' : '✗';
            const color = tr.passed ? 'var(--accent-green)' : 'var(--accent-pink)';
            html += '<div style="display:flex;align-items:center;gap:8px;padding:4px 0;font-family:monospace;font-size:13px;">';
            html += '<span style="color:' + color + ';font-weight:bold;">' + icon + '</span>';
            html += '<span style="color:var(--text-secondary);">Test Case #' + (i + 1) + ':</span>';
            html += '<span style="color:' + color + ';">' + (tr.status || (tr.passed ? 'accepted' : 'wrong_answer')) + '</span>';
            if (!tr.passed) {
                html += '<div style="width:100%;padding-left:24px;font-size:12px;margin-top:2px;">';
                html += '<div style="color:var(--text-muted);">Expected: <span style="font-family:monospace;color:var(--accent-green);">' + escapeHtml(tr.expected_output) + '</span></div>';
                html += '<div style="color:var(--text-muted);">Got: <span style="font-family:monospace;color:var(--accent-pink);">' + escapeHtml(tr.your_output) + '</span></div>';
                html += '</div>';
            }
            html += '</div>';
        });
    }

    if (data.judge_result.message) {
        html += '<div style="margin-top:8px;padding:8px;background:var(--bg-secondary);border-radius:4px;font-family:monospace;font-size:12px;color:var(--accent-orange);white-space:pre-wrap;border:1px solid var(--border);">';
        html += escapeHtml(data.judge_result.message);
        html += '</div>';
    }

    panel.innerHTML = html;
    document.querySelector('.editor-actions').after(panel);
}

function escapeHtml(s) {
    if (!s) return '';
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}

function escapeHtml(s) {
    if (!s) return '';
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}

document.addEventListener('DOMContentLoaded', init);

let allProblems = [];

async function init() {
    const user = await checkAuth();
    if (!user) return;
    await loadProblems();
}

async function loadProblems() {
    const { status, data } = await apiGet('/api/problems');
    if (status !== 200) {
        document.getElementById('problem-list').innerHTML =
            '<div class="empty-state"><p>Failed to load problems.</p></div>';
        return;
    }
    allProblems = data;
    renderProblems(data);
}

function renderProblems(problems) {
    const container = document.getElementById('problem-list');
    if (!problems || problems.length === 0) {
        container.innerHTML = '<div class="empty-state"><p>No problems found.</p></div>';
        return;
    }
    let html = '';
    for (const p of problems) {
        html += '<div class="problem-card" onclick="goToProblem(' + p.id + ')">'
              + '<div class="problem-card-left">'
              + '<span class="problem-card-id">#' + p.id + '</span>'
              + '<span class="problem-card-title">' + esc(p.title) + '</span>'
              + '</div>'
              + '<div class="problem-card-right">'
              + '<span class="badge badge-' + p.difficulty + '">' + p.difficulty + '</span>'
               + '<div class="problem-card-stats">'
               + '<span>' + p.time_limit + 'ms</span>'
               + '<span>' + p.memory_limit + 'MB</span>'
               + '</div>'
               + '<a href="submissions.html?problem_id=' + p.id + '" class="btn btn-sm btn-secondary" style="font-size:12px;" onclick="event.stopPropagation();">Submissions</a>'
               + '</div>'
               + '</div>';
    }
    container.innerHTML = html;
}

function filterProblems() {
    const q = document.getElementById('search-input').value.toLowerCase();
    const filtered = allProblems.filter(p =>
        p.title.toLowerCase().includes(q) ||
        p.difficulty.toLowerCase().includes(q)
    );
    renderProblems(filtered);
}

function goToProblem(id) {
    window.location.href = 'problem-detail.html?id=' + id;
}

function esc(s) {
    var d = document.createElement('div');
    d.textContent = s;
    return d.innerHTML;
}

document.addEventListener('DOMContentLoaded', init);

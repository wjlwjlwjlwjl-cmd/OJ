let problems = [];

async function init() {
    const user = await checkAuth();
    if (!user) return;
    if (user.role !== 'admin') {
        window.location.href = 'problems.html';
        return;
    }
    await loadProblems();
}

async function loadProblems() {
    const { status, data } = await apiGet('/api/problems');
    if (status !== 200) {
        document.getElementById('problem-list').innerHTML = '<p>Failed to load problems</p>';
        return;
    }
    problems = data;
    renderProblemList();
    document.getElementById('empty-state').style.display = 'block';
    document.getElementById('form-container').style.display = 'none';
}

function renderProblemList() {
    const container = document.getElementById('problem-list');
    if (problems.length === 0) {
        container.innerHTML = '<p>No problems yet.</p>';
        return;
    }
    let html = '';
    for (const p of problems) {
        const diffClass = 'diff-' + p.difficulty;
        html += '<div class="problem-item" data-id="' + p.id + '">'
              + '<div class="problem-item-info" onclick="selectProblem(' + p.id + ')">'
              + '<span class="problem-title">' + escapeHtml(p.title) + '</span>'
              + '<span class="' + diffClass + '">' + p.difficulty + '</span>'
              + '</div>'
              + '<div class="problem-item-actions">'
              + '<button onclick="editProblem(' + p.id + ')" class="btn-small">Edit</button>'
              + '<button onclick="deleteProblem(' + p.id + ')" class="btn-small btn-danger">Del</button>'
              + '</div>'
              + '</div>';
    }
    container.innerHTML = html;
}

function showCreateForm() {
    document.getElementById('form-title').textContent = 'Create Problem';
    document.getElementById('save-btn').textContent = 'Create';
    document.getElementById('problem-id').value = '';
    document.getElementById('title').value = '';
    document.getElementById('difficulty').value = 'medium';
    document.getElementById('time_limit').value = '1000';
    document.getElementById('memory_limit').value = '256';
    document.getElementById('description').value = '';
    document.getElementById('test_cases').value = '[]';
    document.getElementById('form-message').textContent = '';
    document.getElementById('empty-state').style.display = 'none';
    document.getElementById('form-container').style.display = 'block';
}

function cancelForm() {
    document.getElementById('form-container').style.display = 'none';
    document.getElementById('empty-state').style.display = 'block';
}

async function saveProblem() {
    const id = document.getElementById('problem-id').value;
    const data = {
        title: document.getElementById('title').value,
        difficulty: document.getElementById('difficulty').value,
        time_limit: parseInt(document.getElementById('time_limit').value) || 1000,
        memory_limit: parseInt(document.getElementById('memory_limit').value) || 256,
        description: document.getElementById('description').value,
        test_cases: document.getElementById('test_cases').value,
    };

    let status, response;
    if (id) {
        ({ status, data: response } = await apiPut('/api/problems/' + id, data));
    } else {
        ({ status, data: response } = await apiPost('/api/problems', data));
    }

    const msg = document.getElementById('form-message');
    if (status === 201 || status === 200) {
        msg.style.color = 'green';
        msg.textContent = 'Saved successfully!';
        await loadProblems();
    } else {
        msg.style.color = 'red';
        msg.textContent = response.error || 'Save failed';
    }
}

function selectProblem(id) {
    const p = problems.find(x => x.id === id);
    if (!p) return;
    document.getElementById('empty-state').innerHTML = '<h3>' + escapeHtml(p.title) + '</h3>'
        + '<p>Difficulty: ' + p.difficulty + '</p>'
        + '<p>Time: ' + p.time_limit + 'ms, Memory: ' + p.memory_limit + 'MB</p>'
        + '<pre>' + escapeHtml(p.description) + '</pre>';
}

async function editProblem(id) {
    const { status, data } = await apiGet('/api/problems/' + id);
    if (status !== 200) return;

    document.getElementById('form-title').textContent = 'Edit Problem';
    document.getElementById('save-btn').textContent = 'Update';
    document.getElementById('problem-id').value = data.id;
    document.getElementById('title').value = data.title;
    document.getElementById('difficulty').value = data.difficulty;
    document.getElementById('time_limit').value = data.time_limit;
    document.getElementById('memory_limit').value = data.memory_limit;
    document.getElementById('description').value = data.description;
    document.getElementById('test_cases').value = data.test_cases;
    document.getElementById('form-message').textContent = '';
    document.getElementById('empty-state').style.display = 'none';
    document.getElementById('form-container').style.display = 'block';
}

async function deleteProblem(id) {
    if (!confirm('Are you sure you want to delete this problem?')) return;

    const { status, data } = await apiDelete('/api/problems/' + id);
    if (status === 200) {
        await loadProblems();
    } else {
        alert(data.error || 'Delete failed');
    }
}

function escapeHtml(str) {
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
}

document.addEventListener('DOMContentLoaded', init);

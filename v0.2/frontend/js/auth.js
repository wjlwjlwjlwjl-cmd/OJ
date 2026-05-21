async function register() {
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const msg = document.getElementById('message');

    const { status, data } = await apiPost('/api/auth/register', { username, password });
    if (status === 201) {
        msg.style.color = 'green';
        msg.textContent = 'Registration successful! Redirecting...';
        setTimeout(() => { window.location.href = 'pages/problems.html'; }, 1000);
    } else {
        msg.style.color = 'red';
        msg.textContent = data.error || 'Registration failed';
    }
}

async function login() {
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const msg = document.getElementById('message');

    const { status, data } = await apiPost('/api/auth/login', { username, password });
    if (status === 200) {
        msg.style.color = 'green';
        msg.textContent = 'Login successful! Redirecting...';
        setTimeout(() => { window.location.href = 'pages/problems.html'; }, 1000);
    } else {
        msg.style.color = 'red';
        msg.textContent = data.error || 'Login failed';
    }
}

async function logout() {
    await apiPost('/api/auth/logout');
    window.location.href = '../index.html';
}

async function checkAuth() {
    const { status, data } = await apiGet('/api/auth/me');
    if (status !== 200) {
        window.location.href = '../index.html';
        return null;
    }
    document.getElementById('username-display').textContent = data.username;
    return data;
}

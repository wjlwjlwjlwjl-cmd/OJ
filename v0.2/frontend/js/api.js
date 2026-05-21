const API_BASE = 'http://127.0.0.1:8080';

async function api(method, path, body) {
    const opts = {
        method,
        headers: { 'Content-Type': 'application/json' },
        credentials: 'include',
    };
    if (body !== undefined) {
        opts.body = JSON.stringify(body);
    }
    const res = await fetch(API_BASE + path, opts);
    const data = await res.json();
    return { status: res.status, data };
}

async function apiGet(path) {
    return api('GET', path);
}

async function apiPost(path, body) {
    return api('POST', path, body);
}

async function apiPut(path, body) {
    return api('PUT', path, body);
}

async function apiDelete(path) {
    return api('DELETE', path);
}

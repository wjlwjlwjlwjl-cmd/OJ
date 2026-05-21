# OJ — Online Judge

A LeetCode-like online judge platform with C++ backend, vanilla frontend, and MySQL database.

## Quick Start

```bash
# 1. Initialize database
./scripts/init_db.sh
# (enter MySQL root password when prompted)

# 2. Start all services
./scripts/start.sh

# 3. Open in browser
# http://localhost:8080
```

## Prerequisites

- **MySQL** 8.0+ (running on localhost:3306)
- **g++** 11+ with C++17 support
- **cmake** 3.16+
- **libmysqlclient-dev**
- **libssl-dev**
- **libgtest-dev** (for running tests)

## Directory Structure

```
oj/
├── backend/          # C++ HTTP server (cpp-httplib)
├── judge/            # Judge service (fork+execve runner)
├── frontend/         # HTML/CSS/JS static files
├── scripts/          # Build & deployment scripts
├── sql/              # Database schema & seed data
├── config/           # Configuration files
└── test/             # Unit & integration tests (64+ tests)
```

## Scripts

| Script | Description |
|--------|-------------|
| `scripts/init_db.sh`    | Create MySQL database, tables, and application user |
| `scripts/build.sh`      | Build judge service + backend + tests |
| `scripts/start.sh`      | Build & start all services (Ctrl+C to stop) |
| `scripts/stop.sh`       | Gracefully stop running services |

## Configuration

Edit `config/config.json`:

- **server.host/port** — Backend listen address (default: 0.0.0.0:8080)
- **db.host/port/user/password/database** — MySQL connection
- **judge.host/port** — Judge service address (default: 127.0.0.1:9090)
- **session.secret** — Session signing secret (change for production!)
- **session.expiry_hours** — Session TTL

## Manual Start (without scripts)

```bash
# Terminal 1: Judge service
cd judge && mkdir -p build && cd build
cmake .. && make -j$(nproc)
./oj-judge

# Terminal 2: Backend
cd backend && mkdir -p build && cd build
cmake .. && make -j$(nproc)
./oj-backend

# Open http://localhost:8080
```

## Running Tests

```bash
./scripts/build.sh   # builds everything including tests

# Judge unit tests (28 tests)
./test/todo-05-judge/build/oj-judge-test

# Submission API tests (21 tests)
./test/todo-06-submission-api/build/oj-submission-test

# Integration tests (15 tests)
./test/todo-08-integration/build/oj-integration-test
```

## Default Admin Account

After running `init_db.sh`, if the seed data was loaded:
- Username: `admin`
- Password: `admin123`

Otherwise, register a new account and manually set `role='admin'` in the database:

```sql
UPDATE users SET role='admin' WHERE username='your-username';
```

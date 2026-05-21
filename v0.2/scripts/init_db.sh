#!/bin/bash
set -e

echo "Initializing OJ database..."

MYSQL_USER="${MYSQL_USER:-root}"
MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_PORT="${MYSQL_PORT:-3306}"

MYSQL_CMD="mysql -u $MYSQL_USER -h $MYSQL_HOST -P $MYSQL_PORT"

if [ -n "$MYSQL_PASSWORD" ]; then
    MYSQL_CMD="$MYSQL_CMD -p$MYSQL_PASSWORD"
fi

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "Creating database and tables..."
$MYSQL_CMD < "$SCRIPT_DIR/sql/init.sql"

echo "Creating application user..."
$MYSQL_CMD -e "
    CREATE USER IF NOT EXISTS 'oj'@'localhost' IDENTIFIED BY 'oj_password';
    GRANT ALL PRIVILEGES ON oj.* TO 'oj'@'localhost';
    FLUSH PRIVILEGES;
"

echo "OJ database initialized successfully!"

PRAGMA encoding = 'UTF-8';

CREATE TABLE IF NOT EXISTS Users (
    username TEXT PRIMARY KEY,
    displayName TEXT UNIQUE
);

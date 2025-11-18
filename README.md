MiniDB — A Lightweight C++ Database Engine with REST API + React Frontend

MiniDB is a custom lightweight database engine written in modern C++17.
Originally inspired by the structure of SQLite’s internals, MiniDB provides:

A modular storage engine

Clean C++ APIs for executing queries

A built-in HTTP server exposing REST endpoints

Optional React frontend for interacting visually with the database

This project demonstrates systems-level programming, API design, and full-stack integration in one deployable application.

Features
Custom Database Engine (C++17)

In-memory row storage with persistence to file

Insert & Select operations

Row serialization / deserialization

Simple query parsing (insert, select)

Modular architecture:

engine_core → storage logic

engine_api → safe wrapper layer

server → HTTP interface

Built-in REST API Server

Powered by httplib.h, the server exposes endpoints:

POST /insert

Insert a row:

{
  "id": 1,
  "username": "alice",
  "email": "alice@example.com"
}

GET /select

Returns all rows:

[
  {
    "id": 1,
    "username": "alice",
    "email": "alice@example.com"
  }
]

Optional React Frontend

A simple UI to:

Add users

Display database rows

Interact with the C++ backend in real-time

Bundled React build files are served directly by the C++ server for easy deployment.

Project Structure
minidb/
│
├── src/
│   ├── engine/
│   │   ├── engine_core.h
│   │   ├── engine_core.cpp
│   │   ├── engine_api.h
│   │   └── engine_api.cpp
│   ├── server.cpp
│   └── CMakeLists.txt
│
├── third_party/
│   ├── httplib.h
│   └── nlohmann/
│       └── json.hpp
│
├── client/   ← React frontend (optional)
│   ├── package.json
│   ├── src/
│   └── build/
│
├── CMakeLists.txt  ← Top-level
└── README.md

Build & Run (C++ Backend)
1. Configure & Build
mkdir build
cd build
cmake ..
cmake --build .

2. Run the server
./minidb


Server runs at:

http://localhost:8080

Build & Integrate React Frontend (Optional)
1. Inside /client:
npm install
npm run build

2. Copy React build into backend folder

Configure C++ server to serve:

client/build/index.html
client/build/static/*

3. Access UI:
http://localhost:8080

🛠 Technologies Used
Backend

C++17

httplib.h (HTTP server)

nlohmann/json (JSON handling)

CMake

Custom database engine

Frontend

React

Axios (API calls)

Tailwind / basic UI components

How It Works (Architecture Overview)
1. engine_core

Implements:

Row definitions

Storage vectors

File read/write

Query execution

2. engine_api

Exposes a clean, safe interface:

engine.insert(id, username, email);
auto rows = engine.select_all();

3. server.cpp

Turns the DB engine into a REST API:

/insert

/select

Serves React build files

Learning Outcomes:

By building this project you demonstrate:

Systems programming in C++

Modern API design

Clean multi-module architecture

Memory management & serialization

Full-stack integration (C++ backend + React frontend)

Deployment-ready project structure

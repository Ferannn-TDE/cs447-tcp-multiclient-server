# cs447-tcp-multiclient-server
# CS 447 - Multi-Client TCP Socket Server (Project I)

A multi-threaded TCP server built with Winsock for Windows, developed for
CS 447 - Networks and Data Communication (SIUE, Prof. Fujinoki).

The server accepts connections from up to 3 clients simultaneously (one
worker thread per connection), reads each client's ID, and replies with
a timestamp — unless the client's IP matches a configured "prohibited
client," in which case it returns an access-denied message instead.

## Author
- **Name:** Oluwaferanmi Odedairo

## How it works
1. Server starts, binds to a port (default `1050`), and listens for connections.
2. Each incoming connection is handed off to a new worker thread via `CreateThread`, so multiple clients can be served at once.
3. The client sends its ID as its first message.
4. The server logs the client's IP, source port, and ID, then:
   - If the client's IP matches `PROHIBITED_CLIENT`, it replies with `"Your access is denied by this server!"`
   - Otherwise, it replies with `"<client ID> <HH:MM:SS>"` — the timestamp of when the ID arrived.
5. The server waits 1 second, closes that connection, and the thread exits. The main loop immediately goes back to `accept()`, so the server stays available for new clients at all times.
6. The provided `client.cpp` (unmodified, instructor-supplied) repeats this process 15 times per client before terminating.

## Build requirements
- Windows OS
- Microsoft Visual Studio (Console App project type)
- Winsock (`ws2_32.lib`) — linked automatically via `#pragma comment(lib, "ws2_32.lib")`, no manual project setting needed

## Building
1. Open Visual Studio → create a new **Console App** (C++) project.
2. Add `server_548.cpp` to the project.
3. Build (Debug or Release, x86 or x64 — either works).

## Running

server_548.exe [port]

- `port` is optional; defaults to `1050` if omitted or invalid.

Example:

server_548.exe 1050


To connect a client:

client.exe <client-ID> <server-IP> <server-port>

Example:

client.exe 1 127.0.0.1 1050


## Configuration
The prohibited client's IP is set via `#define` near the top of `server_548.cpp`:
```cpp
#define PROHIBITED_CLIENT "192.168.1.99"
```
Change this value and rebuild to block a different IP.

## Notes
- `client.cpp` is included for reference/build purposes only and was provided
  by the course instructor — it is not authored by me and was not modified.
- Server supports up to 3 concurrent clients as required by the project spec, though the threading model isn't hard-capped at 3 — it scales with `CreateThread` per connection.

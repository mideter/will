# Will

Will is a C++ TCP messenger with a dedicated server and chat client.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

Open two terminals:

1. Start server:
```bash
./build/will-server
```
2. Run client:
```bash
./build/will-client
```

Expected flow:
- start `will-server` in one terminal
- start `will-client` in another terminal
- type messages in client and see incoming messages from peer in real time


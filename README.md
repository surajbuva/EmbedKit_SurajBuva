# EmbedKit_SurajBuva

**Candidate:** Suraj Buva

## Module: Ring Buffer (`ringbuf.c`)

A fixed-capacity, 8-byte circular FIFO buffer for `uint8_t` values. It supports initialization, byte writes and reads, count queries, and full/empty checks. Writes to a full buffer and reads from an empty buffer fail safely.

The implementation uses bitwise AND for index wrap-around. Since `BUFFER_SIZE` is 8 (a power of two), `index & (BUFFER_SIZE - 1)` wraps the index into the valid range. On microcontrollers without a hardware divider, this can be faster than modulo. The technique requires a power-of-two buffer size.

## Requirements

- C99-compatible compiler (GCC recommended)
- No external libraries or embedded hardware required

## Build and run

```bash
gcc -Wall -std=c99 ringbuf.c -o ringbuf
./ringbuf
```

The program demonstrates filling the buffer, rejecting an extra write, reading three bytes, reusing freed slots, draining the buffer in FIFO order, and rejecting a read when empty.

## Expected FIFO behavior

- Initial writes: `0x41` through `0x48`
- Write `0x99` while full: fails
- First three reads: `0x41`, `0x42`, `0x43`
- New writes: `0x49`, `0x4A`, `0x4B`
- Remaining reads: `0x44` through `0x4B`
- Final read from empty buffer: fails

/*
 * Embed Kit: Ring Buffer Module
 * Candidate: Suraj Buva
 *
 * Fixed-capacity FIFO ring buffer for uint8_t data.
 * Build: gcc -Wall -std=c99 ringbuf.c -o ringbuf
 */

#include <stdio.h>
#include <stdint.h>

#define BUFFER_SIZE 8U
#define BUFFER_MASK (BUFFER_SIZE - 1U)

typedef struct
{
    uint8_t data[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} RingBuffer;

typedef enum
{
    RINGBUF_OK = 0,
    RINGBUF_ERROR = 1
} RingBufferStatus;

/* BUFFER_SIZE is 8, which is a power of two.
 * Therefore, index & (BUFFER_SIZE - 1) wraps the index around to 0..7.
 * A bitwise AND is typically cheaper than modulo on MCUs without a hardware
 * divider. This optimization only works for power-of-two buffer sizes.
 */
static void ringbuf_init(RingBuffer *buffer)
{
    uint8_t i;

    for (i = 0U; i < BUFFER_SIZE; ++i)
    {
        buffer->data[i] = 0U;
    }

    buffer->head = 0U;
    buffer->tail = 0U;
    buffer->count = 0U;
}

static RingBufferStatus ringbuf_write(RingBuffer *buffer, uint8_t value)
{
    if (buffer->count >= BUFFER_SIZE)
    {
        return RINGBUF_ERROR;
    }

    buffer->data[buffer->head] = value;
    buffer->head = (uint8_t)((buffer->head + 1U) & BUFFER_MASK);
    ++buffer->count;

    return RINGBUF_OK;
}

static RingBufferStatus ringbuf_read(RingBuffer *buffer, uint8_t *value)
{
    if ((buffer->count == 0U) || (value == NULL))
    {
        return RINGBUF_ERROR;
    }

    *value = buffer->data[buffer->tail];
    buffer->tail = (uint8_t)((buffer->tail + 1U) & BUFFER_MASK);
    --buffer->count;

    return RINGBUF_OK;
}

static uint8_t ringbuf_count(const RingBuffer *buffer)
{
    return buffer->count;
}

static uint8_t ringbuf_is_full(const RingBuffer *buffer)
{
    return (uint8_t)(buffer->count == BUFFER_SIZE);
}

static uint8_t ringbuf_is_empty(const RingBuffer *buffer)
{
    return (uint8_t)(buffer->count == 0U);
}

int main(void)
{
    RingBuffer buffer;
    uint8_t value;
    uint8_t i;
    const uint8_t initial_data[BUFFER_SIZE] =
        {0x41U, 0x42U, 0x43U, 0x44U, 0x45U, 0x46U, 0x47U, 0x48U};
    const uint8_t extra_data[3U] = {0x49U, 0x4AU, 0x4BU};

    ringbuf_init(&buffer);
    printf("Initial state: count=%u, empty=%s, full=%s\n",
           (unsigned int)ringbuf_count(&buffer),
           ringbuf_is_empty(&buffer) ? "YES" : "NO",
           ringbuf_is_full(&buffer) ? "YES" : "NO");

    /* 1. Fill the buffer with A through H. */
    for (i = 0U; i < BUFFER_SIZE; ++i)
    {
        if (ringbuf_write(&buffer, initial_data[i]) == RINGBUF_OK)
        {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n",
                   (unsigned int)initial_data[i],
                   (unsigned int)ringbuf_count(&buffer),
                   ringbuf_is_full(&buffer) ? " FULL" : "");
        }
        else
        {
            printf("[WRITE] 0x%02X -> FAIL\n", (unsigned int)initial_data[i]);
        }
    }

    printf("Buffer full check: %s; count=%u\n",
           ringbuf_is_full(&buffer) ? "FULL" : "NOT FULL",
           (unsigned int)ringbuf_count(&buffer));

    /* 2. A write to a full buffer must fail without overwriting data. */
    if (ringbuf_write(&buffer, 0x99U) == RINGBUF_ERROR)
    {
        printf("[WRITE] 0x99 -> FAIL (buffer full)\n");
    }

    /* 3. Read the first three bytes. */
    for (i = 0U; i < 3U; ++i)
    {
        if (ringbuf_read(&buffer, &value) == RINGBUF_OK)
        {
            printf("[READ]    -> 0x%02X (count=%u)\n",
                   (unsigned int)value,
                   (unsigned int)ringbuf_count(&buffer));
        }
    }
    printf("Count after 3 reads: %u (expected 5)\n",
           (unsigned int)ringbuf_count(&buffer));

    /* 4. Write I, J, K into the slots freed by the reads. */
    for (i = 0U; i < 3U; ++i)
    {
        if (ringbuf_write(&buffer, extra_data[i]) == RINGBUF_OK)
        {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n",
                   (unsigned int)extra_data[i],
                   (unsigned int)ringbuf_count(&buffer),
                   ringbuf_is_full(&buffer) ? " FULL" : "");
        }
        else
        {
            printf("[WRITE] 0x%02X -> FAIL\n", (unsigned int)extra_data[i]);
        }
    }

    printf("Count after 3 new writes: %u (expected 8)\n",
           (unsigned int)ringbuf_count(&buffer));

    /* 5. Read all eight remaining bytes in FIFO order. */
    for (i = 0U; i < BUFFER_SIZE; ++i)
    {
        if (ringbuf_read(&buffer, &value) == RINGBUF_OK)
        {
            printf("[READ]    -> 0x%02X (count=%u)\n",
                   (unsigned int)value,
                   (unsigned int)ringbuf_count(&buffer));
        }
    }

    printf("Final empty check: %s; count=%u\n",
           ringbuf_is_empty(&buffer) ? "EMPTY" : "NOT EMPTY",
           (unsigned int)ringbuf_count(&buffer));

    /* 6. A read from an empty buffer must fail. */
    if (ringbuf_read(&buffer, &value) == RINGBUF_ERROR)
    {
        printf("[READ] (empty) -> FAIL (buffer empty)\n");
    }

    return 0;
}

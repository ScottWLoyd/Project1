
#define KILOBYTES(b) ((b) * 1024)
#define MEGABYTES(b) ((b) * 1024 * 1024)

static void* xcalloc(size_t num_items, size_t item_size)
{
    void* ptr = calloc(num_items, item_size);
    if (!ptr)
    {
        perror("xcalloc failed");
        exit(1);
    }
    return ptr;
}

static void* xrealloc(void* ptr, size_t num_bytes)
{
    ptr = realloc(ptr, num_bytes);
    if (!ptr)
    {
        perror("xrealloc failed");
        exit(1);
    }
    return ptr;
}

static void* xmalloc(size_t num_bytes)
{
    void* ptr = malloc(num_bytes);
    if (!ptr)
    {
        perror("xmalloc failed");
        exit(1);
    }
    return ptr;
}

static void* memdup(void* src, size_t size)
{
    void* dest = xmalloc(size);
    memcpy(dest, src, size);
    return dest;
}

typedef struct MemoryArena {
    char* base;
    char* next;
    size_t size;
} MemoryArena;

static void InitializeMemoryArena(MemoryArena* arena, size_t size)
{
    arena->base = xmalloc(size);
    arena->next = arena->base;
    arena->size = size;
}

#define push_struct(a, s) push_size(a, sizeof(s))

static void* push_size(MemoryArena* arena, size_t size)
{
    void* ptr = arena->next;
    assert(arena->next + size < arena->base + arena->size);
    arena->next += size;
    return ptr;
}

#define zero_struct(s) zero_size(&(s), sizeof(s));

static void zero_size(void* ptr, size_t size)
{
    uint8_t* byte = (uint8_t*)ptr;
    while (size--)
    {
        *byte++ = 0;
    }
}

typedef struct FileContents {
    size_t len;
    void* data;
    bool success;
} FileContents;

static FileContents ReadEntireFile(char* file_path)
{
    FileContents result = { 0 };

    FILE* file = fopen(file_path, "r");
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);

    result.data = malloc(len);
    result.len = fread(result.data, 1, len, file);
    result.success = true;
    return result;
}

// Stretchy buffers
typedef struct BufHdr
{
    size_t len;
    size_t cap;
    char buf[0];
} BufHdr;

#define buf__hdr(b) ((BufHdr*)((char*)(b) - offsetof(BufHdr, buf)))

#define buf_len(b)  ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b)  ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b)  ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? free(buf__hdr(b)), (b) = NULL : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)

void* buf__grow(const void* buf, size_t new_len, size_t elem_size)
{
    assert(buf_cap(buf) <= (SIZE_MAX - 1) / 2);
    size_t new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf)) / elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap * elem_size;
    BufHdr* new_hdr;
    if (buf) {
        new_hdr = xrealloc(buf__hdr(buf), new_size);
    }
    else {
        new_hdr = xmalloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

char* buf__printf(char* buf, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (buf_len(buf) == 0)
    {
        n++;
    }
    buf_fit(buf, n + buf_len(buf));
    char* dest = buf_len(buf) == 0 ? buf : buf + buf_len(buf) - 1;
    va_start(args, fmt);
    vsnprintf(dest, buf + buf_cap(buf) - dest, fmt, args);
    va_end(args);
    buf__hdr(buf)->len += n;
    return buf;
}
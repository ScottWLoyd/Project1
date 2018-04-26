
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

static void InitializeMemoryArena(MemoryArena* arena, size_t size)
{
    arena->base = (char*)xmalloc(size);
    arena->next = arena->base;
    arena->size = size;
}

static size_t arena_size_remaining(MemoryArena* arena)
{
    return arena->size - (arena->next - arena->base);
}

#define push_struct(a, s) (s*)push_size(a, sizeof(s))

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


static FileContents ReadEntireFile(char* file_path)
{
    FileContents result = { 0 };

    FILE* file = fopen(file_path, "rb");
    if (!file)
    {
        assert(!"Unable to find file! Check path");
    }
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    rewind(file);

    result.data = calloc(1, len);
    result.len = fread(result.data, 1, len, file);
    assert(result.len == len);
    assert(!ferror(file));
    result.success = true;

    fclose(file);

    return result;
}

#if 0
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
#endif

inline BitScanResult find_least_significant_set_bit(uint32_t value)
{
    BitScanResult result = {0};

    for (uint32_t test = 0; test < 32; test++)
    {
        if (value & (1 << test))
        {
            result.index = test;
            result.found = true;
            break;
        }
    }

    return result;
}

inline uint32_t rotate_left(uint32_t value, int32_t shift)
{
#if COMPILER_MSVC
    uint32_t result = _rotl(value, shift);
#else
    // TODO(scott): test this!
    shift &= 31;
    uint32_t result = ((value << shift) | (value >> (32 - shift)));
#endif

    return result;
}
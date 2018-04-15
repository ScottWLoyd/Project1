
#define KILOBYTES(b) ((b) * 1024)
#define MEGABYTES(b) ((b) * 1024 * 1024)
typedef struct MemoryArena {
    char* base;
    char* next;
    size_t size;
} MemoryArena;

typedef struct FileContents {
    size_t len;
    void* data;
    bool success;
} FileContents;

// Stretchy buffers
typedef struct BufHdr
{
    size_t len;
    size_t cap;
#ifdef __cplusplus
    char* buf;
#else
    char buf[0];
#endif
} BufHdr;

typedef struct BitScanResult {
    bool found;
    uint32_t index;
} BitScanResult;

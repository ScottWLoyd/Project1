
#define KILOBYTES(b) ((b) * 1024)
#define MEGABYTES(b) ((b) * 1024 * 1024)


struct MemoryArena {
    char* base;
    char* next;
    size_t size;
};

struct FileContents {
    size_t len;
    void* data;
    bool success;
};

#if 0
// Stretchy buffers
#ifndef __cplusplus
typedef 
#endif
struct BufHdr
{
    size_t len;
    size_t cap;
#ifdef __cplusplus
    char* buf;
#else
    char buf[0];
#endif
} 
#ifndef __cplusplus
BufHdr
#endif
;
#endif

struct BitScanResult {
    bool found;
    uint32_t index;
};

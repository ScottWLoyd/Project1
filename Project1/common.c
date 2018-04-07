
typedef int bool;
#define true (1==1)
#define false (1==0)

typedef struct FileContents {
    size_t len;
    void* data;
    bool success;
} FileContents;

FileContents ReadEntireFile(char* file_path)
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

#pragma pack(push, 1)
typedef struct DibHeader {
    uint32_t header_size;
    uint32_t bitmap_width;
    uint32_t bitmap_height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t horizontal_resolution; // pixel/meter
    int32_t vertical_resolution; // pixel/meter
    uint32_t num_colors;
    uint32_t important_colors;
} DibHeader;

typedef struct BitmapHeader {
    char bmp_identifier[2];
    uint32_t size_in_bytes;
    char reserved[4];
    uint32_t pixel_offset;
} BitmapHeader;

typedef struct Bitmap {
    size_t width;
    size_t height;
    void* pixels;   // 32 bit pixels, ARGB
} Bitmap;
#pragma pack(pop)

Bitmap Win32LoadBitmap(char* file_path)
{
    Bitmap result = { 0 };
    FileContents contents = ReadEntireFile(file_path);
    if (contents.success)
    {
        char* ptr = contents.data;
        BitmapHeader* header = (BitmapHeader*)ptr;
        ptr += sizeof(BitmapHeader);
        DibHeader* dib_header = (DibHeader*)ptr;

        ptr += dib_header->header_size;

        result.width = dib_header->bitmap_width;
        result.height = dib_header->bitmap_height;
        result.pixels = ptr;
    }

    return result;
}

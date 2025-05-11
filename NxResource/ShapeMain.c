#include "NxResource.h"
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <glib.h>

#include <zlib.h>

// Included in generated C source by fut, but not here.
typedef struct {
	bool (*isStreamEnd)(const ZlibInterface *self);
	void (*setWindowBits)(ZlibInterface *self, int windowBits);
	void (*setInput)(ZlibInterface *self, uint8_t const *input, int offset, int length);
	int (*inflate)(ZlibInterface *self, uint8_t *output, int offset, int length);
	void (*finalize)(ZlibInterface *self);
} ZlibInterfaceVtbl;
struct ZlibInterface {
	const ZlibInterfaceVtbl *vtbl;
};

// Zlib implementation class.
// NOTE: When porting to languages supporting classes,
// simply make a class inherited from ZlibInterface.
typedef struct ZlibInflatorImpl {
    // Keep the base in the inherited class.
    ZlibInterface base;

	bool isStreamEnd;
	z_stream strm;
} ZlibInflatorImpl;

// Class methods.
bool ZlibInflatorImpl_IsStreamEnd(const ZlibInflatorImpl *self) {
	return self->isStreamEnd;
}
void ZlibInflatorImpl_SetWindowBits(ZlibInflatorImpl *self, int windowBits) {
    inflateReset2(&self->strm, windowBits);
}
void ZlibInflatorImpl_SetInput(ZlibInflatorImpl *self, uint8_t const *input, int offset, int length) {
    self->strm.next_in = (Bytef *) input + offset;
    self->strm.avail_in = length;
}
int ZlibInflatorImpl_Inflate(ZlibInflatorImpl *self, uint8_t *output, int offset, int length) {
    self->strm.next_out = (Bytef *) output + offset;
    self->strm.avail_out = length;

    self->isStreamEnd = false; // Clear old state.

    int ret = inflate(&self->strm, Z_NO_FLUSH);
    if (ret == Z_STREAM_END) {
        self->isStreamEnd = true;
        // Fall through to return the number of bytes written.
    } else if (ret != Z_OK) {
        return ret;
    }

    return length - self->strm.avail_out;
}
void ZlibInflatorImpl_Finalize(ZlibInflatorImpl *self) {
	 inflateEnd(&self->strm);
}

// Constructor for class.
static void ZlibInflatorImpl_Construct(ZlibInflatorImpl *self) {
    // Vtable for base class.
	static const ZlibInterfaceVtbl vtbl = {
        // IsStreamEnd
		(bool (*)(const ZlibInterface *self)) ZlibInflatorImpl_IsStreamEnd,
        // SetWindowBits
		(void (*)(ZlibInterface *self, int windowBits)) ZlibInflatorImpl_SetWindowBits,
        // SetInput
		(void (*)(ZlibInterface *self, uint8_t const *input, int offset, int length)) ZlibInflatorImpl_SetInput,
        // Inflate
		(int (*)(ZlibInterface *self, uint8_t *output, int offset, int length)) ZlibInflatorImpl_Inflate,
        // Finalize
		(void (*)(ZlibInterface *self)) ZlibInflatorImpl_Finalize,
	};
    // Set vtable.
	self->base.vtbl = &vtbl;

    // Constructor. Initialize fields.
    memset(&self->strm, 0, sizeof(z_stream));
    self->isStreamEnd = false;
    // Default window bits as 15 for raw deflate / use 15+32 for gzip support
    inflateInit2(&self->strm, 15);
}

typedef struct nnmiiResourceHeaderCommonParam {
	int signature;
	int version;
	int fileSize;
} nnmiiResourceHeaderCommonParam;

int main(int argc, char *argv[]) {
    // static_assert(ResourceShapeHeader_SIZE == sizeof(ResourceShapeHeader)); // Does not match

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <shape_resource_file> [glb_filename] [resource_shape_type] [resource_shape_index]\n", argv[0]);
        return 1;
    }

    // Get file path from argv.
    const char *filepath = argv[1];
    // Open the file.
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Allocate the size of the file header.
    // This buffer will be grown to the size of the entire file.
    unsigned char* data = malloc(ResourceShapeHeader_SIZE);

    // Read the file.
    size_t read_size = fread(data, ResourceShapeHeader_SIZE, 1, file);
    if (read_size != 1) {
        fprintf(stderr, "Error reading header from file\n");
        return 1;
    }

    // Get the whole file size. You may get the file size from the filesystem instead.
    uint32_t file_size = ((nnmiiResourceHeaderCommonParam*)data)->fileSize;
    fprintf(stderr, "file size: %d\n", file_size);
    uint32_t file_extra_read_size = file_size - ResourceShapeHeader_SIZE;

    // Re-allocate data.
    data = realloc(data, file_size);
    // Read the remaining data.
    read_size = fread(data + ResourceShapeHeader_SIZE, file_extra_read_size, 1, file);
    if (read_size != 1) {
        fprintf(stderr, "Error reading content from file\n");
        return 1;
    }
    fclose(file); // Close the file.

    ZlibInflatorImpl inflator;
    ZlibInflatorImpl_Construct(&inflator);

    // Run test in main.
    // Main_ReadTest(data, &inflator.base);
/*    PData* pData = Main_ExportTextureTest(argc, argv, data, &inflator.base);

    char* tex_filename = "texture.ktx";
    if (argc > 4) tex_filename = argv[2];
    if (pData != NULL) {
        printf("outputting texture to %s\n", tex_filename);
        // Open texture.ktx for writing.
        FILE* f = fopen(tex_filename, "wb");
        if (!f) {
            perror("fopen");
            return 1;
        }
        printf("data: %p, size: %d\n", PData_GetData(pData), PData_GetSize(pData));
        fwrite(PData_GetData(pData), PData_GetSize(pData), 1, f);
        fclose(f); // Close file - finish writing.
    }
*/
    // Call export method - data has to be freed with g_array_unref.

    GArray* glb_data = Main_ExportTest(argc, argv, data, &inflator.base);

    char* glb_filename = "shape.glb";
    if (argc > 4) glb_filename = argv[2];
    if (glb_data != NULL) {
        printf("outputting shape to %s\n", glb_filename);
        // Open cube.glb for writing.
        FILE* f = fopen(glb_filename, "wb");
        if (!f) {
            perror("fopen");
            return 1;
        }
        // Write data at GArray.data, with GArray.len bytes.
        fwrite(glb_data->data, 1, glb_data->len, f);
        fclose(f); // Close file - finish writing.
    }


    ZlibInflatorImpl_Finalize(&inflator);
    // Free data (allocated by ourselves).
    free(data);

    return 0;
}

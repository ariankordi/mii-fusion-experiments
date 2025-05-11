// main.c
#include <stdio.h>
#include <stdlib.h>
// #include "GLBExporter.h"  // from fut
#include "GLBExporter2.h"

int main(void) {
    // Construct CubeExporter class.
    GLBExporter* exporter = GLBExporter_New();

    // Add cube mesh.
    GLBExporter_AddCubeMesh(exporter);
    // Call export method - data has to be freed with g_array_unref.
    GArray* data = GLBExporter_Export(exporter);

    // Open cube.glb for writing.
    FILE* f = fopen("cube.glb", "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    // Write data at GArray.data, with GArray.len bytes.
    fwrite(data->data, 1, data->len, f);
    fclose(f); // Close file - finish writing.

    // Clean up class and returned output.
    GLBExporter_Delete(exporter);
    g_array_unref(data);

    return 0;
}

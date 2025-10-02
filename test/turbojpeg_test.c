#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "turbojpeg.h"

/* 2x2 RGB pixels */
static const unsigned char kPixels[2 * 2 * 3] = {
  255,0,0,   0,255,0,
  0,0,255,   255,255,255
};

int main(void) {
    tjhandle compressor = tjInitCompress();
    if (!compressor) {
        fprintf(stderr, "tjInitCompress failed: %s\n", tjGetErrorStr());
        return 1;
    }

    unsigned char* jpegBuf = NULL;
    unsigned long jpegSize = 0;

    /* Compress */
    if (tjCompress2(compressor,
        kPixels, 2, 0, 2, TJPF_RGB,
        &jpegBuf, &jpegSize,
        TJSAMP_444, 90, TJFLAG_FASTDCT) != 0) {
        fprintf(stderr, "tjCompress2 failed: %s\n", tjGetErrorStr());
        tjDestroy(compressor);
        return 2;
    }

    tjDestroy(compressor);

    /* Decompress */
    tjhandle decompressor = tjInitDecompress();
    if (!decompressor) {
        fprintf(stderr, "tjInitDecompress failed: %s\n", tjGetErrorStr());
        tjFree(jpegBuf);
        return 3;
    }

    int width, height, jpegSubsamp, jpegColorspace;
    if (tjDecompressHeader3(decompressor, jpegBuf, jpegSize,
        &width, &height, &jpegSubsamp, &jpegColorspace) != 0) {
        fprintf(stderr, "tjDecompressHeader3 failed: %s\n", tjGetErrorStr());
        tjDestroy(decompressor);
        tjFree(jpegBuf);
        return 4;
    }

    unsigned char out[2 * 2 * 3] = { 0 };
    if (tjDecompress2(decompressor, jpegBuf, jpegSize,
        out, width, 0, height, TJPF_RGB, TJFLAG_FASTDCT) != 0) {
        fprintf(stderr, "tjDecompress2 failed: %s\n", tjGetErrorStr());
        tjDestroy(decompressor);
        tjFree(jpegBuf);
        return 5;
    }

    tjDestroy(decompressor);
    tjFree(jpegBuf);

    /* Very loose sanity check: sum bytes */
    unsigned long sum = 0;
    for (size_t i = 0; i < sizeof(out); ++i) sum += out[i];
    if (sum == 0) {
        fprintf(stderr, "TurboJPEG decode produced all zeros\n");
        return 6;
    }

    printf("TurboJPEG API test OK (checksum=%lu)\n", sum);
    return 0;
}

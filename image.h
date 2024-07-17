#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

enum allocation_type {
    NO_ALLOCATION, SELF_ALLOCATED, STB_ALLOCATED
};
enum koordinata{
    X,
    y
};

typedef struct {
    int width;
    int height;
    int channels;
    uint8_t *data;
    size_t size;
    enum allocation_type allocation_;
} Image;
typedef struct Pixels {
    uint8_t red, green, blue, Alpha;
}Pixels;

void Image_load(Image *img, const char *fname);
void Image_create(Image *img, int width, int height, int channels, bool zeroed);
void Image_save(const Image *img, const char *fname);
void Image_free(Image *img);
void Image_to_gray(const Image *orig, Image *gray);
void Gamma(const Image *orig, Image *gamma, double ertek);
void Image_to_blur(const Image *orig, Image *blur);
void Blur(const Image *orig, Image *blur, int kernel_size);
void Image_to_sharp(const Image *orig, Image *sharp);
void line_detection(const Image *orig, Image* Gx, Image* Gy, Image * Line_det);

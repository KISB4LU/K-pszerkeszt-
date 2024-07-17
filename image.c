#include "Image.h"
#include "utils.h"
#include <math.h>
//#include "debugmalloc.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

void Image_load(Image *img, const char *fname) {
    if((img->data = stbi_load(fname, &img->width, &img->height, &img->channels, 0)) != NULL) {
        img->size = img->width * img->height * img->channels;
        img->allocation_ = STB_ALLOCATED;
    }
}


void Image_create(Image *img, int width, int height, int channels, bool zeroed) {
    size_t size = width * height * channels;
    if(zeroed) {
        img->data = calloc(size, 1);
    } else {
        img->data = malloc(size);
    }

    if(img->data != NULL) {
        img->width = width;
        img->height = height;
        img->size = size;
        img->channels = channels;
        img->allocation_ = SELF_ALLOCATED;
    }
}

void Image_save(const Image *img, const char *fname) {
    // Check if the file name ends in one of the .jpg/.JPG/.jpeg/.JPEG or .png/.PNG
    if(str_ends_in(fname, ".jpg") || str_ends_in(fname, ".JPG") || str_ends_in(fname, ".jpeg") || str_ends_in(fname, ".JPEG")) {
        stbi_write_jpg(fname, img->width, img->height, img->channels, img->data, 100);
    } else if(str_ends_in(fname, ".png") || str_ends_in(fname, ".PNG")) {
        stbi_write_png(fname, img->width, img->height, img->channels, img->data, img->width * img->channels);
    } else {
        ON_ERROR_EXIT(false, "");
    }
}

void Image_free(Image *img) {
    if(img->allocation_ != NO_ALLOCATION && img->data != NULL) {
        if(img->allocation_ == STB_ALLOCATED) {
            stbi_image_free(img->data);
        } else {
            free(img->data);
        }
        img->data = NULL;
        img->width = 0;
        img->height = 0;
        img->size = 0;
        img->allocation_ = NO_ALLOCATION;
    }
}

void Image_to_gray(const Image *orig, Image *gray) {
    ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    int channels = orig->channels == 4 ? 2 : 1;
    Image_create(gray, orig->width, orig->height, channels, false);
    ON_ERROR_EXIT(gray->data == NULL, "Error in creating the image");

    for(unsigned char *p = orig->data, *pg = gray->data; p != orig->data + orig->size; p += orig->channels, pg += gray->channels) {
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2))/3.0);
        if(orig->channels == 4) {
            *(pg + 1) = *(p + 3);
        }
    }
}


void Gamma(const Image *orig, Image *gamma, double ertek) {
    ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    Image_create(gamma, orig->width, orig->height, orig->channels, false);
    ON_ERROR_EXIT(gamma->data == NULL, "Error in creating the image");

    // Sepia filter coefficients from https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created
    for(unsigned char *p = orig->data, *pg = gamma->data; p != orig->data + orig->size; p += orig->channels, pg += gamma->channels) {
        *pg       = (uint8_t) fmin(*p * ertek,255.0);
        *(pg + 1) = (uint8_t) fmin(*(p+1) * ertek, 255.0);
        *(pg + 2) = (uint8_t) fmin(*(p+2) * ertek, 255.0);
    }
}
void gauss_blur(Image *blur, const Image *orig, Pixels** matrix){
    double sum = 0;
    unsigned char *pg = blur->data;
    //printf("\nchannels: %d\n", blur->channels);
    for(int i = 1; i< orig->height-1; i++){
        for(int j = 1; j < orig->width-1; j++){
            sum = 0;
            sum +=   matrix[i-1][j-1].red+2*matrix[i-1 ][j].red+  matrix[i-1][j+1].red;
            sum += 2*matrix[i][j-1].red+  4*matrix[i][j].red+  2*matrix[i][j+1].red;
            sum +=   matrix[i+1][j-1].red+2*matrix[i+1][j].red+  matrix[i+1][j+1].red;
            //printf("!%d! %d", sum, uint8_t) sum/16.0);
            sum = sum/16.0;
            *pg = (uint8_t) sum;

            sum = 0;
            sum +=   matrix[i-1][j-1].green+2*matrix[i-1][j].green+  matrix[i-1][j+1].green;
            sum += 2*matrix[i][j-1].green+  4*matrix[i][j].green+  2*matrix[i][j+1].green;
            sum +=   matrix[i+1][j-1].green+2*matrix[i+1][j].green+  matrix[i+1][j+1].green;
            sum = sum/16.0;
            *(pg+1) = (uint8_t) sum;

            sum = 0;
            sum +=   matrix[i-1][j-1].blue+2*matrix[i-1][j].blue+  matrix[i-1][j+1].blue;
            sum += 2*matrix[i][j-1].blue+  4*matrix[i][j].blue+  2*matrix[i][j+1].blue;
            sum +=   matrix[i+1][j-1].blue+2*matrix[i+1][j].blue+  matrix[i+1][j+1].blue;
            sum = sum/16.0;
            *(pg+2) = (uint8_t) sum;
            if(blur->channels == 4)
                *(pg+3) = matrix[i][j].Alpha;

            pg+=blur->channels;
        }
    }
}
int MinMax(int num){

    if(num < 0)
        return 0;

    if(num > 255)
        return 255;

    return num;
}
void convolution_sharpnes(Image *blur, const Image *orig, Pixels** matrix){

    int sum = 0;
    unsigned char *pg = blur->data;
    //printf("\nchannels: %d\n", blur->channels);
    for(int i = 1; i< orig->height-1; i++){
        for(int j = 1; j < orig->width-1; j++){
            sum = 0;
            sum += 0*matrix[i-1][j-1].red +(-1*matrix[i-1 ][j].red)+  0*matrix[i-1][j+1].red;
            sum += (-1*matrix[i][j-1].red)+ 5*matrix[i][j].red     +  (-1*matrix[i][j+1].red);
            sum += 0*matrix[i+1][j-1].red +(-1*matrix[i+1][j].red) +  0*matrix[i+1][j+1].red;
            *pg = (uint8_t) MinMax(sum);

            sum = 0;
            sum += 0*matrix[i-1][j-1].green +(-1*matrix[i-1 ][j].green)+  0*matrix[i-1][j+1].green;
            sum += (-1*matrix[i][j-1].green)+ 5*matrix[i][j].green     +  (-1*matrix[i][j+1].green);
            sum += 0*matrix[i+1][j-1].green +(-1*matrix[i+1][j].green) +  0*matrix[i+1][j+1].green;
            *(pg+1) = (uint8_t) MinMax(sum);

            sum = 0;
            sum += 0*matrix[i-1][j-1].blue +(-1*matrix[i-1 ][j].blue)+  0*matrix[i-1][j+1].blue;
            sum += (-1*matrix[i][j-1].blue)+ 5*matrix[i][j].blue     +  (-1*matrix[i][j+1].blue);
            sum += 0*matrix[i+1][j-1].blue +(-1*matrix[i+1][j].blue) +  0*matrix[i+1][j+1].blue;
            *(pg+2) = (uint8_t) MinMax(sum);
            if(blur->channels == 4)
                *(pg+3) = matrix[i][j].Alpha;

            pg+=blur->channels;
        }
    }
}
void Image_to_sharp(const Image *orig, Image *sharp) {
    ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    Image_create(sharp, orig->width-2, orig->height-2, orig->channels, false);
    ON_ERROR_EXIT(sharp->data == NULL, "Error in creating the image");

    Pixels** Pixel_matrix = (Pixels**)malloc(sizeof(Pixels*)*orig->height);
    for(int i = 0; i<orig->height; i++){
        Pixel_matrix[i] = (Pixels*)malloc(sizeof(Pixels)*orig->width);
    }

    unsigned char *p = orig->data;
    for(int i = 0; i< orig->height; i++){
        for(int j = 0; j < orig->width; j++){
            Pixel_matrix[i][j].red =(uint8_t) *p;
            Pixel_matrix[i][j].green =(uint8_t) *(p+1);
            Pixel_matrix[i][j].blue = (uint8_t)*(p+2);

            if(orig->channels == 4){
                Pixel_matrix[i][j].Alpha = (uint8_t)*(p+3);
            }
            p+= orig->channels;
        }
    }
    convolution_sharpnes(sharp, orig, Pixel_matrix);
    for(int i = 0; i< orig->height; i++){
        free(Pixel_matrix[i]);
     }
     free(Pixel_matrix);
}

void Image_to_blur(const Image *orig, Image *blur) {
    ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    Image_create(blur, orig->width-2, orig->height-2, orig->channels, false);
    ON_ERROR_EXIT(blur->data == NULL, "Error in creating the image");

    Pixels** Pixel_matrix = (Pixels**)malloc(sizeof(Pixels*)*orig->height);
    for(int i = 0; i<orig->height; i++){
        Pixel_matrix[i] = (Pixels*)malloc(sizeof(Pixels)*orig->width);
    }

    unsigned char *p = orig->data;
    for(int i = 0; i< orig->height; i++){
        for(int j = 0; j < orig->width; j++){
            Pixel_matrix[i][j].red =(uint8_t) *p;
            Pixel_matrix[i][j].green =(uint8_t) *(p+1);
            Pixel_matrix[i][j].blue = (uint8_t)*(p+2);

            if(orig->channels == 4){
                Pixel_matrix[i][j].Alpha = (uint8_t)*(p+3);
            }
            p+= orig->channels;
        }
    }

    //printf("matrix[0][0] = %d %d %d", Pixel_matrix[orig->height-1][orig->width-1].red , Pixel_matrix[orig->height-1][orig->width-1].green , Pixel_matrix[orig->height-1][orig->width-1].blue);
    gauss_blur(blur, orig, Pixel_matrix);
     for(int i = 0; i< orig->height; i++){
        free(Pixel_matrix[i]);
     }
     free(Pixel_matrix);
}

void simple_blur(const Image *orig, Image *blur, Pixels **Matrix, int size, int divider);
void Blur(const Image *orig, Image *blur, int kernel_size){
    int size;
    int divider;
    if(kernel_size % 2 == 0){
        size = kernel_size/2;
        divider = (kernel_size+1)*(kernel_size+1);
    }
    else{
        size = (kernel_size-1)/2;
        divider = kernel_size*kernel_size;
    }
    ON_ERROR_EXIT(!(orig->allocation_ != NO_ALLOCATION && orig->channels >= 3), "The input image must have at least 3 channels.");
    Image_create(blur, orig->width-(2*size), orig->height-(2*size), orig->channels, false);
    ON_ERROR_EXIT(blur->data == NULL, "Error in creating the image");

    Pixels** Pixel_matrix = (Pixels**)malloc(sizeof(Pixels*)*orig->height);
    for(int i = 0; i<orig->height; i++){
        Pixel_matrix[i] = (Pixels*)malloc(sizeof(Pixels)*orig->width);
    }

    unsigned char *p = orig->data;
    for(int i = 0; i< orig->height; i++){
        for(int j = 0; j < orig->width; j++){
            Pixel_matrix[i][j].red =(uint8_t) *p;
            Pixel_matrix[i][j].green =(uint8_t) *(p+1);
            Pixel_matrix[i][j].blue = (uint8_t)*(p+2);

            if(orig->channels == 4){
                Pixel_matrix[i][j].Alpha = (uint8_t)*(p+3);
            }
            p+= orig->channels;
        }
    }

    simple_blur(orig, blur, Pixel_matrix, size, divider);
     for(int i = 0; i< orig->height; i++){
        free(Pixel_matrix[i]);
     }
     free(Pixel_matrix);
}

void simple_blur(const Image *orig, Image *blur, Pixels **Matrix, int size, int divider){
    double sumR, sumG, sumB;
    unsigned char *p = blur->data;
    for(int i = size; i < orig->height-size; i++){
        for(int j = size; j < orig->width-size; j++){
            sumR = 0;
            sumG = 0;
            sumB = 0;
            for(int x = (-1)*size; x <=size; x++){
                for(int y =(-1)*size; y <=size; y++){
                    sumR += Matrix[i+x][j+y].red;
                    sumG += Matrix[i+x][j+y].green;
                    sumB += Matrix[i+x][j+y].blue;
                }
            }
            sumR = sumR/divider;
            sumG = sumG/divider;
            sumB = sumB/divider;
            *p = (uint8_t) sumR;
            *(p+1) = (uint8_t) sumG;
            *(p+2) = (uint8_t) sumB;
            p += blur->channels;
        }
    }
}

void sobel_kernel(double **matrix, Image *G,const int kernel[][3], int x, int y){
    Image_create(G, y-2, x-2, 1, false);
    ON_ERROR_EXIT(G->data == NULL, "Error in creating the image");

    int sum;
    unsigned char *pg = G->data;
    for(int i = 1; i< x-1; i++){
        for(int j = 1; j < y-1; j++){
            sum = 0;
            sum += kernel[0][0]*matrix[i-1][j-1]+kernel[0][1]*matrix[i-1][j]+kernel[0][2]*matrix[i-1][j+1];
            sum += kernel[1][0]*matrix[i][j-1]  +kernel[1][1]*matrix[i][j]  +kernel[1][2]*matrix[i][j+1];
            sum += kernel[2][0]*matrix[i+1][j-1]+kernel[2][1]*matrix[i+1][j]+kernel[2][2]*matrix[i+1][j+1];
            *pg = (uint8_t) MinMax(sum);
            pg++;
        }
    }
}

void sobel_blur(double **blur_matrix, uint8_t **matrix, int x, int y){
    double sum = 0;
    //printf("0,0px: %d", matrix[1][1]);
    //printf("x:%d y:%d", x, y);
    for(int i = 1; i< x-1; i++){
        for(int j = 1; j < y-1; j++){
            sum = 0;
            sum +=   matrix[i-1][j-1]+2*matrix[i-1 ][j]+ matrix[i-1][j+1];
            sum += 2*matrix[i][j-1]+  4*matrix[i][j]+  2*matrix[i][j+1];
            sum +=   matrix[i+1][j-1]+2*matrix[i+1][j]+  matrix[i+1][j+1];
            //printf("!%d! %d", sum, uint8_t) sum/16.0);
            sum = sum/16.0;
            blur_matrix[i-1][j-1] = sum;
        }
    }
}

void sobel_gradient(Image *res, Image *Gx, Image *Gy){
    Image_create(res, Gx->width, Gx->height, Gx->channels, false);
    ON_ERROR_EXIT(res->data == NULL, "Error in creating the image");
    // Sepia filter coefficients from https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created
    for(unsigned char *px = Gx->data, *py = Gy->data, *pg = res->data; px != Gx->data + Gx->size; px += Gx->channels, py += Gy->channels, pg += res->channels) {
        *pg      = (uint8_t) fmin(sqrt((*px * *px)+(*py * *py)),255);
    }
}

void line_detection(const Image *orig, Image* Gx, Image* Gy, Image * Line_det){
    Image gray;
    if(orig->channels >2)
        Image_to_gray(orig, &gray);
    else
        gray = *orig;

    uint8_t **matrix = (uint8_t**)malloc(sizeof(uint8_t*)*gray.height);
    for(int i = 0; i < gray.height; i++){
        matrix[i] = (uint8_t*) malloc(sizeof(uint8_t)*gray.width);
    }
    uint8_t *p = gray.data;
    for(int i = 0; i < gray.height; i++){
        for(int j = 0; j<gray.width; j++){
            matrix[i][j] = *p;
            p+= gray.channels;
        }
    }
    double **blur_matrix = (double**) malloc(sizeof(double*)*(gray.height-2));
    for(int i= 0; i <(gray.height-2); i++){
        blur_matrix[i] = (double*) malloc(sizeof(double)*(gray.width-2));
    }
    int x = gray.height, y = gray.width;
    sobel_blur(blur_matrix, matrix, x, y);
    int X_kernel[3][3] = {{-1, 0, 1},
                          {-2, 0, 2},
                          {-1, 0, 1}};

    int Y_kernel[3][3] = {{-1, -2, -1},
                          {0, 0, 0},
                          {1, 2, 1}};

    x -=2;
    y -=2;
    sobel_kernel(blur_matrix, Gy, Y_kernel, x, y);
    sobel_kernel(blur_matrix, Gx, X_kernel, x, y);
    sobel_gradient(Line_det, Gx, Gy);

    //felszabaditas
    for(int i = 0; i < gray.height; i++){
        free(matrix[i]);
    }
    free(matrix);
    for(int i= 0; i <(gray.height-2); i++){
        free(blur_matrix[i]);
    }
    free(blur_matrix);
}

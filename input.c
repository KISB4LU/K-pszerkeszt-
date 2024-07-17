#include "input.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#define INPUT 128

void elkereses(Image *img, Image *res){
    Image Gx, Gy;
    line_detection(img, &Gx, &Gy, res);
    Image_save(&Gx,"data\\Gx.jpg");
    Image_save(&Gy,"data\\Gy.jpg");

    Image_free(&Gx);
    Image_free(&Gy);
}
void homalyositas(Image *img, Image *res){
    bool start = true,err = true;
    char input[INPUT];
    int opt;
    while(start){
        system("cls");
        printf("1. gaussian homalyositas (gyorsabb ido viszont a homalyositas merteke gyengebb)\n");
        printf("2. valaszthato meretu homalyositas (lassaban fut le viszont a homalyositas merteke erosebb)\n");
        gets(input);
        opt = atoi(input);
        if(opt == 1){
            Image_to_blur(img, res);
            start = false;
            return;
        }
        else if(opt == 2){
            int ertek;
            double kernel_size;
            while(err){
            system("cls");
            printf("vallaszon egy erteket 0-100 kozott");
            gets(input);
            ertek = atoi(input);
            if(ertek<0 || ertek > 100){
                system("cls");
                printf("a megadott ertek nem megfelelo\n kerem adja meg ujra!");
                getchar();
                }
            else{
                kernel_size = img->width > img->height ? img->width : img->height;
                kernel_size = sqrt(kernel_size);
                kernel_size = (kernel_size/100)*ertek;
                Blur(img, res,((int)kernel_size) );
                err = false;
                return;
                }

            }
        }
        else{
            system("cls");
            printf("nem letezik ilyen opcio\n kerem adja meg ujra");
        }
    }
}

void vilagositas(Image *img, Image *res){
    bool err = true;
    char input[10];
    int ertek;
    while(err){
        system("cls");
        printf("adjon meg egy erteket 0-100 kozott: ");
        gets(input);
        ertek = atoi(input);
        if(ertek<0 || ertek > 100){
            printf("a megadott szam kivul esik a megadott tartomanyon\nkerem adjon meg egy masik szamot!");
            getchar();
        }
        else
            err = false;
    }
    double gamma = ((double)ertek/100)+1;
    Gamma(img, res, gamma);
}

void sotetites(Image *img, Image *res){
    bool err = true;
    char input[10];
    int ertek;
    while(err){
        system("cls");
        printf("adjon meg egy erteket 0-100 kozott: ");
        gets(input);
        ertek = atoi(input);
        if(ertek<0 || ertek > 100){
            printf("a megadott szam kivul esik a megadott tartomanyon\nkerem adjon meg egy masik szamot!");
            getchar();
        }
        else
            err = false;
    }
    double gamma = (double)ertek/100;
    Gamma(img, res, gamma);
}

void kep_mentes(Image *res, char *file_name){
    bool err = true;
    char *formatum = strrchr(file_name, '.');

    while(err){
        system("cls");
        if(strcmp(formatum, ".jpg") == 0 || strcmp(formatum, ".png") == 0){
            Image_save(res, file_name);
            err = false;
            return;
        }
        else{
            printf("a megadott kepformatun nem egyezik!\nkerem adjon meg egy masik nevet: ");
            gets(file_name);
        }
    }
}









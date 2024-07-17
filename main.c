#include "Image.h"
#include "utils.h"
#include "debugmalloc.h"
#include "input.h"
#define INPUT 128

int main(void) {

    bool start = true, err = true, next = false;
    char file_name[INPUT+1] = " ";
    char input[INPUT+1] = " ";
    int opt;
    while(start){
        Image img, res;
        while(err){
        printf("kerem adja meg a file nevet:");
        gets(file_name);
        system("cls");
        Image_load(&img, file_name);
        if(img.data == NULL){
            printf("hiba a kep betoltesenel\nkerem probalja meg ujra!");
            getchar();
            system("cls");
            }
        else
            err = false;
        }
        printf("valasza ki az opciot:\n");
        printf("1. elkereses\n");
        printf("2. homalyositas\n");
        printf("3. elesites\n");
        printf("4. vilagositas\n");
        printf("5. sotetites\n");
        gets(input);
        opt = atoi(input);
        system("cls");

        switch(opt){
        case 1:
            elkereses(&img, &res);
            if(res.data == NULL){
                printf("Error!");
                return 1;
            }
            next = true;
            break;
        case 2:
            homalyositas(&img, &res);
            if(res.data == NULL){
                printf("Error!");
                return 1;
            }
            next = true;
            break;
        case 3:
            Image_to_sharp(&img, &res);
            if(res.data == NULL){
                printf("Error!");
                return 1;
            }
            next = true;
            break;
        case 4:
            vilagositas(&img, &res);
            if(res.data == NULL){
                printf("Error!");
                return 1;
            }
            next = true;
            break;
        case 5:
            sotetites(&img, &res);
            if(res.data == NULL){
                printf("Error!");
                return 1;
            }
            next = true;
            break;
        default:
            printf("a valasztott opcio nem letezik\nkerem adjon meg egy masik opciot!");
            getchar();
            system("cls");
            continue;
        }
        if(next){
            printf("milyen neven szeretne menteni a kepet: ");
            gets(input);
            kep_mentes(&res, input);
            Image_free(&img);
            Image_free(&res);
            printf("szeretne tovabb folytatni?\n");
            printf("1. igen\n");
            printf("2. nem\n");
            gets(input);
            opt = atoi(input);

            if(opt == 1){
                err = true;
                next = false;
                }
            else
                start = false;

        }
    }


    return 0;
}

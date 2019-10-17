
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "bmp.h"


int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        printf("Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    char *n = argv[1];
    char *infile = argv[2];
    char *outfile = argv[3];

    int m;

    //checks if the resize factor is a digit
    if(isdigit(*n)){

        //turns the number into an int
        m = atoi(n);

        if (m < 1 || m > 100){
        printf("n, the resize factor, must be a number between 1 and 100\n");
        return 1;
        }
    }
    else{
        printf("n, the resize factor, must be a number between 1 and 100\n");
        return 1;
    }





    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        printf("Could not create %s.\n", outfile);
        return 3;
    }

    // declaring info and file headers
    BITMAPFILEHEADER in_bf;
    BITMAPINFOHEADER in_bi;
    BITMAPFILEHEADER out_bf;
    BITMAPINFOHEADER out_bi;

    fread(&in_bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    fread(&in_bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (in_bf.bfType != 0x4d42 || in_bf.bfOffBits != 54 || in_bi.biSize != 40 ||
        in_bi.biBitCount != 24 || in_bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        printf("Unsupported file format.\n");
        return 4;
    }


    //copying infile headers into outfile headers before resizing
    out_bf = in_bf;
    out_bi = in_bi;

    out_bi.biWidth *= m;
    out_bi.biHeight *= m;

    int in_padding = (4 - (in_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int out_padding = (4 - (out_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    out_bi.biSizeImage = ((sizeof(RGBTRIPLE) * out_bi.biWidth) + out_padding) * abs(out_bi.biHeight);

    out_bf.bfSize = out_bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&out_bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&out_bi, sizeof(BITMAPINFOHEADER), 1, outptr);


    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(in_bi.biHeight); i < biHeight; i++)
    {
        // vertical resize
        for (int j = 0; j < m; j++){

            // iterate over pixels in scanline
            for (int k = 0; k < in_bi.biWidth; k++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // horizontal resize
                for (int h = 0; h < m; h++){

                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }

            // skip over padding, if any
            fseek(inptr, in_padding, SEEK_CUR);

            // add padding in the outfile
            for (int l = 0; l < out_padding; l++)
            {
                fputc(0x00, outptr);
            }
            //moving the pointer to the beginning of the scanline cause we need to rewrite it
            fseek(inptr, -((in_bi.biWidth * sizeof(RGBTRIPLE)) + in_padding), SEEK_CUR);
        }
        //moving the pointer to the end of the last scanline so we can start the new one
        fseek(inptr, (in_bi.biWidth * sizeof(RGBTRIPLE)) + in_padding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}

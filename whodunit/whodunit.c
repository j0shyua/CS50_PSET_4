/**
 * Removes red from an inputted BMP file, and
 * saves it in the specified ouput.
 */

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main (int argc, char *argv[])
{
    // Require 3 command-line-arguments
    if (argc != 3)
    {
        fprintf(stderr, "Usage: ./whodunit inputFile outputFile\n");
        return 1;
    }
    
    // Remember names of input/output files
    char *inputFile = argv[1];
    char *outputFile = argv[2];
    
    // Open input file
    FILE *inPointer = fopen(inputFile, "r");
    if (inPointer == NULL)
    {
        fprintf(stderr, "Error: Could'nt open %s.\n", inputFile);
        return 2;
    }

    // Open output file
    FILE *outPointer = fopen(outputFile, "w");
    if (outPointer == NULL)
    {
        fclose(inPointer);
        fprintf(stderr, "Error: Could'nt create %s.\n", outputFile);
        return 3;
    }
    
    // Read inputFile's BITMAPFILEHEADER 
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inPointer);

    // Read inputFile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inPointer);
    
    // Make sure input file is a 24-bit uncompressed BMP version 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outPointer);
        fclose(inPointer);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    // Write the outputFile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outPointer);

    // Write outputFile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outPointer);

    // Determine padding for scanlines (each line of BMP)
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Iterate over each of inputFile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // Iterate over pixels in each scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // Temporary storage (triple stores rgbtBlue/Green/Red)
            RGBTRIPLE triple;

            // Read RGB triple from inputFile
            fread(&triple, sizeof(RGBTRIPLE), 1, inPointer);

            // If a pixel is pure red, change it to white and then write it to the outputFile
            if (triple.rgbtBlue == 0x00 && triple.rgbtGreen == 0x00 && triple.rgbtRed == 0xff)
            {
                triple.rgbtBlue = 0xff;
                triple.rgbtGreen = 0xff;
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outPointer);
            }

            // If the pixel isn't pure red, copy it over to the outputFile
            else
            {
                fwrite(&triple, sizeof(RGBTRIPLE), 1, outPointer);
            }
            
        }

        // Skip over input's padding, if there is any
        fseek(inPointer, padding, SEEK_CUR);

        // Then add padding back into outPointer
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outPointer);
        }
    }

    // Close inputFile
    fclose(inPointer);

    // Close outputFile
    fclose(outPointer);

    // Ended successfully
    printf("Converted image is now in %s.\n", outputFile);
    return 0;
    
}
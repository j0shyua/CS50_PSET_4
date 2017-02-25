/**
 * Resize a BMP that the user provides by a factor
 * of n, and write the image to a new output.
 */
 
#include <stdio.h>
#include <stdlib.h>
 
#include "bmp.h"
 
int main (int argc, char *argv[])
{
    // Require 4 command line arguments.
    if (argc != 4 )
    {
        fprintf (stderr, "Usage: ./resize n inputFile outputFile.\n");
        printf ("n is the factor by which to resize the BMP, and n <= 100\n");
        return 1;
    }
     
    // Remember the names of the input/output and resize factor
    char *inputFile = argv[2];
    char *outputFile = argv[3];
    int n = atoi (argv[1]);
     
    // Open input file
    FILE *inPointer = fopen(inputFile, "r");
    if (inPointer == NULL)
    {
        fclose(inPointer);
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
    
    // Create outputFile's BITMAPFILEHEADER
    BITMAPFILEHEADER bfNew = bf;
    
    // Create outputFile's BITMAPINFOHEADER
    BITMAPINFOHEADER biNew = bi;
    
    // Multiply outputFile's width and height by n.
    biNew.biWidth *= n;
    biNew.biHeight *= n;
    
    // Calculate padding for the outputFile, and store in header
    int paddingNew =  (4 - (biNew.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    // Update information (image size and file size) for outputFile's headers
    biNew.biSizeImage = (((sizeof(RGBTRIPLE) * biNew.biWidth) + paddingNew) * abs(biNew.biHeight));
    bfNew.bfSize = biNew.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // Write the outputFile's BITMAPFILEHEADER
    fwrite(&bfNew, sizeof(BITMAPFILEHEADER), 1, outPointer);

    // Write outputFile's BITMAPINFOHEADER
    fwrite(&biNew, sizeof(BITMAPINFOHEADER), 1, outPointer);

    // Determine padding for scanlines (each line of BMP)
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Iterate over each of inputFile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // Stretch the picture vertically by factor of n
        for (int j = 0; j < n; j++)
        {
            // Iterate over pixels in each scanline
            for (int k = 0; k < bi.biWidth; k++)
            {
                // Temporary storage (triple stores rgbtBlue/Green/Red)
                RGBTRIPLE triple;

                // Read RGB triple from inputFile
                fread(&triple, sizeof(RGBTRIPLE), 1, inPointer);

                // Repeat writing horizontally to stretch by factor of n.
                for (int a = 0; a < n; a++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outPointer);
                }
                
            }
            
            // Then add padding back into outPointer
            for (int k = 0; k < paddingNew; k++)
            {
                fputc(0x00, outPointer);
            }
            
            // Get rid of padding if reading the last line of pixels n times.
            if (j != (n-1))
            {
                long int offset = (sizeof(RGBTRIPLE) * bi.biWidth);
                fseek(inPointer, -offset, SEEK_CUR);
            }
        }
        
        // Skip over input's padding, if there is any
        fseek(inPointer, padding, SEEK_CUR);
        
    } 
 
    // Close inputFile
    fclose(inPointer);

    // Close outputFile
    fclose(outPointer);

    // Ended successfully
    // printf("Converted image is now in %s.\n", outputFile);
    return 0;
}
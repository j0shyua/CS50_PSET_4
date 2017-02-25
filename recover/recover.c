/**
 * Program that scans through a forensic image and 
 * recovers lost JPEGs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define BLOCKSIZE 512

int main (int argc, char *argv[])
{
    // Ensure correct usage
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./recover fileName\n");
        return 1;
    }
    
    // Store inputted file name in inputFile
    char *inputFile = argv[1];
    
    // Check if inputFile can be opened
    FILE *inPointer = fopen(inputFile, "r");
    if (inPointer == NULL)
    {
        fclose(inPointer);
        fprintf(stderr, "Error: couldn't open %s.\n", inputFile);
        return 2;
    }
    
    uint8_t *jpgBuffer = (uint8_t *)malloc (BLOCKSIZE);
    bool jpegFound = false;
    
    while (jpegFound == false)
    {
        fread(jpgBuffer, 1, BLOCKSIZE, inPointer);
        
        if (jpgBuffer[0] == 0xff && jpgBuffer[1] == 0xd8 && jpgBuffer[2] == 0xff && (jpgBuffer[3] & 0xf0) == 0xe0)
        {
            jpegFound = true;
        }
    }
    
    for (int i = 0; ; i++)
    {
        // Create a new file for the JPG to be stored in (###.jpg)
        char fileName[8];
        sprintf(fileName, "%03i.jpg", i);
        FILE *savedImg = fopen(fileName, "w");
        
        bool newImageFound = false;
        
        do
        {
            fwrite(jpgBuffer, 1, BLOCKSIZE, savedImg);
            
            // If last block is less than 512 bytes, stop
            if (fread(jpgBuffer, 1, BLOCKSIZE, inPointer) < BLOCKSIZE)
            {
                fclose(savedImg);
                fclose(inPointer);
                free (jpgBuffer);
                return 0;
            }
            
            if (jpgBuffer[0] == 0xff && jpgBuffer[1] == 0xd8 && jpgBuffer[2] == 0xff && (jpgBuffer[3] & 0xf0) == 0xe0)
            {
                newImageFound = true;
                fclose(savedImg);
            }
            
        } while (newImageFound == false);
        
    }

    fclose(inPointer);
    free (jpgBuffer);
    
    // Success
    return 0;
}
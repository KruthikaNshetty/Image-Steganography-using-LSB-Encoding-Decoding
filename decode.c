#include <stdio.h>          
#include "decode.h"        
#include "types.h"          
#include "common.h"        
#include <string.h>   

// Function to read and validate command line arguments for decoding
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2],".bmp")==NULL)   // Check if input file is a .bmp
        return e_failure;              // Return failure if not

    decInfo->stego_image_fname = argv[2]; // Store stego image filename

    if(argv[3]==NULL)                  // If output filename not provided
    {
        printf("INFO: Output File not mentioned. Creating default file name as output\n");
        decInfo->secret_fname = "output";  // Use default name "output"
    }
    else
    {
        if(strchr(argv[3],'.') != NULL)   // If output name has extension
         decInfo->secret_fname = strtok(argv[3], "."); // Remove extension
         else
        decInfo->secret_fname = argv[3];   // Store provided name
    }

    return e_success;  // Arguments are valid
}

// Function to open the stego image file
Status open_file(DecodeInfo *decInfo)
{
    printf("INFO: Opening required files\n");
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r"); // Open for reading

    if(decInfo->fptr_stego_image == NULL)   // Check if file opened successfully
    {
        perror("fopen");                     // Print system error
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;                    // Return failure
    }

    printf("INFO: Opened %s\n", decInfo->stego_image_fname);
    return e_success;
}

// Function to decode and verify the magic string
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char imageBuffer[8];                    // Buffer to read 8 bytes at a time
    printf("INFO: Decoding Magic String Signature\n");

    for(int i=0; i<2; i++)                  // Magic string is 2 bytes
    {
        char data = 0;                      // Temporary byte to store decoded value
        fread(imageBuffer, sizeof(char), 8, decInfo->fptr_stego_image); // Read 8 bytes from image
        decode_byte_to_lsb(&data, imageBuffer); // Decode LSBs into a single byte
        if(data != magic_string[i])         // Check against expected magic string
            return e_failure;              // Return failure if mismatch
    }

    return e_success;
}

// Function to decode secret file extension size (stored in 32 bits)
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char imageBuffer[32];                    // Buffer to read 32 bytes
    printf("INFO: Decoding %s File Extension size\n", decInfo->secret_fname);

    fread(imageBuffer, sizeof(char), 32, decInfo->fptr_stego_image); // Read 32 bytes
    decInfo->size_extn_secret_file = 0;      // Initialize extension size
    decode_size_to_lsb(imageBuffer, &decInfo->size_extn_secret_file); // Decode size

    printf("INFO: Done\n");
    return e_success;
}

// Function to decode the secret file extension
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char imageBuffer[8];                     // Buffer for 1 byte
    printf("INFO: Decoding %s File Extension\n", decInfo->secret_fname);

    for(int i=0; i<decInfo->size_extn_secret_file; i++) // Loop over extension length
    {
        char data = 0;                       // Temporary byte
        fread(imageBuffer, sizeof(char), 8, decInfo->fptr_stego_image); // Read 8 bytes
        decode_byte_to_lsb(&data, imageBuffer); // Decode LSBs to get actual character
        decInfo->extn_secret_file[i] = data;   // Store character in extension
    }

    printf("INFO: Done\n");
    return e_success;
}

// Function to decode secret file size (stored in 32 bits)
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char imageBuffer[32];                     // Buffer to read 32 bytes
    printf("INFO: Decoding %s File size\n", decInfo->secret_fname);

    fread(imageBuffer, sizeof(char), 32, decInfo->fptr_stego_image); // Read 32 bytes
    decInfo->size_secret_file = 0;           // Initialize size
    decode_size_to_lsb(imageBuffer, &decInfo->size_secret_file); // Decode size

    printf("INFO: Done\n");
    return e_success;
}

// Function to decode the actual secret file data
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char imageBuffer[8];                      // Buffer for 1 byte
    printf("INFO: Decoding %s File Data\n", decInfo->secret_fname);

    for(int i=0; i<decInfo->size_secret_file; i++) // Loop for each byte of secret file
    {
        char data = 0;                        // Temporary byte
        fread(imageBuffer, sizeof(char), 8, decInfo->fptr_stego_image); // Read 8 bytes
        decode_byte_to_lsb(&data, imageBuffer); // Decode byte from LSBs
        fprintf(decInfo->fptr_secret, "%c", data); // Write decoded byte to file
    }

    if(ftell(decInfo->fptr_secret) == decInfo->size_secret_file) // Check if all bytes written
    {
        printf("INFO: Done\n");
        return e_success;
    }
    else
        return e_failure;
}

// Function to decode a single byte from 8 LSBs
Status decode_byte_to_lsb(char *data, char *image_buffer)
{
    *data = 0;                                // Initialize byte
    for(int i=0; i<8; i++)                    // Loop over 8 bits
    {
        *data = ((image_buffer[i] & 1) << i) | *data; // Extract LSB and set bit
    }
    return e_success;
}

// Function to decode a 32-bit integer from 32 LSBs
Status decode_size_to_lsb(char *imageBuffer, int* data)
{
    *data = 0;                                // Initialize integer
    for(int i=0; i<32; i++)                   // Loop over 32 bits
    {
        *data = *data | ((imageBuffer[i] & 1) << i); // Extract LSB and set bit
    }
    return e_success;
}

// Main decoding function
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_file(decInfo) == e_failure)       // Open stego file
        return e_failure;

    printf("INFO: ## Decoding Procedure Started ##\n");

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // Skip BMP header

    if(decode_magic_string(MAGIC_STRING, decInfo) == e_failure) // Verify magic string
        return e_failure;

    if(decode_secret_file_extn_size(decInfo) == e_failure) // Decode extension size
        return e_failure;

    if(decode_secret_file_extn(decInfo) == e_failure) // Decode extension
        return e_failure;

    sprintf(decInfo->temp, "%s%s", decInfo->secret_fname, decInfo->extn_secret_file); // Combine name+ext
    decInfo->secret_fname = decInfo->temp;         // Update secret filename

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w"); // Open output file for writing
    if(decInfo->fptr_secret == NULL)
        printf("ERROR: Unable to open %s file\n", decInfo->secret_fname);

    if(decode_secret_file_size(decInfo) == e_failure) // Decode secret file size
        return e_failure;

    if(decode_secret_file_data(decInfo) == e_failure) // Decode secret file data
        return e_failure;

    fclose(decInfo->fptr_secret);                  // Close output file
    fclose(decInfo->fptr_stego_image);             // Close stego image

    printf("INFO: ## Decoding Done Successfully ##\n");
    return e_success;
}

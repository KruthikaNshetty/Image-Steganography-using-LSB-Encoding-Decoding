/*
Name: Kruthika N Shetty

Project Name: Steganography

Project Description:
This project implements a Steganography System in C that allows hiding secret files .txt, .c, .h, .sh)
inside BMP images using Least Significant Bit (LSB) manipulation. The system ensures that the hidden 
data does not visibly alter the image and can be securely retrieved later.

Encoding:
The encoding part hides the secret file within a source BMP image. It stores a magic string, the secret file’s
extension, its size, and the actual content into the least significant bits of the image bytes, producing a stego 
image that looks identical to the original.

Decoding:
The decoding module extracts the hidden data from the stego image. It retrieves the magic string to verify the image,
reads the secret file extension and size, and then reconstructs the original secret file from the LSBs, saving it to the 
output file.
*/

#include <stdio.h>        
#include "encode.h"       
#include "decode.h"        
#include "types.h"         
#include <string.h>        

// Function prototype to check whether operation is encode or decode
OperationType check_operation_type(char *);

// Main function
int main(int argc, char *argv[])
{
    EncodeInfo encInfo;    // Structure variable to hold encoding information
    DecodeInfo decInfo;    // Structure variable to hold decoding information
    if(argc <=2)
    {
        printf("ERROR: Invalid arguments\n");
        printf("Usage-For encoding : ./a.out -e [src.bmp] [secret.txt] <dest.bmp>\n");
        printf("For decoding : ./a.out -d [dest.bmp]  <output>\n");
        return 0;           // Exit program
    }
    // Check if number of arguments is less than required for encoding
    if( (strcmp(argv[1], "-e") == 0) && argc < 4 )
    {
        printf("ERROR: Invalid arguments\n");
        printf("Usage-For encoding : ./a.out -e [src.bmp] [secret.txt] <dest.bmp>\n");
        printf("For decoding : ./a.out -d [dest.bmp]  <output>\n");
        return 0;           // Exit program
    }
    // Check if number of arguments is less than required for decoding
    

    // Check if operation type is supported (-e or -d)
    if (check_operation_type(argv[1]) == e_unsupported)
    {
        printf("ERROR: Unsupported operation\n");
        return 0;           // Exit program
    }
    // If operation is encoding
    else if(check_operation_type(argv[1]) == e_encode)
    {
        // Read and validate encoding arguments
        if(read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("ERROR: Invalid extensions\n");
            return 0;       // Exit program if arguments are invalid
        }
        do_encoding(&encInfo);  // Call function to perform encoding
    }
    // If operation is decoding
    else
    {
        // Read and validate decoding arguments
        if(read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
           printf("ERROR: Invalid extensions\n");
           return 0;       // Exit program if arguments are invalid
        }
        do_decoding(&decInfo);  // Call function to perform decoding
    }
}

// Function to check operation type based on command line argument
OperationType check_operation_type(char *symbol)
{
    if(strcmp(symbol,"-e") == 0)       // If symbol is -e
        return e_encode;               // Return encode type
    else if(strcmp(symbol,"-d") == 0)  // If symbol is -d
        return e_decode;               // Return decode type
    else
        return e_unsupported;          // Unsupported operation
}

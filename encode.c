#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include<string.h>
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET); 

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    
    // Find the size of secret file data
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */


 /* Read and validate input arguments for encoding */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(strstr(argv[2],".bmp")==NULL) // Check if source image is BMP
    return e_failure;
    encInfo->src_image_fname=argv[2]; // Store source image name

    if(strstr(argv[3],".txt")==NULL && strstr(argv[3],".c")==NULL && strstr(argv[3],".h")==NULL && strstr(argv[3],".sh")==NULL) // Check if secret file has valid extension (.txt, .c, .h, .sh)
    return e_failure;
    encInfo->secret_fname=argv[3]; // Store secret file name

    if(argv[4]==NULL)  // If output file not provided, assign default dest.bmp
    {

        
        printf("INFO: Output File not mentioned.Creating dest.bmp as defualt\n");
        encInfo->stego_image_fname="dest.bmp";
    }
    else
    {
        if(strstr(argv[4],".bmp")==NULL) // Ensure output file is BMP
        return e_failure;
        encInfo->stego_image_fname=argv[4];  // Store stego image name
    }
    return e_success;  // Return success

}

Status open_files(EncodeInfo *encInfo) /* Open all required files */
{
    // Src Image file
    printf("INFO: Opening required files\n");
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");  // Open source image
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->src_image_fname);
    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL) // Check for errors
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->secret_fname);
    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");// Open stego image
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->stego_image_fname);
    printf("INFO: Done\n");
    // No failure return e_success
    
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
     
     printf("INFO: checking for %s size\n",encInfo->secret_fname);
     encInfo->size_secret_file =get_file_size(encInfo->fptr_secret); // Get secret file size
     printf("INFO: Done.Not Empty\n");
     printf("INFO: checking for %s Capacity to handle secret.txt\n",encInfo->src_image_fname);
     encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); // Get image capacity
     if(encInfo->image_capacity > (16+32+32+32+((encInfo->size_secret_file)*8))) // Check if image has enough capacity for all data + metadata
     {
        printf("INFO: Done.Found OK\n");
        return e_success;
     }
     else
     return e_failure; // Not enough capacity
}

/* Copy BMP header from source to destination */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char imageBuffer[54];  // BMP header is 54 bytes
    rewind(fptr_src_image);  // Go to beginning of source image
    fread(imageBuffer,sizeof(char),54,fptr_src_image); // Read header
    printf("INFO: Copying Image Header\n");
    fwrite(imageBuffer,sizeof(char),54,fptr_dest_image); // Write header to stego image
    if(ftell(fptr_src_image)==ftell(fptr_dest_image))   // Verify copy
    {
        printf("INFO: Done\n");
        return e_success;
    }
    else
    return e_failure;
}

/* Encode Magic String into image */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char imageBuffer[8];   // Buffer to hold 8 bytes
    printf("INFO: Encoding Magic String Signature\n");
    for(int i=0;i<2;i++) // Loop for 2 characters
    {
        fread(imageBuffer,sizeof(char),8,encInfo->fptr_src_image); // Read 8 bytes
        encode_byte_to_lsb(magic_string[i],imageBuffer);          // Encode character
        fwrite(imageBuffer,sizeof(char),8,encInfo->fptr_stego_image); // Write to stego
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    {
        printf("INFO: Done\n");
        return e_success;
    }
    else
    return e_failure;
}

/* Encode secret file extension size */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char  imageBuffer[32]; // 32-bit buffer
    printf("INFO: Encoding %s File Extension size\n",encInfo->secret_fname);
    fread(imageBuffer,sizeof(char),32,encInfo->fptr_src_image); // Read 32 bytes
    encode_size_to_lsb(size,imageBuffer);  // Encode size
    fwrite(imageBuffer,sizeof(char),32,encInfo->fptr_stego_image);  // Write to stego
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    {
        printf("INFO: Done\n");
        return  e_success;
    }
    else
    return e_failure;


}

/* Encode secret file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
     char imageBuffer[8];         // 8-byte buffer
    int len = strlen(file_extn);  // Get extension length
    printf("INFO: Encoding %s File Extension\n",encInfo->secret_fname);
    for(int i=0;i<len;i++)    // Loop for each character
    {
        fread(imageBuffer,sizeof(char),8,encInfo->fptr_src_image); // Read 8 bytes
        encode_byte_to_lsb(file_extn[i],imageBuffer);              // Encode character
        fwrite(imageBuffer,sizeof(char),8,encInfo->fptr_stego_image); // Write to stego
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    {
        printf("INFO: Done\n");
        return  e_success;
    }
    else
    return e_failure;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char  imageBuffer[32]; // 32-byte buffer
    printf("INFO: Encoding %s File size\n",encInfo->secret_fname);
    fread(imageBuffer,sizeof(char),32,encInfo->fptr_src_image);  // Read 32 bytes
    encode_size_to_lsb(file_size,imageBuffer);  // Encode size
    fwrite(imageBuffer,sizeof(char),32,encInfo->fptr_stego_image); // Write to stego
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    {
        printf("INFO: Done\n");
    return  e_success;
    }
    else
    return e_failure;
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);   // Rewind secret file
    fread(encInfo->secret_data,sizeof(char),encInfo->size_secret_file,encInfo->fptr_secret);
    char  imageBuffer[8];
    printf("INFO: Encoding %s File Data\n",encInfo->secret_fname); // Loop over secret data
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(imageBuffer,sizeof(char),8,encInfo->fptr_src_image); // Read 8 bytes
        encode_byte_to_lsb(encInfo->secret_data[i],imageBuffer);
        fwrite(imageBuffer,sizeof(char),8,encInfo->fptr_stego_image);  // Write to stego
    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_stego_image))
    {
        printf("INFO: Done\n");
    return  e_success;
    }
    else
    return e_failure;

}


/* Copy remaining BMP image data */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    printf("INFO: Copying Left Over Data\n");
    while (fread(&ch, sizeof(char), 1, fptr_src) == 1) // Read byte by byte
    {
        fwrite(&ch, sizeof(char), 1, fptr_dest);
    }
    printf("INFO: Done\n");
    return e_success;

}

/* Encode single byte into LSBs of 8 bytes */
 Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> i) & 1); // Replace LSB
    }
    return e_success;
}

 Status encode_size_to_lsb(int size, char *image_buffer) /* Encode 32-bit size into LSBs */
{
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size >> i) & 1); // Replace LSB
    }
    return e_success;
}


/* Full encoding procedure */
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo)==e_failure) // Open files
    return e_failure;
    printf("INFO: ## Encoding Procedure Started ##\n");
    if(check_capacity(encInfo)==e_failure)// Check image capacity
    return e_failure;
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure)
    {
        printf("ERROR: Bmp header coping failed\n");
        return e_failure;
    }
    if(encode_magic_string(MAGIC_STRING,encInfo)==e_failure)
    {
        printf("ERROR: Magic string encoding failed\n");
        return e_failure;
    }
    char *a=strchr(encInfo->secret_fname,'.');  // Get extension
    strcpy(encInfo->extn_secret_file,a);       // Store extension
    int len=strlen(encInfo->extn_secret_file);
    if(encode_secret_file_extn_size(len,encInfo)==e_failure)
    {
        printf("ERROR: Encoding secret file extension size failed\n");
        return  e_failure;
    }
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_failure)
    {
        printf("ERROR: Encoding secret file extension failed\n");
        return e_failure;
    }

    if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_failure)
    {
        printf("ERROR: Encoding secret file size failed\n");
        return e_failure;
    }
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: Encoding secret file data failed\n");
        return e_failure;
    }
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Encoding remaining image data failed\n");
        return e_failure;
    }
    fclose(encInfo->fptr_src_image);  // Close source image
    fclose(encInfo->fptr_secret); // Close secret file
    fclose(encInfo->fptr_stego_image);  // Close stego image
    printf("INFO: ##  Encoding Done Successfully ##\n");

    return e_success;    // Return success
}
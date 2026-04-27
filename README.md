## Image-Steganography-using-LSB-Encoding-Decoding
## Description
This project implements image steganography by hiding secret text data inside a BMP image using Least Significant Bit (LSB) encoding. The system also supports decoding to retrieve the hidden message without noticeable image distortion.

## 🚀 Features
- Encode secret message into image
- Decode hidden message from image
- Preserves image quality
- Works with BMP format
  
## 🛠️ Technologies Used
- C Programming
- Bitwise Operations
- File Handling
- Pointers
- Command Line Arguments

## ⚙️ How It Works
- Reads BMP image header and pixel data
- Replaces LSB of pixel bytes with message bits
- Stores encoded image
- Decoding extracts bits and reconstructs message

  ## ⚠️ Challenges Faced
- Practical use of bitwise operations
- Working with binary files
- Data hiding techniques
- Debugging low-level file operations

  ▶️ How to Run
  1 gcc *.c
  2 ./stego -e input.bmp output.bmp "secret message"
  3 ./stego -d output.bmp






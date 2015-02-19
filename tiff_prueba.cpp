
//gcc tiff_prueba.cpp -o tiff_prueba -L/usr/local/lib -ltiff
#include <tiffio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
using namespace std;


const char save_name[] = "/Users/ajaver/Desktop/DCR_D_1237.tif";
const char file_name[] = "/Users/ajaver/Desktop/John_Format/DCR_D_1237.PVSeq";

const int HEIGHT = 2048;
const int WIDTH = 2048;
const int TOT_PIX  = HEIGHT*WIDTH;
const char INI_KEY[] = "PV01"; // key that tells a new image block starts
const int INI_KEY_SIZE = strlen(INI_KEY);
const int HEADER_SIZE = 2048;

int ini_key_match(char * buffer, int position, int buff_size){
    //check if the position matches the new block key
    
    //exit if the position is larger that the buffer size
    if (position + INI_KEY_SIZE > buff_size)
        return 0;

    
    for (int j = 0; j<INI_KEY_SIZE; j++){
        //exit if any character does not match the key
        if (buffer[position+j] != INI_KEY[j])
            return 0;
    }
    //only return true if all the next characters match the key
    return 1;
}


int main()
{
    ifstream file;
    streampos file_size;
    char * file_data;
    uint8_t image_buffer[TOT_PIX] = {0};
    uint16_t num_zeros;
    
    file.open(file_name, ios::in | ios::binary | ios::ate);
    // ios::ate position file at the end of the file
    if (file.is_open()){
        //read all the file into memory (faster than reading one character at a time, but could give problems if the image is too big
        file_size = file.tellg();
        file_data = new char[file_size];
        
        file.seekg(0, ios::beg);
        file.read(file_data, file_size);
        file.close();
        
        TIFF* out_tiff = TIFFOpen(save_name, "w");
        
        
        int file_position = 0;
        int image_position = 0;
        int image_number = 0;
        while (file_position < file_size){
            
            TIFFSetField (out_tiff, TIFFTAG_IMAGEWIDTH, WIDTH);  // set the width of the image
            TIFFSetField(out_tiff, TIFFTAG_IMAGELENGTH, HEIGHT);    // set the height of the image
            TIFFSetField(out_tiff, TIFFTAG_SAMPLESPERPIXEL, 1);   // set number of channels per pixel
            TIFFSetField(out_tiff, TIFFTAG_BITSPERSAMPLE, 8);    // set the size of the channels
            TIFFSetField(out_tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
            
            
            TIFFSetField(out_tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
            TIFFSetField(out_tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW); 
            //TIFFSetField(out_tiff, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
            
            
            
            if (ini_key_match(file_data, file_position, file_size)){
                //save previous data
                if (image_number > 0){
                    TIFFWriteEncodedStrip(out_tiff, 0, image_buffer, TOT_PIX);
                    TIFFWriteDirectory(out_tiff);
                }
                
                //initialize image buffer
                image_position = 0;
                for (int m = 0; m<TOT_PIX; m++)
                    image_buffer[m] = 0;
                
                cout << file_position << ' ' << image_number << endl;
                
                image_number++;
                // move position after the header
                file_position += HEADER_SIZE;
                
                
                //if (image_number > 1) return 0;
            }
            else{
                if (file_data[file_position] != 0)
                {
                    image_buffer[image_position] = file_data[file_position];
                    file_position++;
                    image_position++;
                }
                else{
                    // read the next two bytes to determine the number of zeros
                    //must cast into unsigned char otherwise it would not work
                    num_zeros = (unsigned char)file_data[file_position+1] << 8 | (unsigned char)file_data[file_position+2];
                    
                    image_position += num_zeros;
                    
                    //cout << (file_data[file_position+1]) << ' ' << uint8_t(file_data[file_position+2]) << endl;
                    //cout << file_position << ' ' << num_zeros << endl;
                    //if (image_position > TOT_PIX)
                    //    cout << "Image buffer overflow " << TOT_PIX << ' ' << image_position<<endl;
                    
                    file_position += 3;
                }
            }
            
        }
        
        TIFFWriteEncodedStrip(out_tiff, 0, image_buffer, TOT_PIX);
        TIFFClose(out_tiff);
        delete file_data;

    }
    
    return 0;
}

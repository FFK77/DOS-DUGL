#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <png.h>

#include "DUGL.h"


// PNG ////////////////////////////////////////////////////////////////////

FILE *pngFile = NULL;
void *pngIn = NULL;
int sizePngIn = 0;
int ptrPngIn = 0;

png_byte header[8];
int png_width;
int png_height;
png_byte png_color_type;
png_byte png_bit_depth;

png_structp png_ptr;
png_infop png_info_ptr;
png_infop png_end_ptr;
int png_number_of_passes;
png_bytep * png_row_pointers;

void CloseOpenPNG() {
    if (png_row_pointers!=NULL) {
        for (int y=0; y<png_height; y++) {
            if (png_row_pointers[y]!=NULL) {
                free(png_row_pointers[y]);
                png_row_pointers[y] = NULL;
            }
        }
        free(png_row_pointers);
        png_row_pointers = NULL;
    }
    png_destroy_read_struct(&png_ptr, &png_info_ptr, &png_end_ptr);
    png_width = 0;
    png_height = 0;
    png_color_type = 0;
    png_bit_depth = 0;
    png_number_of_passes = 0;
}
#define PNG_CLOSE_FILE() \
  if (pngFile!=NULL) { \
    fclose(pngFile); pngFile = NULL; \
  }

#define PNG_MEM_READ_INIT(IN, SIZE) { \
            pngIn = (void*)(IN); \
            sizePngIn = (int)SIZE; \
            ptrPngIn = 0; \
        }

#define PNG_MEM_READ(DST, SIZEREAD, BYTES_READY) { \
            if (sizePngIn - ptrPngIn >= (int)SIZEREAD) { \
                memcpy((void*)DST, &((char*)(pngIn))[ptrPngIn], SIZEREAD); \
                BYTES_READY = SIZEREAD; \
                ptrPngIn += (int)SIZEREAD; \
            } else { \
                BYTES_READY = 0; \
            } \
        }

void ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes,
                             png_size_t byteCountToRead) {
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    if(io_ptr == NULL)
        return;   // add custom error handling here

    int bytesReady = 0;
    PNG_MEM_READ(outBytes, byteCountToRead, bytesReady);

    if(bytesReady == 0)
        return;   // add custom error handling here
}

int DecodePNG() {
    int y = 0;
    png_width = 0;
    png_height = 0;
    png_color_type = 0;
    png_bit_depth = 0;
    png_number_of_passes = 0;
    png_row_pointers = NULL;

    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, png_info_ptr);

    png_width = png_get_image_width(png_ptr, png_info_ptr);
    png_height = png_get_image_height(png_ptr, png_info_ptr);
    png_color_type = png_get_color_type(png_ptr, png_info_ptr);
    png_bit_depth = png_get_bit_depth(png_ptr, png_info_ptr);

    png_number_of_passes = png_set_interlace_handling(png_ptr);

    // convert palette to RGB or RGBA (if masked) image
    if(png_color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
        png_bytep trans_alpha = NULL;
        int num_trans = 0;
        png_color_16p trans_color = NULL;

        png_get_tRNS(png_ptr, png_info_ptr, &trans_alpha, &num_trans, &trans_color);
        png_color_type = (trans_alpha != NULL) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
    }

    // Convert 1-2-4 bits grayscale images to 8 bits  grayscale.
    if (png_color_type == PNG_COLOR_TYPE_GRAY && png_bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8 (png_ptr);
    if (png_get_valid (png_ptr, png_info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha (png_ptr);

    // force to 8 bits per color channel
    if (png_bit_depth == 16)
        png_set_strip_16 (png_ptr);
    else if (png_bit_depth < 8)
        png_set_packing (png_ptr);

    png_read_update_info(png_ptr, png_info_ptr);

    // read file
    if (setjmp(png_jmpbuf(png_ptr))) {
        CloseOpenPNG();
        return 0;
    }

    png_row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * png_height);
    if (png_row_pointers==NULL) { // non mem ?
        CloseOpenPNG();
        return 0;
    }
    for (y=0; y<png_height; y++) {
        png_row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,png_info_ptr));
        if (png_row_pointers[y]==NULL) { // non mem ?
            CloseOpenPNG();
            return 0;
        }
    }

    png_read_image(png_ptr, png_row_pointers);
    png_read_end(png_ptr, png_end_ptr);

    return 1;
}

int LoadPNGMem(void *In, int SizeIn) {
    int bytesReady = 0;

    // check buffer
    if (In==NULL || SizeIn <= 8)
        return 0;

    PNG_MEM_READ_INIT(In, SizeIn);

    PNG_MEM_READ(header, 8, bytesReady);
    if (bytesReady == 0) {
        PNG_MEM_READ_INIT(NULL, 0);
        return 0;
    }

    // Initialize png read
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr==NULL) {
        return 0;
    }
    png_info_ptr = png_create_info_struct(png_ptr);
    if (png_info_ptr==NULL) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return 0;
    }
    png_end_ptr = png_create_info_struct(png_ptr);
    if (png_info_ptr==NULL) {
        png_destroy_read_struct(&png_ptr, &png_info_ptr, (png_infopp)NULL);
        return 0;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &png_info_ptr, &png_end_ptr);
        return 0;
    }

    png_set_read_fn(png_ptr, In, ReadDataFromInputStream);
    //png_init_io(png_ptr, pngFile);

    return DecodePNG();
}


int LoadPNGFile(char *filename) {
    int resultDecode = 0;
    pngFile = NULL; // source
    png_width = 0;
    png_height = 0;
    png_color_type = 0;
    png_bit_depth = 0;
    png_number_of_passes = 0;
    png_row_pointers = NULL;

    // open png file
    if ((pngFile = fopen(filename,"rb")) == NULL)
        return 0;

    fread(header, 1, 8, pngFile);
    if (png_sig_cmp(header, 0, 8)) {
        fclose(pngFile);
        pngFile = NULL;
        return 0;
    }

    // Initialize png read
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr==NULL) {
        fclose(pngFile);
        return 0;
    }
    png_info_ptr = png_create_info_struct(png_ptr);
    if (png_info_ptr==NULL) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(pngFile);
        pngFile = NULL;
        return 0;
    }
    png_end_ptr = png_create_info_struct(png_ptr);
    if (png_info_ptr==NULL) {
        png_destroy_read_struct(&png_ptr, &png_info_ptr, (png_infopp)NULL);
        fclose(pngFile);
        pngFile = NULL;
        return 0;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &png_info_ptr, &png_end_ptr);
        fclose(pngFile);
        pngFile = NULL;
        return 0;
    }

    png_init_io(png_ptr, pngFile);

    resultDecode = DecodePNG();
    PNG_CLOSE_FILE();
    return resultDecode;
}

int ExtractPNG(DgSurf **S) {
    int irow,iscan;
    short *outScan;
    unsigned char *ScanPtr;
    int BfPos;

    // no mem ? no RGB ? no grayscale
    if (CreateSurf(S,png_width,png_height,16)==0) {
        return 0;
    }
    // RGB
    if (png_color_type==PNG_COLOR_TYPE_RGB) {
        // get image scanlines
        for (iscan=0; iscan<png_height; iscan++) {

            outScan=(short*)((*S)->rlfb+((*S)->ScanLine*iscan));
            ScanPtr=(unsigned char*)png_row_pointers[iscan];
            BfPos=0;
            // RGB 24 -> BGR 16 (565)
            for (irow=0; irow<png_width; irow++) {
                outScan[irow]=(ScanPtr[BfPos+2]>>3)|((ScanPtr[BfPos+1]>>2)<<5)| ((ScanPtr[BfPos]>>3)<<11);
                BfPos+=3;
            }
        }
    } else if (png_color_type==PNG_COLOR_TYPE_GRAY) { // GRAY
        // get image scanlines
        for (iscan=0; iscan<png_height; iscan++) {
            outScan=(short*)((*S)->rlfb+((*S)->ScanLine*iscan));
            ScanPtr=(unsigned char*)png_row_pointers[iscan];
            // GRAY 8 -> BGR 16 (565)
            for (irow=0; irow<png_width; irow++) {
                outScan[irow]=(ScanPtr[irow]>>3)|((ScanPtr[irow]>>2)<<5)| ((ScanPtr[irow]>>3)<<11);
            }
        }
    } else if (png_color_type==PNG_COLOR_TYPE_RGBA) { // RGBA
        // get image scanlines
        for (iscan=0; iscan<png_height; iscan++) {

            outScan=(short*)((*S)->rlfb+((*S)->ScanLine*iscan));
            ScanPtr=(unsigned char*)png_row_pointers[iscan];
            BfPos=0;
            // RGB 24 -> BGR 16 (565) or black if transparent
            for (irow=0; irow<png_width; irow++) {
                if(ScanPtr[BfPos+3]!=0)
                    outScan[irow]=(ScanPtr[BfPos+2]>>3)|((ScanPtr[BfPos+1]>>2)<<5)| ((ScanPtr[BfPos]>>3)<<11);
                else
                    outScan[irow] = 0;
                BfPos+=4;
            }
        }
    }

    CloseOpenPNG();
    return 1;

}

int LoadMemPNG16(DgSurf **S, void *In, int SizeIn) {
    if (LoadPNGMem(In, SizeIn) == 0)
        return 0;

    return ExtractPNG(S);
}

int LoadPNG16(DgSurf **S,char *filename) {
    if (LoadPNGFile(filename) == 0)
        return 0;

    return ExtractPNG(S);
}


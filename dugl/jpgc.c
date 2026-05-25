#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <png.h>
#include <setjmp.h>

#include "DUGL.h"
// JPEG ////////////////////////////////////////////////////////////////////

// jpeg error handling

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  //(*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


int GetJpegImg(DgSurf **S,j_decompress_ptr cinfo) {
   JSAMPROW row_pointer[1];
   int irow,iscan;
   short *outScan;
   unsigned char *ScanPtr;
   int BfPos;
   char cgray;

   // read the jpeg image header
   jpeg_read_header(cinfo, TRUE);

   // valid jpeg
   if (cinfo->image_width<=0 || cinfo->image_height<=0) {
     jpeg_destroy_decompress(cinfo);
     return 0;
   }

   // no mem ? no RGB ? no grayscale
   if ((CreateSurf(S,cinfo->image_width,cinfo->image_height,16)==0) ||
       ((cinfo->num_components!=3) && (cinfo->num_components!=1)) ) {
     jpeg_destroy_decompress(cinfo);
     return 0;
   }
   // start decompress
   jpeg_start_decompress(cinfo);
   // alloc RGB 24bpp scanline
   row_pointer[0] =
     (unsigned char *)malloc(cinfo->output_width*cinfo->num_components);
   ScanPtr=row_pointer[0];
   // RGB 24bpp ?
   if (cinfo->num_components==3) {
     // get image scanlines
     for (iscan=0;cinfo->output_scanline<cinfo->image_height;iscan++) {

       jpeg_read_scanlines(cinfo,row_pointer, 1);
       outScan=(short*)((*S)->rlfb+((*S)->ScanLine*iscan));
       BfPos=0;
       // RGB 24 -> BGR 16 (565)
       for (irow=0;irow<cinfo->image_width;irow++) {
         outScan[irow]=(ScanPtr[BfPos+2]>>3)|((ScanPtr[BfPos+1]>>2)<<5)|
                 ((ScanPtr[BfPos]>>3)<<11);
         BfPos+=3;
       }
     }
   }
   // gray scale 8bpp ?
   if (cinfo->num_components==1) {
     // get image scanlines
     for (iscan=0;cinfo->output_scanline<cinfo->image_height;iscan++) {

       jpeg_read_scanlines(cinfo,row_pointer, 1);
       outScan=(short*)((*S)->rlfb+((*S)->ScanLine*iscan));
       // gray 8bpp -> BGR 16 (565)
       for (irow=0;irow<cinfo->image_width;irow++) {
         cgray=ScanPtr[irow];
         outScan[irow]=(cgray>>3)|((cgray>>2)<<5)|((cgray>>3)<<11);
       }
     }
   }

   // free ressources
   jpeg_finish_decompress(cinfo);
   jpeg_destroy_decompress(cinfo);

   free(row_pointer[0]);
   return 1;

}

int LoadJPG16(DgSurf **S,char *filename) {
   FILE *jpgFile; // source
   int retGet;
   // init jpeg
   struct jpeg_decompress_struct cinfo;
//   struct jpeg_error_mgr jerr;
   struct my_error_mgr jerr;

   // open jpeg file
   jpgFile=fopen(filename,"rb");
   if (jpgFile==NULL)
     return 0;

   /* We set up the normal JPEG error routines, then override error_exit. */
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit;
   /* libjpeg will jump here if any error occured */
   if (setjmp(jerr.setjmp_buffer)) {
     jpeg_destroy_decompress(&cinfo);
     fclose(jpgFile);
     return 0;
   }
   jpeg_create_decompress(&cinfo);

   // attach the file as source
   jpeg_stdio_src(&cinfo, jpgFile);

   retGet=GetJpegImg(S,&cinfo);
   // close file
   fclose(jpgFile);

   return retGet;
}

// LoadMemJpeg16 -------------------------------------

typedef struct my_src_mgr my_src_mgr;
struct my_src_mgr
{
        struct jpeg_source_mgr pub;
        JOCTET eoi_buffer[2];
};

static void init_source(j_decompress_ptr cinfo)
{
}

static int fill_input_buffer(j_decompress_ptr cinfo)
{
        return 1;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
        my_src_mgr *src = (void *)cinfo->src;
        if (num_bytes > 0)
        {
                while (num_bytes > (long)src->pub.bytes_in_buffer)
                {
                        num_bytes -= (long)src->pub.bytes_in_buffer;
                        fill_input_buffer(cinfo);
                }
        }
        src->pub.next_input_byte += num_bytes;
        src->pub.bytes_in_buffer -= num_bytes;
}

static void term_source(j_decompress_ptr cinfo)
{
}

void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char const *buffer, size_t bufsize)
{
        my_src_mgr *src;
        if (! cinfo->src)
        {
                cinfo->src = (*cinfo->mem->alloc_small)((void *)cinfo, JPOOL_PERMANENT, sizeof(my_src_mgr));;
        }
        src = (void *)cinfo->src;
        src->pub.init_source = init_source;
        src->pub.fill_input_buffer = (boolean (*)(struct jpeg_decompress_struct *))fill_input_buffer;
        src->pub.skip_input_data = skip_input_data;
        src->pub.resync_to_restart = jpeg_resync_to_restart;
        src->pub.term_source = term_source;
        src->pub.next_input_byte = buffer;
        src->pub.bytes_in_buffer = bufsize;
}

int LoadMemJPG16(DgSurf **S,void *buffJpeg,int sizeBuff) {
   // init jpeg
   struct jpeg_decompress_struct cinfo;
//   struct jpeg_error_mgr jerr;
   struct my_error_mgr jerr;
   int retGet;

   // valid buffer ? size ?
   if (buffJpeg==NULL || sizeBuff<=1)
     return 0;

   /* We set up the normal JPEG error routines, then override error_exit. */
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit;
   /* libjpeg will jump here if any error occured */
   if (setjmp(jerr.setjmp_buffer)) {
     jpeg_destroy_decompress(&cinfo);
     return 0;
   }
   jpeg_create_decompress(&cinfo);

   // attach the file as source
   jpeg_memory_src(&cinfo, buffJpeg,sizeBuff);

   retGet=GetJpegImg(S,&cinfo);

   return retGet;
}


int SaveJPG16(DgSurf *S,char *filename,int quality) {
   FILE *jpgFile; // destination
   int retGet;
   int irow,iscan,BfPos;
   // init jpeg
   struct jpeg_compress_struct cinfo;
   //   struct jpeg_error_mgr jerr;
   struct my_error_mgr jerr;
   // scanline jpeg data to compress
   JSAMPROW row_pointer[1];
   unsigned char *ScanPtr;
   unsigned short *InScan;

   // invalid DgSurf
   if (S==NULL) return 0;

   // alloc line data
   row_pointer[0] = (unsigned char *)malloc(S->ResH*3);
   ScanPtr=row_pointer[0];

   // non mem ?
   if (row_pointer[0]==NULL)
     return 0;

   // open jpeg file
   jpgFile=fopen(filename,"wb");
   if (jpgFile==NULL)
     return 0;

   /* We set up the normal JPEG error routines, then override error_exit. */
   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit;
   /* libjpeg will jump here if any error occured */
   if (setjmp(jerr.setjmp_buffer)) {
     free(row_pointer[0]);
     jpeg_destroy_compress(&cinfo);
     fclose(jpgFile);
     return 0;
   }


   // create the compress
   jpeg_create_compress(&cinfo);
   // specify destination
   jpeg_stdio_dest(&cinfo,jpgFile);
   // setting parameter
   cinfo.image_width=S->ResH;
   cinfo.image_height=S->ResV;
   cinfo.input_components=3;
   cinfo.in_color_space=JCS_RGB;
   // set defaults
   jpeg_set_defaults(&cinfo);
   // set quality
   jpeg_set_quality(&cinfo,quality, TRUE);

   // start compressing
   jpeg_start_compress(&cinfo,TRUE);

   for (iscan=0;cinfo.next_scanline<cinfo.image_height;iscan++) {

     InScan=(short*)(S->rlfb+(S->ScanLine*iscan));
     BfPos=0;
     // BGR 16 -> RGB 24
     for (irow=0;irow<S->ResH;irow++) {
       ScanPtr[BfPos+2]=(InScan[irow]&0x1f)<<3;
       ScanPtr[BfPos+1]=(InScan[irow]&0x7e0)>>3;
       ScanPtr[BfPos+0]=(InScan[irow]&0xf800)>>8;
       BfPos+=3;
     }

//   while (cinfo.next_scanline<cinfo.image_height) {

     jpeg_write_scanlines(&cinfo,row_pointer,1);
   }

   // free ressources
   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);
   free(row_pointer[0]);
   fclose(jpgFile);
   return 1;
}



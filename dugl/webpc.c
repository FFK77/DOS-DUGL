#include <stdio.h>
#include <stdlib.h>
#include <webp/decode.h>

#include "DUGL.h"
// WEBP ////////////////////////////////////////////////////////////////////


int LoadWEBP16(DgSurf **S,char *filename) {
    int retGet = 0;
    DFileBuffer* imgFBuff = CreateMemDFileBufferFromFile(filename, "rb");
    if (imgFBuff != NULL) {
        retGet = LoadMemWEBP16(S, (void*)imgFBuff->m_data, (int)imgFBuff->m_bytesInBuff);
        DestroyDFileBuffer(imgFBuff);
    } //else printf("failed reading '%s'\n", filename);

    return retGet;
}

int LoadMemWEBP16(DgSurf **S,void *buffwebp,int sizeBuff) {
    int retGet = 0;
    int iwidth = 0, iheight = 0;
    WebPDecoderConfig config;
    WebPBitstreamFeatures wfeatures;

    if (WebPInitDecoderConfig(&config)) {
        if(WebPGetFeatures((uint8_t*)buffwebp, (size_t) sizeBuff, &wfeatures)== VP8_STATUS_OK && wfeatures.has_animation == 0) {
            //printf("webp get info success (width, height)(%i, %i)\n", iwidth, iheight);
            uint8_t *imgRGBA = WebPDecodeRGBA((uint8_t*)buffwebp, (size_t) sizeBuff, &iwidth, &iheight);
            if (imgRGBA != NULL) {
                //printf("success decoding webp!\n");
                if (CreateSurf(S, iwidth, iheight, 16)) {
                    // convert RGBA to RGB16
                    for (int pix=0; pix < (*S)->SizeSurf/2; pix++) {
                        if (imgRGBA[pix*4+3] != 0)
                            ((short*)((*S)->rlfb))[pix] = RGB16(imgRGBA[pix*4], imgRGBA[pix*4+1], imgRGBA[pix*4+2]);
                        else
                            ((short*)((*S)->rlfb))[pix] = 0;
                    }
                    retGet = 1;
                }
                WebPFree(imgRGBA);
            } //else //printf("failure decoding webp!\n");
        } //else //printf("failed webp get info!\n");
    }// failure to init Decoder Config

   return retGet;
}





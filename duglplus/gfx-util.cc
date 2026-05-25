#include <stdio.h>
#include <stdlib.h>
#include <DUGL.h>

// resize a 16bpp DgSurf into an another DgSurf
void resizeSurf16(DgSurf *SDstSurf,DgSurf *SSrcSurf, bool swapHz = false, bool swapVt = false)
{

  if (SDstSurf->BitsPixel != 16 || SSrcSurf->BitsPixel != 16)
    return;

  if (!swapHz && !swapVt &&
      SDstSurf->ResH == SSrcSurf->ResH && SDstSurf->ResV == SSrcSurf->ResV) {
     SurfCopy(SDstSurf, SSrcSurf);
     return;
  }

  DgSurf OldSurf;

  // Get Current DgSurf
  DgGetCurSurf(&OldSurf);

  // set dest DgSurf as destination
  DgSetCurSurf(SDstSurf);

  // draw the resize polygone inside the dest DgSurf
  ResizeViewSurf16(SSrcSurf, swapHz, swapVt);

  // restore
  DgSetCurSurf(&OldSurf);
}

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DUGL.h"

#define BMFONT_LSIGN_INFO       "info "
#define BMFONT_LSIGN_COMMON     "common "
#define BMFONT_LSIGN_PAGE       "page "
#define BMFONT_LSIGN_CHARS      "chars "
#define BMFONT_LSIGN_CHAR       "char "
#define BMFONT_LSIGN_KERNINGS   "kernings "
#define BMFONT_LSIGN_KERNING    "kerning "

#define BMFONT_INFO_ID          0
#define BMFONT_COMMON_ID        1
#define BMFONT_PAGE_ID          2
#define BMFONT_CHARS_ID         3
#define BMFONT_CHAR_ID          4
#define BMFONT_KERNINGS_ID      5
#define BMFONT_KERNING_ID       6

bool LoadBMFONT(DBMFONT **resDBMFONT, char *fileDescName, DgSurf *AllCharsSurf) {
	DFileBuffer *FileBuffer = NULL;
    DBMFONT *newBMFont = malloc(sizeof(DBMFONT));
    DSplitString *ListInfoLine = CreateDSplitString(0, 128);
    DSplitString *QuoteSplitter = CreateDSplitString(3, 2048);
    static char* BMMultiDelims = "= ";
    static int countLineSigns = 7;
    static char *BMFontLineSigns[] = {
        BMFONT_LSIGN_INFO,
        BMFONT_LSIGN_COMMON,
        BMFONT_LSIGN_PAGE,
        BMFONT_LSIGN_CHARS,
        BMFONT_LSIGN_CHAR,
        BMFONT_LSIGN_KERNINGS,
        BMFONT_LSIGN_KERNING
    };
    static int BMFontSignsLength[] = {
        sizeof(BMFONT_LSIGN_INFO)-1,
        sizeof(BMFONT_LSIGN_COMMON)-1,
        sizeof(BMFONT_LSIGN_PAGE)-1,
        sizeof(BMFONT_LSIGN_CHARS)-1,
        sizeof(BMFONT_LSIGN_CHAR)-1,
        sizeof(BMFONT_LSIGN_KERNINGS)-1,
        sizeof(BMFONT_LSIGN_KERNING)-1
    };

    if (newBMFont == NULL || AllCharsSurf == NULL || ListInfoLine == NULL || QuoteSplitter == NULL) {
        return false;
    }
    FileBuffer = CreateDFileBuffer(0);
    if (FileBuffer == NULL) {
        free(newBMFont);
        return false;
    }

    if (!OpenFileDFileBuffer(FileBuffer, fileDescName, "rt")) {
        DestroySurf(newBMFont->CharsMainSurf);
        DestroyDFileBuffer(FileBuffer);
        free(newBMFont);
        return false;
	}

	newBMFont->CharsMainSurf = AllCharsSurf;
	// init BMFONT with dummy
	memcpy(newBMFont, &dummyDBMFONT, sizeof(DBMFONT));
	// set main source DgSurf
	newBMFont->CharsMainSurf = AllCharsSurf;

    // parse fnt description file
    char *nexBMLine = (char*)malloc(ListInfoLine->maxGlobLength+1);
    SetMultiDelimDSplitString(ListInfoLine, BMMultiDelims); // set list of delimiters for MultiDelimSplit
    bool infoLineFound = false;
    bool commonLineFound = false;
    bool pageLineFound = false;
    bool validFileFormat = true;

	while (!IsEndOfFileDFileBuffer(FileBuffer) && validFileFormat) {
        if (GetLineDFileBuffer(FileBuffer, nexBMLine, ListInfoLine->maxGlobLength) == 0)
            continue; // ignore empty line

        // detect line signature
        int LineSignIdx = -1;
        int globCharsPlus = 0;
        for (int i = 0; i < countLineSigns; i++) {
            if (strncmp(BMFontLineSigns[i], nexBMLine, BMFontSignsLength[i]) == 0) {
                LineSignIdx = i;
                globCharsPlus = BMFontSignsLength[i];
                break;
            }
        }
        if (LineSignIdx == -1) {
            // unknow line! // issue warning
        }
        else {
            // try jump "face" attribut for "info " line
            if (LineSignIdx == BMFONT_INFO_ID && !infoLineFound) {
                if (strncmp(&nexBMLine[globCharsPlus], "face", 4) == 0) {
                    int countQuotes = 0;
                    while (countQuotes < 2 && nexBMLine[globCharsPlus] != '\0') {
                        globCharsPlus++;
                        if (nexBMLine[globCharsPlus] == '"') {
                            countQuotes++;
                            globCharsPlus++;
                        }
                    }
                }
            }
            // extract attributs
            splitMultiDelimDSplitString(ListInfoLine, &nexBMLine[globCharsPlus], false);
            TrimStringsDSplitString(ListInfoLine);

            switch (LineSignIdx) {
                case BMFONT_INFO_ID:
                    // handle only once
                    if (!infoLineFound) {
                        infoLineFound = true;
                        // extract attributs
                    }
                    break;

                case BMFONT_COMMON_ID:
                    // handle only once
                    if (!commonLineFound) {
                        commonLineFound = true;
                        int commonAttsFound = 0;
                        int commonLineHeight = 0;
                        int commonBase = 0;
                        int commonScaleW = 0;
                        int commonScaleH = 0;
                        // extract attributs
                        for (int i = 0; i < ListInfoLine->countStrings; i+=2) {
                            if (!strcmp("lineHeight", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    commonAttsFound |= 1;
                                    commonLineHeight = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("base", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    commonAttsFound |= 1 << 1;
                                    commonBase = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("scaleW", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    commonAttsFound |= 1 << 2;
                                    commonScaleW = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("scaleH", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    commonAttsFound |= 1 << 3;
                                    commonScaleH = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            }
                        }
                        if (commonAttsFound != 0xf ||
                            newBMFont->CharsMainSurf->ResH != commonScaleW ||
                            newBMFont->CharsMainSurf->ResV != commonScaleH) {
                            validFileFormat = false;
                            //printf("invalid common file Format");
                        } else {
                            newBMFont->CharsGHeight = commonLineHeight;
                            newBMFont->CharsGLineHeight = commonBase;
                        }
                    }
                    break;

                case BMFONT_PAGE_ID:
                    // handle only once
                    if (!pageLineFound) {
                        pageLineFound = true;
                    }
                    break;

                case BMFONT_CHARS_ID:
                    // ignore
                    break;

                case BMFONT_CHAR_ID:
                    if (commonLineFound) {
                        int charAttsFound = 0;
                        int charID = 0;
                        int charX = 0;
                        int charY = 0;
                        int charWIDTH = 0;
                        int charHEIGHT = 0;
                        int charXOFFSET = 0;
                        int charYOFFSET = 0;
                        int charXADVANCE = 0;
                        // converted data
                        int charOrgY = 0; // from descending to ascending y axis
                        // extract attributs
                        for (int i = 0; i < ListInfoLine->countStrings; i+=2) {
                            if (!strcmp("id", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1;
                                    charID = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("x", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1 << 1;
                                    charX = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("y", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1 << 2;
                                    charY = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("width", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1 << 3;
                                    charWIDTH = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("height", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1 << 4;
                                    charHEIGHT = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("xoffset", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1 << 5;
                                    charXOFFSET = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("yoffset", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1 << 6;
                                    charYOFFSET = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            } else if (!strcmp("xadvance", ListInfoLine->ListStrings[i]) && (i+1) < ListInfoLine->countStrings) {
                                if (splitDSplitString(QuoteSplitter, ListInfoLine->ListStrings[i+1], '"', false) == 1) {
                                    charAttsFound |= 1 << 7;
                                    charXADVANCE = atoi(QuoteSplitter->ListStrings[0]);
                                }
                            }
                        }
                        if (charAttsFound != 0xff || charID == 0 || charID > 255) {
                            //printf("invalid char line %i\n", charID);
                            // invalid line, missing attributs
                        } else {
                            // convert to ascending
                            charOrgY = newBMFont->CharsMainSurf->MaxY - charY - charHEIGHT+1;

                            if (charX + (charWIDTH - 1) <= newBMFont->CharsMainSurf->MaxX && charOrgY + charHEIGHT-1 <= newBMFont->CharsMainSurf->MaxY) {
                                if (charWIDTH > 0 && charHEIGHT > 0) {
                                    DgView charView = { .OrgX = charX, .OrgY = charOrgY, .MaxX = charWIDTH-1, .MaxY = charHEIGHT-1, .MinX = 0, .MinY = 0 };
                                    CreateSurfBuffView(&newBMFont->CharsSSurfs[charID], newBMFont->CharsMainSurf->ResH, newBMFont->CharsMainSurf->ResV, 16,
                                                       (void*)newBMFont->CharsMainSurf->rlfb, &charView);
                                }
                                newBMFont->CharsPlusX[charID] = charXADVANCE;
                                newBMFont->CharsWidth[charID] = charWIDTH;
                                newBMFont->CharsXOffset[charID] = charXOFFSET;
                                newBMFont->CharsYOffset[charID] = charYOFFSET;
                                newBMFont->CharsHeight[charID] = charHEIGHT;
                            } else {
                                //printf("Warning '%c' ignored out of Image!\n", charID);
                            }
                        }
                    } else {
                        validFileFormat = false;
                        //printf("invalid common file Format");
                    }

                    break;

                case BMFONT_KERNINGS_ID:
                    // ignore
                    break;

                case BMFONT_KERNING_ID:
                    // ignore
                    break;
                default:
                    // ignore / issue warning ?

            }
        }
	}

    free(nexBMLine);
	DestroyDFileBuffer(FileBuffer);
	DestroyDSplitString(QuoteSplitter);
	DestroyDSplitString(ListInfoLine);

	*resDBMFONT = newBMFont;
    return validFileFormat;
}

void DestroyBMFONT(DBMFONT *pBMFONT) {
    if (pBMFONT == NULL)
        return;
    if (pBMFONT->CharsMainSurf != NULL) {
        DestroySurf(pBMFONT->CharsMainSurf);
        free(pBMFONT);
    } else {
        return;
    }
}

int  WidthBMText(DBMFONT *pBMFONT, char *str) {
    unsigned char *strIt = (unsigned char*)str;
    int widthText = 0;
    unsigned char lastChar = 0;

    if (strIt == NULL || pBMFONT == NULL)
        return 0;
    while (*strIt != '\0') {
        widthText += pBMFONT->CharsPlusX[(lastChar = (*strIt))];
        strIt ++;
    }
    // special handling for last char: remove plus and add char width
    widthText -= pBMFONT->CharsPlusX[lastChar];
    widthText += pBMFONT->CharsWidth[lastChar];

    return widthText;
}

int  WidthPosBMText(DBMFONT *pBMFONT, char *str, int pos) {
    unsigned char *strIt = (unsigned char*)str;
    int widthText = 0;
    unsigned char lastChar = 0;

    if (strIt == NULL || pBMFONT == NULL)
        return 0;
    for (int i =0; i < pos &&  (*strIt) != '\0'; i++, strIt ++) {
        widthText += pBMFONT->CharsPlusX[(lastChar = (*strIt))];
    }
    // special handling for last char: remove plus and add char width
    widthText -= pBMFONT->CharsPlusX[lastChar];
    widthText += pBMFONT->CharsWidth[lastChar];
    return widthText;
}

// Position in *str if we progress by "width" pixels, return -1 if we crossed string boundaries
int  PosWidthBMText(DBMFONT *pBMFONT, char *str,int  width) {
    unsigned char *strIt = (unsigned char*)str;
    int startX = 0;
    int widthText = 0;

    if (strIt == NULL || pBMFONT == NULL)
        return 0;
    for (int i =0; (*strIt) != '\0'; i++, strIt ++) {
        if (strIt[1] != '\0') // next char isn't the end,
            widthText += pBMFONT->CharsPlusX[(*strIt)];
        else
            widthText += pBMFONT->CharsWidth[(*strIt)];
        if (width >= startX && width <= widthText)
            return i;
        startX = widthText;
    }
    return -1;
}

void OutTextBMMode16(char *str, DBMFONT_ADJUST bmfont_adjust) {
    int initX = CurDBMFONT.CharX;
    int widthLineText = 0;

	switch (bmfont_adjust) {
	  case BMFONT_ADJUST_SRC:
        CurDBMFONT.CharX = initX;
	    break;
	  case BMFONT_ADJUST_MID:
	    widthLineText=WidthBMText(&CurDBMFONT, str);
	    CurDBMFONT.CharX=(CurSurf.MinX+CurSurf.MaxX)/2-widthLineText/2;
	    break;
	  case BMFONT_ADJUST_DST:
        widthLineText=WidthBMText(&CurDBMFONT, str);
	    CurDBMFONT.CharX=(CurSurf.MaxX-widthLineText);
	    break;
	  default:
	    return;
	}
	OutTextBM16(str);
}

void OutMultiLinesTextBM16(char *str, int MaxLines, DBMFONT_ADJUST bmfont_adjust) {
    DSplitString *linesSplitter = CreateStrDSplitString(MaxLines, str);
    if (linesSplitter == NULL) {
        return;
    }
    int initX = CurDBMFONT.CharX;

    if (splitDSplitString(linesSplitter, NULL, '\n', true) > 0) {
        for (int i = 0; i < MaxLines && i < linesSplitter->countStrings; i++) {
            OutTextBMMode16(linesSplitter->ListStrings[i], bmfont_adjust);
            CurDBMFONT.CharY -= CurDBMFONT.CharsGHeight; // move to next line
            CurDBMFONT.CharX = initX; // restore startX
        }
    }
    DestroyDSplitString(linesSplitter);
}

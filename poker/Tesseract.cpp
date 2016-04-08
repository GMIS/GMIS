// Tesseract.cpp: implementation of the Tesseract class.
//
//////////////////////////////////////////////////////////////////////


#include "Tesseract.h"
#include "bmp.h"
#include <assert.h>
#include "BaseEnergy.h"


CTesseract::CTesseract(){
	m_InitOK = false;
}

CTesseract::~CTesseract(){

}

bool CTesseract::SetLang(const char* LangDir,const char* LangName){
	if(m_InitOK){
		End();
		m_InitOK = false;
	}

	int ret = Init(LangDir, LangName, tesseract::OEM_DEFAULT);
	
	if (ret==-1)
	{
		End();
		m_InitOK = false;
	}else{	
		SetPageSegMode(tesseract::PSM_AUTO);
		m_InitOK = true;
	}
	return m_InitOK;
}


std::string CTesseract::OCR(tstring  FileName){
	std::string  s;

	if (!m_InitOK)
	{
		return s;
	}

	AnsiString FileName1 = eSTRING::UTF16TO8(FileName);

	try
	{
		STRING text_out;	
		ProcessPages(FileName1.c_str(),NULL,0,&text_out);

		s = text_out.string();	
	}
	catch (...)
	{
	}

	return s;
};


	
std::string CTesseract::OCR(PIX* pix){
	if (!m_InitOK)
	{
		return "";
	};

	if (pix == NULL)
	{
		return "";
	};
	
	STRING text_out;	
	ProcessPage(pix,0,"nofile",NULL,0,&text_out);
	
	std::string s = text_out.string();

	return s;
}
std::string CTesseract::OCR(CBitmapEx& Image){
	std::string s;

	const int size = 512*512;
	static BYTE buf[size];

	long ImageSize;	
	ImageSize = Image.Save(buf,size);

	if(ImageSize==0)
	{
		return s;
	};

	PIX*  pix = GetPixFromeStreamBmp((LPBYTE)&buf,ImageSize);
	if (pix)
	{
		s = OCR(pix);
		pixDestroy(&pix);
	}
	return s;
}

int MemRead(void* Dest, int size, int num,unsigned char* ImageData,int& pos,int DataSize)
{
	int n=size*num;
	memcpy(Dest,ImageData+pos,n);
	pos+=n;
	assert(pos<=DataSize);
	return n;
}

PIX * GetPixFromeStreamBmp(unsigned char* ImageData,int DataSize)
{
	l_uint16   sval;
	l_uint32   ival;
	l_int16    bfType, bfSize, bfFill1, bfReserved1, bfReserved2;
	l_int16    offset, bfFill2, biPlanes, depth, d;
	l_int32    biSize, width, height, xres, yres, compression, ignore;
	l_int32    imagebytes, biClrUsed, biClrImportant;
	l_uint8   *colormapBuf;
	l_int32    colormapEntries;
	l_int32    fileBpl, extrabytes, readerror;
	l_int32    pixWpl, pixBpl;
	l_int32    i, j, k;
	l_uint8    pel[4];
	l_uint8   *data;
	l_uint32  *line, *pword;
	PIX        *pix, *pixt;
	PIXCMAP   *cmap;
	


    if (!ImageData)
        return  NULL;
    int pos=0;

        /* Read bitmap file header */
    ignore = MemRead((char *)&sval,1,2,ImageData,pos,DataSize);
    bfType = convertOnBigEnd16(sval);
    if (bfType != BMP_ID)
        return NULL;

    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    bfSize = convertOnBigEnd16(sval);
    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    bfFill1 = convertOnBigEnd16(sval);
    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    bfReserved1 = convertOnBigEnd16(sval);
    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    bfReserved2 = convertOnBigEnd16(sval);
    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    offset = convertOnBigEnd16(sval);
    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    bfFill2 = convertOnBigEnd16(sval);

        /* Read bitmap info header */
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    biSize = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    width = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    height = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    biPlanes = convertOnBigEnd16(sval);
    ignore = MemRead((char *)&sval, 1, 2, ImageData,pos,DataSize);
    depth = convertOnBigEnd16(sval);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    compression = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    imagebytes = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    xres = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    yres = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    biClrUsed = convertOnBigEnd32(ival);
    ignore = MemRead((char *)&ival, 1, 4, ImageData,pos,DataSize);
    biClrImportant = convertOnBigEnd32(ival);

    if (compression != 0)
        return NULL;

        /* A little sanity checking.  It would be nice to check
         * if the number of bytes in the file equals the offset to
         * the data plus the imagedata, but this won't work when
         * reading from memory, because fmemopen() doesn't implement
         * ftell().  So we can't do that check.  The imagebytes for
         * uncompressed images is either 0 or the size of the file data.
         * (The fact that it can be 0 is perhaps some legacy glitch).  */
    if (width < 1)
        return NULL;
    if (height < 1)
        return NULL;
    if (depth < 1 || depth > 32)
        return NULL;
    fileBpl = 4 * ((width * depth + 31)/32);
    if (imagebytes != 0 && imagebytes != fileBpl * height)
        return NULL;
    if (offset < BMP_FHBYTES + BMP_IHBYTES)
        return NULL;
    if (offset > BMP_FHBYTES + BMP_IHBYTES + 4 * 256)
        return NULL;

        /* Handle the colormap */
    colormapEntries = (offset - BMP_FHBYTES - BMP_IHBYTES) / sizeof(RGBA_QUAD);
    if (colormapEntries > 0) {
        if ((colormapBuf = (l_uint8 *)CALLOC(colormapEntries,
                                             sizeof(RGBA_QUAD))) == NULL)
            return NULL;

            /* Read colormap */
        if (MemRead(colormapBuf, sizeof(RGBA_QUAD), colormapEntries, ImageData,pos,DataSize)
                 != colormapEntries) {
            FREE(colormapBuf);
            return NULL;
        }
    }

        /* Make a 32 bpp pix if depth is 24 bpp */
    d = depth;
    if (depth == 24)
        d = 32;
    if ((pix = pixCreate(width, height, d)) == NULL)
        return NULL;
    pixSetXRes(pix, (l_int32)((l_float32)xres / 39.37 + 0.5));  /* to ppi */
    pixSetYRes(pix, (l_int32)((l_float32)yres / 39.37 + 0.5));  /* to ppi */
    pixWpl = pixGetWpl(pix);
    pixBpl = 4 * pixWpl;

    cmap = NULL;
    if (colormapEntries > 256)
    {  
		  //L_WARNING("more than 256 colormap entries!", procName);
	}
	if (colormapEntries > 0) {  /* import the colormap to the pix cmap */
        cmap = pixcmapCreate(L_MIN(d, 8));
        FREE(cmap->array);  /* remove generated cmap array */
        cmap->array  = (void *)colormapBuf;  /* and replace */
        cmap->n = L_MIN(colormapEntries, 256);
    }
    pixSetColormap(pix, cmap);

    /* Seek to the start of the bitmap in the file */
    pos = 0;//fseek(fp, offset, 0);
    

    if (depth != 24) {  /* typ. 1 or 8 bpp */
        data = (l_uint8 *)pixGetData(pix) + pixBpl * (height - 1);
        for (i = 0; i < height; i++) {
            if (MemRead(data, 1, fileBpl, ImageData,pos,DataSize) != fileBpl) {
                pixDestroy(&pix);
                return NULL;
            }
            data -= pixBpl;
        }
    }
    else {  /*  24 bpp file; 32 bpp pix
             *  Note: for bmp files, pel[0] is blue, pel[1] is green,
             *  and pel[2] is red.  This is opposite to the storage
             *  in the pix, which puts the red pixel in the 0 byte,
             *  the green in the 1 byte and the blue in the 2 byte.
             *  Note also that all words are endian flipped after
             *  assignment on L_LITTLE_ENDIAN platforms.
             *
             *  We can then make these assignments for little endians:
             *      SET_DATA_BYTE(pword, 1, pel[0]);      blue
             *      SET_DATA_BYTE(pword, 2, pel[1]);      green
             *      SET_DATA_BYTE(pword, 3, pel[2]);      red
             *  This looks like:
             *          3  (R)     2  (G)        1  (B)        0
             *      |-----------|------------|-----------|-----------|
             *  and after byte flipping:
             *           3          2  (B)     1  (G)        0  (R)
             *      |-----------|------------|-----------|-----------|
             *
             *  For big endians we set:
             *      SET_DATA_BYTE(pword, 2, pel[0]);      blue
             *      SET_DATA_BYTE(pword, 1, pel[1]);      green
             *      SET_DATA_BYTE(pword, 0, pel[2]);      red
             *  This looks like:
             *          0  (R)     1  (G)        2  (B)        3
             *      |-----------|------------|-----------|-----------|
             *  so in both cases we get the correct assignment in the PIX.
             *
             *  Can we do a platform-independent assignment?
             *  Yes, set the bytes without using macros:
             *      *((l_uint8 *)pword) = pel[2];           red
             *      *((l_uint8 *)pword + 1) = pel[1];       green
             *      *((l_uint8 *)pword + 2) = pel[0];       blue
             *  For little endians, before flipping, this looks again like:
             *          3  (R)     2  (G)        1  (B)        0
             *      |-----------|------------|-----------|-----------|
             */
        readerror = 0;
        extrabytes = fileBpl - 3 * width;
        line = pixGetData(pix) + pixWpl * (height - 1);
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                pword = line + j;
                if (MemRead(&pel, 1, 3, ImageData,pos,DataSize) != 3)
                    readerror = 1;
                *((l_uint8 *)pword + COLOR_RED) = pel[2];
                *((l_uint8 *)pword + COLOR_GREEN) = pel[1];
                *((l_uint8 *)pword + COLOR_BLUE) = pel[0];
            }
            if (extrabytes) {
                for (k = 0; k < extrabytes; k++)
                    ignore = MemRead(&pel, 1, 1, ImageData,pos,DataSize);
            }
            line -= pixWpl;
        }
        if (readerror) {
            pixDestroy(&pix);
            return NULL;
        }
    }

    pixEndianByteSwap(pix);

        /* ----------------------------------------------
         * The bmp colormap determines the values of black
         * and white pixels for binary in the following way:
         * if black = 1 (255), white = 0
         *      255, 255, 255, 0, 0, 0, 0, 0
         * if black = 0, white = 1 (255)
         *      0, 0, 0, 0, 255, 255, 255, 0
         * We have no need for a 1 bpp pix with a colormap!
         * ---------------------------------------------- */
    if (depth == 1 && cmap) {
/*        L_INFO("Removing colormap", procName); */
        pixt = pixRemoveColormap(pix, REMOVE_CMAP_BASED_ON_SRC);
        pixDestroy(&pix);
        pix = pixt;  /* rename */
    }

    return pix;
}



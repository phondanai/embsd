//	set path=C:\MinGW\bin;%path%

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

void abort_program(const char * msg)
{
	printf("abort: (%s) is FALSE!\n", msg);
	exit(-1);
}

#define ENABLE_ASSERT

#ifdef ENABLE_ASSERT
#define ASSERT(n)  if(!(n))  abort_program(#n)
#else
#define ASSERT(n)  (n)
#endif

void BMPIMG_initialize(BMPIMG * img)
{
	img->bitmap = 0;
	img->colormap_size = 0;
	img->size_x = 0;
	img->size_y = 0;
	img->size_xy = 0;
	img->out_pix_bits = 24;
}

void BMPIMG_destroy(BMPIMG * img)
{
	if(img->bitmap){
		free(img->bitmap);
		img->bitmap = 0;
	}
}

int check_bmp_file_extension(const char * fname)
{
	return (strcmp(fname + strlen(fname) - 4, ".bmp") == 0);
}

#define FERROR(fp) (ferror(fp) || feof(fp))

/*******************************************/
unsigned int getshort(FILE * fp)
{
	int c, c1;
	c = getc(fp);  c1 = getc(fp);
	return ((unsigned int) c) + (((unsigned int) c1) << 8);
}


/*******************************************/
unsigned int getint(FILE * fp)
{
	int c, c1, c2, c3;
	c = getc(fp);  c1 = getc(fp);  c2 = getc(fp);  c3 = getc(fp);
	return ((unsigned int) c) +
		(((unsigned int) c1) << 8) + 
		(((unsigned int) c2) << 16) +
		(((unsigned int) c3) << 24);
}

/*******************************************/
void putshort(FILE * fp, int i)
{
	int c, c1;
	
	c = ((unsigned int ) i) & 0xff;
	c1 = (((unsigned int) i)>>8) & 0xff;
	putc(c, fp);   putc(c1,fp);
}

/*******************************************/
void putint(FILE * fp, int i)
{
	int c, c1, c2, c3;
	c  = ((unsigned int ) i)      & 0xff;  
	c1 = (((unsigned int) i)>>8)  & 0xff;
	c2 = (((unsigned int) i)>>16) & 0xff;
	c3 = (((unsigned int) i)>>24) & 0xff;
	
	putc(c, fp);   putc(c1,fp);  putc(c2,fp);  putc(c3,fp);
}
/*******************************************/
int BMPIMG_loadBMP1(BMPIMG * img, FILE * fp, int w, int h)
{
	int   i,j,c,bitnum,padw, count = 0;
	//	byte *pp;
	
	int index;
	PIX * bitmap = img->bitmap;
	c = 0;
	padw = ((w + 31)/32) * 32;  /* 'w', padded to be a multiple of 32 */
	
	for (i=h-1; i>=0; i--) {
		index = (i * w);
		for (j=0,bitnum=0; j<padw; j++,bitnum++) {
			if ((bitnum&7) == 0) { /* read the next byte */
				c = getc(fp);
				if(c == EOF){
					printf("EOF encountered (count = %d/%d)",
						count, h * w / 8);
					break;
				}
				count ++;
				bitnum = 0;
			}
			if (j<w) {
				bitmap[index + j].I = ((c & 0x80) != 0);
				c <<= 1;
			}
		}
		if (FERROR(fp)) break;
	}
	return (FERROR(fp));
}  

/*******************************************/
int BMPIMG_loadBMP4(BMPIMG * img, FILE * fp, int w, int h, int comp)
{
	int   i,j,c,c1,x,y,nybnum,padw,rv;
	
	int count = 0;
	int index;
	PIX * bitmap = img->bitmap;
	rv = 0;
	c = c1 = 0;
	if (comp == BBI_RGB) {   /* read uncompressed data */
		padw = ((w + 7)/8) * 8; /* 'w' padded to a multiple of 8pix (32 bits) */
		
		for (i=h-1; i>=0; i--) {
			index = (i * w);
			for (j=0,nybnum=0; j<padw; j++,nybnum++) {
				if ((nybnum & 1) == 0) { /* read next byte */
					c = getc(fp);
					if(c == EOF){
						printf("EOF encountered (count = %d/%d)",
							count, h * w / 2);
						break;
					}
					count ++;
					nybnum = 0;
				}
				if (j<w) {
					int cc = (c & 0xf0) >> 4;
					bitmap[index + j].I = cc;
					c <<= 4;
				}
			}
			if (FERROR(fp)) break;
		}
	}
	
	else if (comp == BBI_RLE4) {  /* read RLE4 compressed data */
		x = y = 0;  
		index = x + (h - y - 1) * w;
		while (y<h) {
			c = getc(fp);
			if (c == EOF) { rv = 1;  break; }
			if (c) {                     /* encoded mode */
				c1 = getc(fp);
				for (i=0; i<c; i++,x++,index++) {
					int cc = (i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f);
					bitmap[index].I = cc;
				}
			}
			else {    /* c==0x00  :  escape codes */
				c = getc(fp);
				if (c == EOF) { rv = 1;  break; }
				if (c == 0x00) {      /* end of line */
					x=0;  y++;  
					index = x + (h - y - 1) * w;
				} 
				else if (c == 0x01) break;  /* end of pic8 */
				else if (c == 0x02) {       /* delta */
					c = getc(fp);  x += c;
					c = getc(fp);  y += c;
					index = x + (h - y - 1) * w;
				}
				else {                      /* absolute mode */
					for (i=0; i<c; i++, x++, index++) {
						int cc;
						if ((i&1) == 0) c1 = getc(fp);
						cc = (i&1) ? (c1 & 0x0f) : ((c1>>4)&0x0f);
						bitmap[index].I = cc;
					}
					if (((c&3)==1) || ((c&3)==2))
						getc(fp);  /* read pad byte */
				}
			}  /* escape processing */
			if (FERROR(fp)) break;
		}  /* while */
	}
	else {
		printf("unknown BMP compression type 0x%0x\n", comp);
		return 1;
	}
	if (FERROR(fp)) rv = 1;
	return rv;
}  



/*******************************************/
int BMPIMG_loadBMP8(BMPIMG * img, FILE * fp, int w, int h, int comp)
{
	int   i,j,c,c1,padw,x,y,rv, index, count = 0;
	PIX * bitmap = img->bitmap;
	
	rv = 0;
	
	if (comp == BBI_RGB) {   /* read uncompressed data */
		padw = ((w + 3)/4) * 4; /* 'w' padded to a multiple of 4pix (32 bits) */
		
		for (i=h-1; i>=0; i--) {
			index = (i * w);
			for (j=0; j<padw; j++) {
				c = getc(fp);  
				if(c == EOF){
					printf("loadBMP8: EOF encountered (count = %d/%d)",
						count, h * w);
					rv = 1;
					break;
				}
				if (j<w){
					bitmap[index + j].I = c;
					count ++;
				}
			}
			if (FERROR(fp)) 
				break;
		}
	}
	else if (comp == BBI_RLE8) {  /* read RLE8 compressed data */
		x = y = 0;
		index = x + (h-y-1)*w;
		while (y<h) {
			c = getc(fp);
			if (c == EOF) { rv = 1;  break; }
			if (c) {                /* encoded mode */
				c1 = getc(fp);
				for (i=0; i<c; i++,x++,index++){
					int cc = c1;
					bitmap[index].I = cc;
				}
			}
			else {    /* c==0x00  :  escape codes */
				c = getc(fp);
				if (c == EOF) { rv = 1;  break; }
				if (c == 0x00) {  /* end of line */
					x=0;  y++;  
					index = x + (h - y - 1) * w;
				} 
				else if (c == 0x01) break;   /* end of pic8 */
				else if (c == 0x02) {        /* delta */
					c = getc(fp);  x += c;
					c = getc(fp);  y += c;
					index = x + (h-y-1)*w;
				}
				else {        /* absolute mode */
					for (i=0; i<c; i++, x++, index++) {
						c1 = getc(fp);
						bitmap[index].I = c1;
					}
					if (c & 1) getc(fp);  /* odd length run: read an extra pad byte */
				}
			}  /* escape processing */
			if (FERROR(fp)) break;
		}  /* while */
	}
	else {
		printf("unknown BMP compression type 0x%0x\n", comp);
		return 1;
	}
	if (FERROR(fp)) rv = 1;
	return rv;
}  

/*******************************************/
int BMPIMG_loadBMP24(BMPIMG * img, FILE * fp, int w, int h)
{
	int   i,j, padb,rv, index;
	PIX * bitmap = img->bitmap;
	int count = 0, ccc;
	
	rv = 0;
	
	padb = (4 - ((w*3) % 4)) & 0x03;  /* # of pad bytes to read at EOscanline */
	
	for (i=h-1; i>=0; i--) {
		index = (i * w);
		for (j=0; j<w; j++) {
			if((ccc = getc(fp)) == EOF){
				printf("EOF encountered (count = %d/%d)",
					count, h * w * 3);
				break;
			}
			bitmap[index + j].B = ccc;
			if((ccc = getc(fp)) == EOF){
				printf("EOF encountered (count = %d/%d)",
					count, h * w * 3);
				break;
			}
			bitmap[index + j].G = ccc;
			if((ccc = getc(fp)) == EOF){
				printf("EOF encountered (count = %d/%d)",
					count, h * w * 3);
				break;
			}
			bitmap[index + j].R = ccc;
			count += 3;
			if(FERROR(fp)){
				break;
			}
		}
		for (j=0; j<padb; j++) getc(fp);
		rv = (FERROR(fp));
		if (rv)
			break;
	}
	return rv;
}  

int BMPIMG_openBMP(BMPIMG * img, FILE * fp)
{
	
	long filesize;
	
	
	
	int          i, c, c1, rv;
	int bPad;
	//char          buf[512];
	int count = 0;
	
	unsigned int bfSize, bfOffBits, biSize, biWidth, biHeight, biPlanes;
	unsigned int biBitCount, biCompression, biSizeImage, biXPelsPerMeter;
	unsigned int biYPelsPerMeter, biClrUsed, biClrImportant;
	
	
	fseek(fp, 0L, SEEK_END);      /* figure out the file size */
	filesize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	
	
	/* read the file type (first two bytes) */
	c = getc(fp);  c1 = getc(fp);    // Check if it Bit Map format or not
	if (c!='B' || c1!='M') {
		printf("File type is not 'BM' (%c%c)", c, c1);
		fclose(fp);
		return 0;
	}
	
	bfSize = getint(fp);
	getshort(fp);         /* reserved and ignored */
	getshort(fp);
	bfOffBits = getint(fp);
	biSize          = getint(fp);
	
	if (biSize == WIN_NEW || biSize == OS2_NEW) {
		biWidth         = getint(fp);
		biHeight        = getint(fp);
		biPlanes        = getshort(fp);
		biBitCount      = getshort(fp);
		biCompression   = getint(fp);
		biSizeImage     = getint(fp);
		biXPelsPerMeter = getint(fp);
		biYPelsPerMeter = getint(fp);
		biClrUsed       = getint(fp);
		biClrImportant  = getint(fp);
	}
	
	else { 
		printf("Old BMP format not supported. Sorry....");
		fclose(fp);
		return 0;
	}

	img->size_x = biWidth;
	img->size_y = biHeight;
	img->size_xy = img->size_x * img->size_y;
	
	if(FERROR(fp)) {
		printf("EOF reached in file header");
		fclose(fp);
		return 0;
	}
	
	
	/* error checking */
	if((biBitCount!=1 && biBitCount!=4 && biBitCount!=8 && biBitCount!=24) || 
		biPlanes!=1 || biCompression>BBI_RLE4 && biBitCount != 32) {
		
		printf("Bogus BMP File!  (bitCount=%d, Planes=%d, Compression=%d)\n",
			biBitCount, biPlanes, biCompression);
		
		fclose(fp);
		return 0;
	}
	
	if(((biBitCount==1 || biBitCount==24) && biCompression != BBI_RGB) ||
		(biBitCount==4 && biCompression==BBI_RLE8) ||
		(biBitCount==8 && biCompression==BBI_RLE4)) {
		printf("Bogus BMP File!  (bitCount=%d, Compression=%d)\n",
			biBitCount, biCompression);
		
		fclose(fp);
		return 0;
	}
	
	
	bPad = 0;
	if(biSize != WIN_OS2_OLD) {
		/* skip ahead to colormap, using biSize */
		c = biSize - 40;    /* 40 bytes read from biSize to biClrImportant */
		for (i=0; i<c; i++) getc(fp);
		bPad = bfOffBits - (biSize + 14);
	}
	
	/* load up colormap, if any */
	if(biBitCount!=24) {
		int i, cmaplen;
		
		
		cmaplen = (biClrUsed) ? biClrUsed : 1 << biBitCount;
		if(cmaplen > 256){
			printf("colormap size %d is too large. Sorry. it must be 256 or smaller....\n", cmaplen);
			fclose(fp);
			return 0;
		}
		img->colormap_size = cmaplen;
		for (i=0; i<cmaplen; i++) {
			int cc = getc(fp);
			img->colormap[i].B = cc;
			cc = getc(fp);
			img->colormap[i].G = cc;
			cc = getc(fp);
			img->colormap[i].R = cc;
			if (biSize != WIN_OS2_OLD) {
				getc(fp);
				bPad -= 4;
			}
		}
	}
	else
		img->colormap_size = 0;
	
	if (FERROR(fp)){
		printf("EOF reached in BMP colormap\n");
		fclose(fp);
		return 0;
	}
	
	//	CString s, ss;
	
	if (biSize != WIN_OS2_OLD) {
    /* Waste any unused bytes between the colour map (if present)
		and the start of the actual bitmap data. */
		
		while (bPad > 0) {
			(void) getc(fp);
			bPad--;
		}
	}
	
	img->bitmap = (PIX *) malloc(sizeof(PIX) * img->size_xy);
	if(img->bitmap == 0){
		printf("could not allocate memory for %d x %d pixel image\n", img->size_xy);
		fclose(fp);
		return 0;
	}
	
	
	/* load up the image */
	if      (biBitCount == 1) 
		rv = BMPIMG_loadBMP1(img, fp,biWidth,biHeight);
	else if (biBitCount == 4) 
		rv = BMPIMG_loadBMP4(img, fp,biWidth,biHeight,biCompression);
	else if (biBitCount == 8)
		rv = BMPIMG_loadBMP8(img, fp,biWidth,biHeight,biCompression);
	else
		rv = BMPIMG_loadBMP24(img, fp,biWidth,biHeight);
	
	if (rv) 
		printf("File appears truncated.  Winging it.\n");

	if(biBitCount != 24){
		int i;
		PIX * bitmap = img->bitmap;
		PIX * colormap = img->colormap;
		for(i = 0; i < img->size_xy; i ++){
			int c = bitmap[i].I;
			bitmap[i].R = colormap[c].R;
			bitmap[i].G = colormap[c].G;
			bitmap[i].B = colormap[c].B;
		}
		for(i = 0; i < img->colormap_size; i ++){
			printf("colormap[%3d] = (R:%3d, G:%3d, B:%3d)\n",
				i, colormap[i].R, colormap[i].G, colormap[i].B);
		}
	}
	printf("BMP info: size_x = %d, size_y = %d\ncolormap_size = %d, bits_per_pixel = %d, biCompression = %d\n",
		img->size_x, img->size_y, img->colormap_size, biBitCount, biCompression);
	return 1;
}  

int BMPIMG_save(BMPIMG * img)
{
	FILE * fw;
	if(img->out_pix_bits != 1 && img->out_pix_bits != 4 &&
		img->out_pix_bits != 8 && img->out_pix_bits != 24){
		printf("out_pix_bits = %d is invalid (must be 1, 4, 8, 24)\n", img->out_pix_bits);
		return 0;
	}
	if(img->out_pix_bits == 24){
		if(img->colormap_size != 0){
			printf("img: colormap_size(%d) should be 0 when out_pix_bits = 24\n",
				img->colormap_size);
			return 0;
		}
	}
	if(img->out_pix_bits != 24 && img->colormap_size != (1 << img->out_pix_bits)){
		printf("img: colormap_size(%d) and out_pix_bits(%d) are inconsistent\n(must satisfy colormap_size == (1 << out_pix_bits)\n",
			img->colormap_size, img->out_pix_bits);
		return 0;
	}
	fw = fopen("result.bmp", "wb");
	{
		//int bfSize, bfOffBits, biSize;
		//int biWidth, biHeight, biPlanes, biBitCount, biCompression;
		//int biSizeImage, biXPelsPerMeter, biYPelsPerMeter, biClrUsed, biClrImportant;

		int bfSize, bfOffBits, biSize;
		int biWidth;
		int biSizeImage;

		int bitsPerLine = img->out_pix_bits * img->size_x;
		int bytesPerLine = (bitsPerLine + 7) / 8;
		bytesPerLine = ((bytesPerLine + 3) / 4) * 4;

		biSizeImage = bytesPerLine * img->size_y;

		biSize = WIN_NEW;				// 40 bytes for BMP_INFO_HEADER
		bfOffBits = biSize + 14 + img->colormap_size * 4;		// offset is biSize + 14 + colormap_size * 4
		bfSize = bfOffBits + biSizeImage;

		//////////// BMP header:
		fputc('B', fw); fputc('M', fw);	// "BM"
		putint(fw, bfSize);				// fileSize 
		putint(fw, 0);					// reserved and ignored

		putint(fw, bfOffBits);			// offset to bitmap data (from beginning of file)

		putint(fw, biSize);				// biSize = size of BMP_INFO_HEADER (biSize -- biClrImportant)

		biWidth = img->size_x;

		putint(fw, biWidth);			// biWidth
		putint(fw, img->size_y);			// biHeight
		putshort(fw, 1);			// biPlanes
		putshort(fw, img->out_pix_bits);		// biBitCount
		putint(fw, BBI_RGB);		// biCompression
		putint(fw, biSizeImage);		// biSizeImage
		putint(fw, 0);	// biXPelsPerMeter
		putint(fw, 0);	// biYPelsPerMeter
		putint(fw, 0);			// biClrUsed
		putint(fw, 0);		// biClrImportant
	}
	{
		int i;
		for(i = 0; i < img->colormap_size; i ++){
			putc(img->colormap[i].B, fw);
			putc(img->colormap[i].G, fw);
			putc(img->colormap[i].R, fw);
			putc(0, fw);
		}
	}
	if(img->out_pix_bits == 1){
		int i, j;
		int bytes_per_line = (img->size_x + 7) / 8 - img->size_x / 8;
		int pad_x = ((bytes_per_line + 3) / 4) * 4;
		for(i = img->size_y - 1; i >= 0; i --){
			PIX * pix = img->bitmap + i * img->size_x;
			int b = 0;
			for(j = 0; j < img->size_x; j ++){
				b = (b << 1) | (pix[j].I & 1);
				if((j & 7) == 7)
					putc(b, fw);
			}
			for(j = 0; j < pad_x; j ++){
				b = (b << 1);
				if((j & 7) == 7)
					putc(b, fw);
			}
		}
	}
	else if(img->out_pix_bits == 4){
		int i, j;
		int bytes_per_line = (img->size_x + 1) / 2;
		int pad_x = ((bytes_per_line + 3) / 4) * 4 - img->size_x / 2;
		for(i = img->size_y - 1; i >= 0; i --){
			PIX * pix = img->bitmap + i * img->size_x;
			int b = 0;
			for(j = 0; j < img->size_x; j ++){
				b = (b << 4) | (pix[j].I & 0xf);
				if((j & 1) == 1)
					putc(b, fw);
			}
			for(j = 0; j < pad_x; j ++){
				b = (b << 4);
				if((j & 1) == 1)
					putc(b, fw);
			}
		}
	}
	else if(img->out_pix_bits == 8){
		int i, j;
		int pad_x = ((img->size_x + 3) / 4) * 4 - img->size_x;
		for(i = img->size_y - 1; i >= 0; i --){
			PIX * pix = img->bitmap + i * img->size_x;
			for(j = 0; j < img->size_x; j ++)
				putc(pix[j].I, fw);
			for(j = 0; j < pad_x; j ++)
				putc(0, fw);
		}
	}
	else if(img->out_pix_bits == 24){
		int i, j;
		int pad_x = ((img->size_x * 3 + 3) / 4) * 4 - img->size_x * 3;
		for(i = img->size_y - 1; i >= 0; i --){
			PIX * pix = img->bitmap + i * img->size_x;
			for(j = 0; j < img->size_x; j ++){
				putc(pix[j].B, fw);
				putc(pix[j].G, fw);
				putc(pix[j].R, fw);
			}
			for(j = 0; j < pad_x; j ++)
				putc(0, fw);
		}
	}
	fclose(fw);
	return 1;
}

int BMPIMG_open(BMPIMG * img, const char * fname)
{
	FILE * fp;
	if(!check_bmp_file_extension(fname)){
		printf("file <%s> does not have .bmp file extension\n", fname);
		return 0;
	}
	fp = fopen(fname, "rb");
	if(fp == 0){
		printf("cannot open file <%s>\n", fname);
		return 0;
	}
	return BMPIMG_openBMP(img, fp);
}


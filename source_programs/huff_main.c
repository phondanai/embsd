//	set path=C:\MinGW\bin;%path%

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huff.h"
#include "bmp.h"


void abort_program(const char * msg)
{
	printf("abort: (%s) is FALSE!\n", msg);
	exit(-1);
}

#include <math.h>
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
	c = getc(fp);  c1 = getc(fp);
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

double log2(double n)
{
	return log(n) / log(2);
}

void swap3(void ** a0, void ** a1)
{
	void * t = *a0;
	*a0 = *a1;
	*a1 = t;
}

int partition(void * a[], int left, int right, int (* is_ordered)(void *, void *))
{
	void * p_pivot = a[right];
	int i = left, k = right;
	while(i < k){
		while(i < k && is_ordered(a[i], p_pivot)) i ++;
		while(i < k && !is_ordered(a[k], p_pivot)) k --;
		if(i < k) swap3(&a[i], &a[k]);
	}
	if(right > k) swap3(&a[right], &a[k]);
	return k;
}

void quick_sort(void * a[], int left, int right, int (* is_ordered)(void *, void *))
{
	if(left < right){
		int pivot_pos = partition(a, left, right, is_ordered);
		quick_sort(a, left, pivot_pos - 1, is_ordered);
		quick_sort(a, pivot_pos + 1, right, is_ordered);
	}
}

void SINFO_initialize(SINFO * sinfo, int i)
{
	sinfo->index = i;
	sinfo->count = 0;
	sinfo->left = 0;
	sinfo->right = 0;
	sinfo->parent = 0;
	sinfo->code = 0;
	sinfo->code_length = 0;
	sinfo->pos = 0;
}

void HINFO_initialize_p_sinfo(HINFO * hinfo)
{
	int i;
	for(i = 0; i < 256; i ++){
		hinfo->p_sinfo[i] = &hinfo->sinfo[i];
	}
}

void HINFO_initialize(HINFO * hinfo)
{
	int i;
	for(i = 0; i < 256; i ++){
		SINFO_initialize(&hinfo->sinfo[i], i);
		SINFO_initialize(&hinfo->inode[i], i);
	}
	hinfo->root_node = 0;
	hinfo->vertical_pitch = 1;
}

int SINFO_is_ordered(void * a, void * b)
{
	return ((SINFO *) a)->count > ((SINFO *) b)->count;
}

char * convert_char(int c)
{
	static char __str[3] = {0, 0, 0};
#define SET_STRING(c0, c1) __str[0] = c0; __str[1] = c1;
	switch(c){
		case '\a': SET_STRING('\\', 'a') break;
		case '\b': SET_STRING('\\', 'b') break;
		case '\f': SET_STRING('\\', 'f') break;
		case '\n': SET_STRING('\\', 'n') break;
		case '\r': SET_STRING('\\', 'r') break;
		case '\t': SET_STRING('\\', 't') break;
		case '\v': SET_STRING('\\', 'v') break;
		default: SET_STRING(c, 0); break;
	}
	return __str;
}

void HINFO_count_symbols(HINFO * hinfo)
{
	hinfo->text_count = 0;
	while(1){
		int c = fgetc(hinfo->fp_txt);
		if(c == EOF)
			break;
		ASSERT(c >= 0 && c < 256);
		hinfo->sinfo[c].count ++;
		hinfo->text_count ++;
	}
	printf("text_count = %d\n", hinfo->text_count);
}

void SINFO_print_huffman_code(SINFO * sinfo)
{
	int i, mask;
	printf("(%2d bits: ", sinfo->code_length);
	for(i = 0, mask = 1; i < sinfo->code_length; i ++, mask <<= 1)
		printf("%c", ((sinfo->code) & mask) ? '1' : '0');
	printf(" )");
}

void SINFO_create_sinfo_node(SINFO * child0, SINFO * child1, SINFO * parent)
{
	child0->parent = parent;
	child1->parent = parent;
	parent->left = child0;
	parent->right = child1;
	parent->count = child0->count + child1->count;
}

void SINFO_assign_code(SINFO * sinfo)
{
	if(sinfo->parent){
		sinfo->code = (SIB_DIR(sinfo, right) << sinfo->parent->code_length) + (sinfo->parent->code);
		sinfo->code_length = sinfo->parent->code_length + 1;
	}
	if(sinfo->left) SINFO_assign_code(sinfo->left);
	if(sinfo->right) SINFO_assign_code(sinfo->right);
}

void HINFO_sort_symbols(HINFO * hinfo)
{
	int i;
	HINFO_initialize_p_sinfo(hinfo);
	quick_sort((void **) hinfo->p_sinfo, 0, 255, SINFO_is_ordered);
	for(i = 255; i >= 0; i --){
		if(hinfo->p_sinfo[i]->count)
			break;
	}
	hinfo->symbol_count = i + 1;
}

void HINFO_create_huffman_tree(HINFO * hinfo)
{
	int tail, cindex = 0;
	HINFO_sort_symbols(hinfo);
	tail = hinfo->symbol_count - 1;
	while(tail > 0){
		SINFO * child0, * child1, * parent;
		child1 = hinfo->p_sinfo[tail];
		child0 = hinfo->p_sinfo[tail - 1];
		parent = &hinfo->inode[cindex ++];
		SINFO_create_sinfo_node(child0, child1, parent);
		hinfo->p_sinfo[tail - 1] = parent;
		quick_sort((void **) hinfo->p_sinfo, 0, tail - 1, SINFO_is_ordered);
		tail --;
	}
	ASSERT(cindex == hinfo->symbol_count - 1);
	hinfo->root_node = hinfo->p_sinfo[0];
	SINFO_assign_code(hinfo->root_node);
}
void HINFO_print_sorted_symbols(HINFO * hinfo)
{
	int i, total_bits = 0, total_bytes;
	HINFO_sort_symbols(hinfo);
	for(i = 0; i < hinfo->symbol_count; i ++){
		SINFO * sinfo = hinfo->p_sinfo[i];
		printf("%3d<%2s> : %5d ",
		sinfo->index, convert_char(sinfo->index), sinfo->count);
		SINFO_print_huffman_code(sinfo);
		printf("\n");
		total_bits += sinfo->code_length * sinfo->count;
	}
	total_bytes = (total_bits + 7) / 8;
	printf("total bits = %d\ntotal bytes = %d\n",
		total_bits, total_bytes);
	printf("compression rate = %f\n", (double) total_bytes / (double) hinfo->text_count);
}
typedef struct bit_buffer
{
	FILE * fp;
	unsigned int word;
	int bit_pos;
} BITBUF;

void BITBUF_initialize(BITBUF * bitbuf, FILE * fp)
{
	bitbuf->fp = fp;
	bitbuf->word = 0;
	bitbuf->bit_pos = 0;
}

#define ASSERT_BITS(word, bits)	ASSERT((word & ~((1 << bits) - 1)) == 0)

void BITBUF_write_bits(BITBUF * bitbuf, unsigned int val, int bits)
{
	ASSERT(bitbuf->bit_pos >= 0 && bitbuf->bit_pos < 8);
	ASSERT_BITS(bitbuf->word, bitbuf->bit_pos);
	ASSERT(bits <= 24);
	ASSERT_BITS(val, bits);
	bitbuf->word |= (val << bitbuf->bit_pos);
	bitbuf->bit_pos += bits;
	while(bitbuf->bit_pos >= 8){
		unsigned char c = bitbuf->word;
		fputc(c, bitbuf->fp);
		bitbuf->word >>= 8;
		bitbuf->bit_pos -= 8;
	}
}

void BITBUF_flush_bits(BITBUF * bitbuf)
{
	unsigned char c = bitbuf->word;
	ASSERT(bitbuf->bit_pos >= 0 && bitbuf->bit_pos < 8);
	ASSERT_BITS(bitbuf->word, bitbuf->bit_pos);
	if(bitbuf->bit_pos > 0){
		fputc(c, bitbuf->fp);
		bitbuf->word = 0;
		bitbuf->bit_pos = 0;
	}
}

unsigned int BITBUF_read_bits(BITBUF * bitbuf, int bits)
{
	unsigned int data;
	unsigned int c;
	ASSERT(bitbuf->bit_pos >= 0 && bitbuf->bit_pos < 8);
	ASSERT_BITS(bitbuf->word, bitbuf->bit_pos);
	ASSERT(bits <= 24);
	while(bitbuf->bit_pos < bits){
		if((c = fgetc(bitbuf->fp)) == EOF){
			printf("End-Of-File Reached!!\n bit_pos = %d, bits = %d\n", bitbuf->bit_pos, bits);
			exit(-1);
		}
		bitbuf->word |= (c << bitbuf->bit_pos);
		bitbuf->bit_pos += 8;
	}
	data = bitbuf->word & ((1 << bits) - 1);
	bitbuf->bit_pos -= bits;
	bitbuf->word >>= bits;
	return data;
}

void SINFO_write_children(SINFO * sinfo, BITBUF * bitbuf)
{
	if(sinfo->left == 0 || sinfo->right == 0) return;
	BITBUF_write_bits(bitbuf, sinfo->left->left == 0, 1);
	BITBUF_write_bits(bitbuf, sinfo->left->index, 8);
	BITBUF_write_bits(bitbuf, sinfo->right->left == 0, 1);
	BITBUF_write_bits(bitbuf, sinfo->right->index, 8);
}

void HINFO_write_huffman_tree(HINFO * hinfo, BITBUF * bitbuf)
{
	int i;
	fwrite(&hinfo->text_count, 4, 1, bitbuf->fp);	/* symbol count */
	fputc(hinfo->symbol_count - 1, bitbuf->fp);
	for(i = 0; i < hinfo->symbol_count - 1; i ++){
		SINFO_write_children(&hinfo->inode[i], bitbuf);
	}
}

#define LINK_SINFO(ch_node, p_node, child)	\
	ASSERT(p_node->child == 0);				\
	p_node->child = ch_node;				\
	ch_node->parent = p_node

void HINFO_read_huffman_tree(HINFO * hinfo, BITBUF * bitbuf)
{
	int i;
	SINFO * sinfo, * child;
	fread(&hinfo->text_count, 4, 1, bitbuf->fp);	/* symbol count */
	hinfo->symbol_count = fgetc(bitbuf->fp) + 1;
	printf("hinfo->text_count = %d, hinfo->scount = %d\n", hinfo->text_count, hinfo->symbol_count);
	for(i = 0; i < hinfo->symbol_count - 1; i ++){
		sinfo = &hinfo->inode[i];
		child = (BITBUF_read_bits(bitbuf, 1)) ? hinfo->sinfo : hinfo->inode;
		child += BITBUF_read_bits(bitbuf, 8);
		LINK_SINFO(child, sinfo, left);
		child = (BITBUF_read_bits(bitbuf, 1)) ? hinfo->sinfo : hinfo->inode;
		child += BITBUF_read_bits(bitbuf, 8);
		LINK_SINFO(child, sinfo, right);
	}
	hinfo->root_node = &hinfo->inode[hinfo->symbol_count - 2];
	ASSERT(hinfo->root_node->left && hinfo->root_node->right);
	SINFO_assign_code(hinfo->root_node);
}

int HINFO_decode_symbol(HINFO * hinfo, BITBUF * bitbuf)
{
	SINFO * sinfo = hinfo->root_node;
	ASSERT(sinfo && sinfo->left && sinfo->right);
	while(1){
		int bit = BITBUF_read_bits(bitbuf, 1);
		sinfo = (bit) ? sinfo->right : sinfo->left;
		if(sinfo->right == 0)
			break;
	}
	ASSERT(sinfo->left == 0 && sinfo->right == 0);
	return sinfo->index;
}

FILE * open_processed_file(const char * fname, const char * fmode, const char * fext)
{
	char comp_filename[MAX_FILENAME_LENGTH + 1 + 4];
	FILE * fp_comp;
	ASSERT(strlen(fname) <= MAX_FILENAME_LENGTH);
	sprintf(comp_filename, "%s%s", fname, fext);
	fp_comp = fopen(comp_filename, fmode);
	if(fp_comp == 0){
		printf("cannot open <%s> with mode(%s)\n", comp_filename, fmode);
	}
	return fp_comp;
}

void HINFO_write_compressed_file(HINFO * hinfo, const char * fname)
{
	int ccount = 0;
	SINFO * sinfo;
	BITBUF bitbuf;
	FILE * fp_comp = open_processed_file(fname, "wb", ".hmc");
	if(fp_comp == 0)
		return;
	BITBUF_initialize(&bitbuf, fp_comp);
	HINFO_write_huffman_tree(hinfo, &bitbuf);
	rewind(hinfo->fp_txt);
	while(1){
		int c = fgetc(hinfo->fp_txt);
		if(c == EOF)
			break;
		ASSERT(c >= 0 && c < 256);
		sinfo = &hinfo->sinfo[c];
		ASSERT(sinfo->code_length > 0);
		BITBUF_write_bits(&bitbuf, sinfo->code, sinfo->code_length);
		ccount ++;
	}
	BITBUF_flush_bits(&bitbuf);
	ASSERT(hinfo->text_count == ccount);
	fclose(fp_comp);
}

void HINFO_check_compressed_file(HINFO * hinfo, const char * fname)
{
	int ccount = hinfo->text_count;
	BITBUF bitbuf;
	FILE * fp_comp = open_processed_file(fname, "rb", ".hmc");
	if(fp_comp == 0)
		return;
	BITBUF_initialize(&bitbuf, fp_comp);
	while(ccount > 0){
		int c = HINFO_decode_symbol(hinfo, &bitbuf);
		printf("%c", c);
		ccount --;
	}
	fclose(fp_comp);
}

void HINFO_read_compressed_file(HINFO * hinfo, const char * fname)
{
	FILE * fp_comp, * fp_out;
	int ccount = hinfo->text_count;
	BITBUF bitbuf;
	HINFO_initialize(hinfo);
	fp_comp = open_processed_file(fname, "rb", ".hmc");
	BITBUF_initialize(&bitbuf, fp_comp);
	HINFO_read_huffman_tree(hinfo, &bitbuf);
	ccount = hinfo->text_count;
	fp_out = open_processed_file(fname, "wb", ".txt");
	while((ccount --) > 0){
		int c = HINFO_decode_symbol(hinfo, &bitbuf);
		if(c == EOF)
			break;
		fputc(c, fp_out);
	}
	fclose(fp_comp);
	fclose(fp_out);
	printf("text_count = %d\n", hinfo->text_count);
}

void BMPIMG_create_colormap(BMPIMG * img, int RQ, int GQ, int BQ)

{

    int i, j, k;

    if(RQ * GQ * BQ > 256 || RQ <= 0 || GQ <= 0 || BQ <= 0){
        printf("Q-levels out of range!!! (%d * %d * %d = %d)\n",
               RQ, GQ, BQ, RQ * GQ * BQ);
        return;

    }

    for(i = 0; i < RQ; i ++){
        int r_min = (i * 256 + RQ - 1) / RQ;
        int r_max = ((i + 1) * 256 + RQ - 1) / RQ - 1;

        for(j = 0; j < GQ; j ++){

            int g_min = (j * 256 + GQ - 1) / GQ;
            int g_max = ((j + 1) * 256 + GQ - 1) / GQ - 1;
            for(k = 0; k < BQ; k ++){

                int b_min = (k * 256 + BQ - 1) / BQ;
                int b_max = ((k + 1) * 256 + BQ - 1) / BQ - 1;
                int cid = i * GQ * BQ + j * BQ + k;

                img->colormap[cid].R = (r_min + r_max) / 2;
                img->colormap[cid].G = (g_min + g_max) / 2;
                img->colormap[cid].B = (b_min + b_max) / 2;

                printf("%3d: R(%3d,%3d) G(%3d,%3d) B(%3d,%3d)\n", cid,

                       r_min, r_max, g_min, g_max, b_min, b_max);

            }
        }
    }
}


void BMPIMG_convert_to_colormap(BMPIMG * img, int RQ, int GQ, int BQ)

{
    int i;
    PIX * pix;

    for(i = 0, pix = img->bitmap; i < img->size_xy; i ++, pix ++){
        pix->I = (pix->R * RQ / 256) * GQ * BQ +
        (pix->G * GQ / 256) * BQ +
        (pix->B * BQ / 256);
    }

    img->out_pix_bits = 8;
    img->colormap_size = 256;

}

int main(int argc, char * argv[])
{

    FILE * fp;
    HINFO hinfo, hinfo2;

    HINFO_initialize(&hinfo);

    BMPIMG img;
    BMPIMG_initialize(&img);

    if(!BMPIMG_open(&img, argv[1])){
        printf("open file <%s> failed\n", argv[1]);
        return 0;

    }
    BMPIMG_create_colormap(&img, 6, 6, 6);
    BMPIMG_convert_to_colormap(&img, 6, 6, 6);
    BMPIMG_save(&img);
    BMPIMG_destroy(&img);
    fp = fopen("result.bmp", "rb");
    hinfo.fp_txt = fp;
    HINFO_count_symbols(&hinfo);
    HINFO_create_huffman_tree(&hinfo);
    HINFO_print_sorted_symbols(&hinfo);
    HINFO_write_compressed_file(&hinfo, argv[1]);
    HINFO_read_compressed_file(&hinfo2, argv[1]);
    fclose(fp);

    return 1;
}

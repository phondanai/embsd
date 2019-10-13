//	set path=C:\MinGW\bin;%path%

#define BBI_RGB  0
#define BBI_RLE8 1
#define BBI_RLE4 2
#define BBI_BITFIELD 3

#define WIN_OS2_OLD 12
#define WIN_NEW     40
#define OS2_NEW     64

typedef struct pixel
{
	unsigned char R, G, B, I;
} PIX;

typedef struct bmp_info
{
	PIX * bitmap;
	PIX colormap[256];
	int colormap_size;
	int size_x, size_y, size_xy;
	int out_pix_bits;	/* valid values are: 24, 8, 4, 1 */
} BMPIMG;

void BMPIMG_initialize(BMPIMG * img);
void BMPIMG_destroy(BMPIMG * img);
int BMPIMG_save(BMPIMG * img);
int BMPIMG_open(BMPIMG * img, const char * fname);



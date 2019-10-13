#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

#define max_level 255


void BMPIMG_greyscale1(BMPIMG * img)
{
	int i;
	PIX * pix;
	/* set greyscale colormap */
	for(i = 0; i < 256; i ++){
		img->colormap[i].B = i;
		img->colormap[i].G = i;
		img->colormap[i].R = i;
	}
	img->out_pix_bits = 8;
	img->colormap_size = 256;
	pix = img->bitmap;
	for(i = 0; i < img->size_xy; i ++, pix ++){
#if 1	/* definition of Y (luminance) */
		pix->I = (unsigned char) (pix->R * 0.299 + pix->G * 0.587 + pix->B * 0.114);
#else	/* simple average of RGB */
		pix->I = (pix->R + pix->G + pix->B) / 3;
#endif
	}
}

void BMPIMG_negative(BMPIMG * img)
{
	int i;
	PIX * pix;
	/* set greyscale colormap */
	for(i = 0; i < 256; i ++){
		img->colormap[i].B = i;
		img->colormap[i].G = i;
		img->colormap[i].R = i;
	}
	img->out_pix_bits = 8;
	img->colormap_size = 256;
	pix = img->bitmap;
	for(i = 0; i < img->size_xy; i ++, pix ++){
#if 1	
		pix->I = ((max_level - pix->R) + (max_level - pix->G) + (max_level - pix->B)) / 3;
#else	/* simple average of RGB */
		pix->I = (pix->R + pix->G + pix->B) / 3;
#endif
	}
}

int main(int argc, char * argv[])
{
    BMPIMG img;
    BMPIMG_initialize(&img);

    if(!BMPIMG_open(&img, argv[1]))
    {
        printf("open file <%s> failed\n", argv[1]);
        return 0;
    }

    BMPIMG_negative(&img);

    BMPIMG_save(&img);
    BMPIMG_destroy(&img);
    return 1;
}

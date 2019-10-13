#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

#define max_level 255

int avg_r, avg_g, avg_b;

void BMPIMG_convert_to_colormap(BMPIMG * img, int RQ, int GQ, int BQ)
{
	int i;
	PIX * pix;
	for(i = 0, pix = img->bitmap; i < img->size_xy; i ++, pix ++){
		pix->I = (pix->R * RQ / 256) * GQ * BQ +
			(pix->G * GQ / 256) * BQ +
			(pix->B * BQ / 256);
        printf("i[%d]: %d \n", i, pix->I);
	}
	img->out_pix_bits = 8;
	img->colormap_size = 256;

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
                printf("%d %d %d\n", avg_r, avg_g, avg_b);
				img->colormap[cid].R = avg_r;//(r_min + r_max) / 2;
				img->colormap[cid].G = avg_g;//(g_min + g_max) / 2;
				img->colormap[cid].B = avg_b;//(b_min + b_max) / 2;
				//img->colormap[cid].R = (r_min + r_max) / 2;
				//img->colormap[cid].G = (g_min + g_max) / 2;
				//img->colormap[cid].B = (b_min + b_max) / 2;
				//printf("%3d: R(%3d,%3d) G(%3d,%3d) B(%3d,%3d)\n", cid,
				//	r_min, r_max, g_min, g_max, b_min, b_max); 
			}
		}
	}
    
	//for(i = 0; i < 256; i ++)
	//	printf("i = %3d : %d\n", i, i * BQ / 256);
}

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

void BMPIMG_bitmap(BMPIMG * img)
{
	int i,j;
	PIX * pix;
	img->out_pix_bits = 8;
	img->colormap_size = 256;
	pix = img->bitmap;
    unsigned char tempR, tempG, tempB;
    int size = img->size_xy;
    int sum_r=0, sum_g=0, sum_b=0;

    for(int i =0; i < img->colormap_size; i++){
        printf("colormap[%d]: R: %d G: %d B:%d\n", i, img->colormap[i].R, img->colormap[i].G, img->colormap[i].B);
    }

    for(i = 0; i < img->size_xy; i ++)
    {
        //printf("\n[%d]: %d", i, pix[i].R);
        //printf("\n[%d]: %d", i, pix[i].R);
        //printf("\n[%d]: %d", i, pix[i].R);
        sum_r = sum_r + pix[i].R;
        sum_g = sum_g + pix[i].G;
        sum_b = sum_b + pix[i].B;
    }

    avg_r = sum_r / img->size_xy;
    avg_g = sum_g / img->size_xy;
    avg_b = sum_b / img->size_xy;

    //printf("%3d %3d %3d\n", avg_r, avg_g, avg_b);
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

//    BMPIMG_create_colormap(&img,8,8,4);
	BMPIMG_bitmap(&img);
	BMPIMG_convert_to_colormap(&img, 8, 8, 4);
    //BMPIMG_create_colormap(&img, 8,8,4);

    BMPIMG_save(&img);
    BMPIMG_destroy(&img);
    return 1;
}

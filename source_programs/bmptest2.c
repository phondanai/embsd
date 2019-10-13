//	set path=C:\MinGW\bin;%path%

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"

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

//#define RQ	6
//#define GQ	7
//#define BQ	6

//#define RQ	8
//#define GQ	8
//#define BQ	4

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
	//for(i = 0; i < 256; i ++)
	//	printf("i = %3d : %d\n", i, i * BQ / 256);
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

#if 1
#define CLAMP(pp) ((pp > 255) ? 255 : (pp < 0) ? 0 : pp)
#define DIF_ERR(pos, coef)		\
			{ int pp;	\
			pp = (pix + pos)->R + eR * coef / 16;	\
			(pix + pos)->R = CLAMP(pp);				\
			pp = (pix + pos)->G + eG * coef / 16;	\
			(pix + pos)->G = CLAMP(pp);				\
			pp = (pix + pos)->B + eB * coef / 16;	\
			(pix + pos)->B = CLAMP(pp);	}
#else
#define DIF_ERR(pos, coef)		\
			{(pix + pos)->R += eR * coef / 16;	\
			(pix + pos)->G += eG * coef / 16;	\
			(pix + pos)->B += eB * coef / 16;}	
#endif

void BMPIMG_dither(BMPIMG * img, int RQ, int GQ, int BQ)
{
	int i, j, eR, eG, eB;
	PIX * pix;
	for(i = 0, pix = img->bitmap; i < img->size_y; i ++){
		for(j = 0; j < img->size_x; j ++, pix ++){
			pix->I = (pix->R * RQ / 256) * GQ * BQ +
				(pix->G * GQ / 256) * BQ +
				(pix->B * BQ / 256);
			eR = pix->R - img->colormap[pix->I].R;
			eG = pix->G - img->colormap[pix->I].G;
			eB = pix->B - img->colormap[pix->I].B;

			if(j < img->size_x - 1)
				DIF_ERR(1, 7);
			if(i < img->size_y - 1){
				if(j > 0)
					DIF_ERR(img->size_x - 1, 3);
				DIF_ERR(img->size_x, 5);
				if(j < img->size_x - 1)
					DIF_ERR(img->size_x + 1, 1);
			}
		}
	}
	img->out_pix_bits = 8;
	img->colormap_size = 256;
}

void BMPIMG_average(BMPIMG * img, int fx, int fy)
{
	int i, j, k, l;
	int sx = (fx + 1) / 2, sy = (fy + 1) / 2;
	int area = fx * fy;
	PIX * pix, * out_pix;
	PIX * out_buf = (PIX *) malloc(img->size_xy * sizeof(PIX));
	if(out_buf == 0){
		printf("malloc failed in BMPIMG_lowpass!\n");
		return;
	}
	pix = img->bitmap;
	out_pix = out_buf;
	for(i = 0; i < img->size_y; i ++){
		for(j = 0; j < img->size_x; j ++, pix ++, out_pix ++){
			PIX * p2 = pix + (sy - fy) * img->size_x;
			int r = 0, g = 0, b = 0;
			for(k = sy - fy; k < sy; k ++, p2 += img->size_x){
				PIX * p3 = p2 + (sx - fx);
				if(i + k < 0 || i + k >= img->size_y)
					continue;
				for(l = sx - fx; l < sx; l ++, p3 ++){
					if(j + l < 0 || j + l >= img->size_x)
						continue;
					r += p3->R;
					g += p3->G;
					b += p3->B;
				}
			}
			out_pix->R = r / area;
			out_pix->G = g / area;
			out_pix->B = b / area;
		}
	}
	pix = img->bitmap;
	img->bitmap = out_buf;
	free(pix);
}

void BMPIMG_binarize(BMPIMG * img, int fx, int fy)
{
	int i, j, k, l;
	int sx = (fx + 1) / 2, sy = (fy + 1) / 2;
	int area = fx * fy;
	PIX * pix, * out_pix;
	PIX * out_buf = (PIX *) malloc(img->size_xy * sizeof(PIX));
	if(out_buf == 0){
		printf("malloc failed in BMPIMG_lowpass!\n");
		return;
	}
	BMPIMG_greyscale1(img);
	pix = img->bitmap;
	out_pix = out_buf;
	for(i = 0; i < img->size_y; i ++){
		for(j = 0; j < img->size_x; j ++, pix ++, out_pix ++){
			PIX * p2 = pix + (sy - fy) * img->size_x;
			int sum = 0;
			for(k = sy - fy; k < sy; k ++, p2 += img->size_x){
				PIX * p3 = p2 + (sx - fx);
				if(i + k < 0 || i + k >= img->size_y)
					continue;
				for(l = sx - fx; l < sx; l ++, p3 ++){
					if(j + l < 0 || j + l >= img->size_x)
						continue;
					sum += p3->I;
				}
			}
			out_pix->I = (pix->I >= sum / area) ? 255 : 0;
		}
	}
	pix = img->bitmap;
	img->bitmap = out_buf;
#if 0
	img->out_pix_bits = 1;
	img->colormap_size = 2;
	img->colormap[0].R = 0;
	img->colormap[0].G = 0;
	img->colormap[0].B = 0;
	img->colormap[1].R = 255;
	img->colormap[1].G = 255;
	img->colormap[1].B = 255;
#endif
	free(pix);
}

int main(int argc, char * argv[])
{
	BMPIMG img;
	int fx = 8, fy = 8, fz = 4;
	BMPIMG_initialize(&img);
	if(argc < 2 || argc > 5){
		printf("invalid command!\n");
		printf("usage: %s <filename>\n", argv[0]);
		return 0;
	}
	if(!BMPIMG_open(&img, argv[1])){
		printf("open file <%s> failed\n", argv[1]);
		return 0;
	}
	if(argc >= 3)
		fx = atoi(argv[2]);
	if(argc >= 4)
		fy = atoi(argv[3]);
	if(argc >= 5)
		fz = atoi(argv[4]);
#if 0
	//BMPIMG_greyscale1(&img);
	//BMPIMG_average(&img, fx, fy);
	//BMPIMG_binarize(&img, fx, fy);
#else
	//BMPIMG_greyscale1(&img);
	//BMPIMG_create_colormap(&img, fx, fy, fz);
	//BMPIMG_convert_to_colormap(&img, fx, fy, fz);
	//BMPIMG_dither(&img, fx, fy, fz);
#endif
	BMPIMG_binarize(&img, fx, fy);
	BMPIMG_save(&img);
	BMPIMG_destroy(&img);
	return 1;
}

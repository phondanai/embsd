//
//  mycomdecom.c
//  ESDT4
//
//  Created by CHATCHAWAL SANGKEETTRAKARN on 11/14/17.
//  Copyright © 2017 CHATCHAWAL SANGKEETTRAKARN. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"
#include "huff.h"


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
    /** do some image processing here **/
    
    BMPIMG_create_colormap(&img, 6, 6, 6);
    BMPIMG_convert_to_colormap(&img, 6, 6, 6);
    BMPIMG_save(&img);
    
    /** writes to result.bmp **/
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


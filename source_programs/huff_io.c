//	set path=C:\MinGW\bin;%path%

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huff.h"


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

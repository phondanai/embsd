//	set path=C:\MinGW\bin;%path%

void abort_program(const char * msg);

double log2(double n);
#define ENABLE_ASSERT

#ifdef ENABLE_ASSERT
#define ASSERT(n)  if(!(n))  abort_program(#n)
#else
#define ASSERT(n)  (n)
#endif

#define MAX_FILENAME_LENGTH 100

#define COMPRESSED_FILE_EXTENSION ".hmc"
#define DECOMPRESSED_FILE_EXTENSION ".txt"

typedef struct sym_info
{
	int count;
	struct sym_info * left, * right, * parent;
	unsigned char index, code_length;
	short pos;
	unsigned int code;
} SINFO;

typedef struct huffman_info
{
	FILE * fp_txt;
	SINFO sinfo[256], inode[256], * p_sinfo[256], * root_node;
	int text_count, symbol_count;
	int vertical_pitch;
} HINFO;

#define SIB_DIR(sinfo, child) (sinfo->parent && sinfo->parent->child == sinfo)

char * convert_char(int c);
void SINFO_print_huffman_code(SINFO * sinfo);

void HINFO_initialize(HINFO * hinfo);
void SINFO_assign_code(SINFO * sinfo);
void HINFO_sort_symbols(HINFO * hinfo);

void HINFO_write_compressed_file(HINFO * hinfo, const char * fname);
void HINFO_check_compressed_file(HINFO * hinfo, const char * fname);
void HINFO_read_compressed_file(HINFO * hinfo, const char * fname);

void HINFO_print_entropy(HINFO * hinfo);
void HINFO_print_tree_structure(HINFO * hinfo);

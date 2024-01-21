/*
 * UNEXECOMP -- An upacker for EXE files packed with "COMPRESSOR v1.01 by
 * 	Knowledge Dynamics Corp."
 *
 * Public Domain or CC0, whichever suits you better.
 * See http://creativecommons.org/publicdomain/zero/1.0/
 *
 */

/*
 * This program only works on 32-bit LSB machines.
 *
 * TODO: Fix READ_WORD and WRITE_WORD to make it portable.
 *
 * This is a C99 program, it uses "char something[variable]" construct. Convert
 * those to malloc calls to make it C89.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef unsigned short word;
typedef unsigned int word24;

struct EXE {
  unsigned short signature; /* == 0x5a4D, "MZ" */
  unsigned short bytes_in_last_block;
  unsigned short blocks_in_file;
  unsigned short num_relocs;
  unsigned short header_paragraphs;
  unsigned short min_extra_paragraphs;
  unsigned short max_extra_paragraphs;
  unsigned short ss;
  unsigned short sp;
  unsigned short checksum;
  unsigned short ip;
  unsigned short cs;
  unsigned short reloc_table_offset;
  unsigned short overlay_number;
};
/*
	A compressed file looks like this :

offset     length    purpose
0          0x200     DOS exe header, with wrong block count, see "struct EXE" above
0x200      0x3       jump instruction to 0x29C
0x203      0x99      unpacker data
0x29C	   0x3EE	 unpacker code
0x68a				 <garbage, will be used for variable in unpacker code>
0x699	   0x25		 packed header
0x6C0				 special buffer, unknown purpose
<offset>			 packed exe

	NOTE: offsets might be different in your file :(

offset to packed header = main_header.blocks * 512 + main_header.extra_bytes
offset to packed exe = offset_to_packed_header + packed_header.headpars * 16
length of packed exe = filesize - offset_to_packed_exe
length of unpacked exe = packed_header.blocks * 512 + packed_header.extra_bytes

As mentioned, block count in "main_header" (header read from offset 0) is WRONG, but it contains
the offset required to read compressed data.

Uncompressed data should be written into 0x20, right after the new header.
New header is created by taking "packed header" and updating few values in it.
*/
const int mzSize = 0x1C; /* size of "struct EXE" */

#define READ_WORD(str, index) str[index+1]*0x100 + str[index];
#define WRITE_WORD(str, index, val) { unsigned short *w = (unsigned short *) &str[index]; *w = val; }

#define MBUFFER_SIZE	1024
#define MBUFFER_EDGE (MBUFFER_SIZE - 3)

//#define DEBUG

/* Uncompression algorithm -- good old unLZW. */
int load_infile(FILE *f, char *buffer, int expect_len) {

	int i, n, j;

	unsigned long long res_pos = 0;

	char result[expect_len];

	printf("Expected result: %d (%04x)\n", expect_len, expect_len);

	/*
	 * The data is kept in BIT-positioned "blocks".
	 * We use several variables to iterate it with some
	 * level of sanity:
	 */
	int pos = 0;	/* Position in bits */

	int byte_pos = 0; /* Position in bytes */
	int bit_pos = 0; /* Extra shift in bits */

	/*
	 * Each "block" can take from 9 to 12 bits of data.
	 * Last 8 bits are "the value" 
	 * First N bits are "the key"
	 */
	int step = 9;	/* Bits per step */

	/* Those masks help us get the relevant bits */ 
	word keyMask[4] = {
		0x01FF, 	// 0001 1111 
		0x03FF,		// 0011 1111
		0x07FF,		// 0111 1111
		0x0FFF,		// 1111 1111
	};

	/*
	 * If the "the key" is unset (00), the value is read as-is.
	 * Otherwise, the whole "block" (N+8 bytes) is treated
	 * as a dictionary key, and the dictionary is queried.
	 * The exceptions are the 0x0100 value, which means
	 * the dictionary must be cleared, and the 0x0101 value,
	 * which marks END OF FILE.
	 */

	/*
	 * The dictionary.
	 * Each entry consists of an index to a previous entry
	 * and a value, forming a tree.
	 */
	word dict_key[768 * 16];
	char dict_val[768 * 16];
	word dict_index = 0x0102; /* Start populating dictionary from 0x0102 */
	word dict_range = 0x0200; /* Allow that much entries before increasing step */

	/* Since data stored this way is backwards, we need a small queue */
	char queue[0xFF];
	int queued = 0;

	/* Read buffer */ 
	char mbuffer[MBUFFER_SIZE];

	/* Data */
	word next_index = 0;	/* block of data we currently examine */

	char last_char  = 0;	/* value from previous iteration */
	word last_index = 0;	/* block from previous iteration */

	word24 big_index;	/* temp. variable to safely load and shift 3 bytes */
	word keep_index;	/* temp. variable to keep "next_index" before making it "last_index" */
	int reset_hack=0;	/* HACK -- delay dictionary reset to next iteration */ 

	/* Read first chunk of data */
	n = fread(mbuffer, sizeof(char), MBUFFER_SIZE, f);

	/* Decompress */
	while (1) 
	{
		/* We need a dictionary reset */
		if (reset_hack) 
		{
			step = 9;
			dict_range = 0x0200;
			dict_index = 0x0102;
		}

		/* Since "pos" is in bits, we get position in bytes + small offset in bits */
		byte_pos = pos / 8;
		bit_pos = pos % 8;

		pos += step;	/* And advance to the next chunk */

		/* Edge of buffer, read more data from file */
		if (byte_pos >= MBUFFER_EDGE)
		{
				int bytes_extra = MBUFFER_SIZE - byte_pos;//~= 3
				int bytes_left = MBUFFER_SIZE - bytes_extra;//~= 1021

				/* Copy leftovers */
				for (j = 0; j < bytes_extra; j++) mbuffer[j] = mbuffer[bytes_left + j];

				/* Read in the rest */				
				n = fread(&mbuffer[bytes_extra], sizeof(char), bytes_left, f);

				/* Reset cursor */
				pos = bit_pos + step;	/* Add all unused bits */
				byte_pos = 0;
				/* On dictionary reset, use byte offset as bit offset*/
				if (reset_hack) bit_pos = bytes_extra;
		}
#ifdef DEBUG
printf("%04d\t0x%04X:%01X\t", pos, byte_pos, bit_pos);
#endif
		/* Read index from position "byte_pos", bit offset "bit_pos" */
		big_index = 
			((mbuffer[byte_pos+2] & 0x00FF) << 16) | 
			((mbuffer[byte_pos+1] & 0x00FF) << 8) | 
			(mbuffer[byte_pos] & 0x00FF);

		big_index >>= bit_pos;
		big_index &= 0x0000FFFF;

		next_index = big_index;
		next_index &= keyMask[ (step - 9) ];

		/* Apply the value as-is, continuing with dictionary reset, C) */
		if (reset_hack) 
		{
			/* Save index */
			last_index = next_index;
			/* Output char value */
			last_char = (next_index & 0x00FF);

			result[res_pos++] = last_char;
			/* We're done with the hack */
			reset_hack = 0;
			continue;
		}

		if (next_index == 0x0101)	/* End Of File */
		{
			/* DONE */
			break;
		}

		if (next_index == 0x0100) 	/* Reset dictionary */
		{
			/* Postpone it into next iteration */
			reset_hack = 1;
			/* Note: this hack avoids code duplication, but	makes the algorithm
			 * harder to follow. Basically, what happens, when the "reset" 
			 * command is being hit, is that 
			 * A) the dictionary is reset
			 * B) one more value is being read (this is the code duplication bit)
			 * C) the value is applied to output as-is
			 * D) we continue as normal
			 */
			continue;
		}

		/* Remember *real* "next_index" */
		keep_index = next_index;

		/* No dictionary entry to query, step back */
		if (next_index >= dict_index)
		{
			next_index = last_index;
			/* Queue 1 char */
			queue[queued++] = last_char;
		}

		/* Quering dictionary? */
		while (next_index > 0x00ff)
		{
			/* Queue 1 char */
			queue[queued++] = dict_val[next_index];
			/* Next query: */
			next_index = dict_key[next_index];
		}

		/* Queue 1 char */
		last_char = (next_index & 0x00FF);
		queue[queued++] = last_char;

		/* Unqueue */
		while (queued) 
		{
			result[res_pos++] = queue[--queued];
		}

		/* Save value to the dictionary */
		dict_key[dict_index] = last_index; /* "goto prev entry" */
		dict_val[dict_index] = last_char;  /* the value */
		dict_index++;

		/* Save *real* "next_index" */
		last_index = keep_index;

		/* Edge of dictionary, increase the bit-step, making range twice as large. */
		if (dict_index >= dict_range && step < 12) 
		{
			step += 1;
			dict_range *= 2;
		}
	}

	/* Done */
	//fclose(f);

	/* Print-out the result */
	//fprintf(stderr,"0000:0000   ");
	{
		int kk, jj = 0;//	for (kk = 0; kk < res_pos; kk++) {fprintf(stderr, "%02hhx ", result[kk]);if (++jj >= 16) { fprintf(stderr, "\n%04d:%04x   ",kk/16,kk+1); jj= 0;}}
		for (kk = 0; kk < res_pos; kk++) {buffer[kk] = result[kk];}
	}
	//fprintf(stderr,"\n");

	if (res_pos != expect_len) 
	{
		printf("WARNING!! Filesize mismatch, most likely an error.\n");
	}
	printf("COMP LEN: %d, %04x | UN LEN: %d, %04x\n", byte_pos, byte_pos, res_pos, res_pos);
	return res_pos;
}

void read_EXE(struct EXE *h, unsigned char *buffer);
void write_EXE(unsigned char *buffer, struct EXE *h);
void print_EXE(struct EXE *h, const char *name);

/* MAIN */
int main(int argc, char **argv) {

	const char *inputFile = NULL;
	const char *outputFile = NULL;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s INPUT-FILE OUTPUT-FILE\n", argv[0]);
		exit(-3);
	}
	if (argc > 1) inputFile = argv[1];
	if (argc > 2) outputFile = argv[2];

	unsigned char mzHeader[0x20];

	struct EXE mz;

	FILE *f;

	int n;

 	f = fopen(inputFile, "rb");

 	if (!f) {
 		fprintf(stderr, "Unable to open '%s'\n", inputFile);
 		return -1;
 	}

	n = fread(mzHeader, sizeof(char), mzSize, f); /* read bytes */

	if (n < mzSize) {
		fprintf(stderr, "Unable to read EXE header in '%s'\n", inputFile);
		fclose(f);
		return -1;
	}

	read_EXE(&mz, &mzHeader[0]); /* move bytes to struct */

	if (mz.signature != 0x5a4D) {
		fprintf(stderr, "Unable to parse EXE header in '%s'\n", inputFile);
		fclose(f);
		return -1;
	}

	print_EXE(&mz, inputFile); /* show some info to stdout */

	/* Verify this file is actually compressed, and not just an exe */
	/* We check if 0x200 contains "e99900". Another approach is to check
	 * if 0x24d contains "???.EXE" */
	unsigned char test_buf[4];
	fseek(f, 0x200, SEEK_SET);
	n = fread(test_buf, sizeof(char), 4, f); /* read bytes */
	if (n < 4 || test_buf[0] != 0xE9 || test_buf[1] != 0x99 || test_buf[2] != 0x00) {
		fprintf(stderr, "No signature found (%02x%02x%02x at 0x200), aborting.\n", test_buf[0],test_buf[1],test_buf[2]);
		fclose(f);
		return -1;
	}


	int exe_data_start = mz.header_paragraphs * 16L;

	int extra_data_start = mz.blocks_in_file * 512L;
	if (mz.bytes_in_last_block)
  		extra_data_start -= (512 - mz.bytes_in_last_block);

	printf("Compressed header starts at %d  %08x\n", extra_data_start, extra_data_start);

	unsigned char mzHeader2[0x25];

	fseek(f, extra_data_start, SEEK_SET);

	/* read packed exe header */
	n = fread(mzHeader2, sizeof(char), 0x25, f); /* read bytes */

	struct EXE innerMz;

	read_EXE(&innerMz, &mzHeader2[0]); /* move bytes to struct */

	int exe_data_start2 = innerMz.header_paragraphs * 16L;

	int extra_data_start2 = innerMz.blocks_in_file * 512L;
	if (innerMz.bytes_in_last_block)
  		extra_data_start2 -= (512 - innerMz.bytes_in_last_block);

	printf("Compressed   data starts at +%d +%08x\n", exe_data_start2, exe_data_start2);

	int expected_size = extra_data_start2 - exe_data_start2;

//	printf("Expected data size: %d (%04x)\n",expected_size,expected_size); 
//	print_EXE(&innerMz, "<packed>");

	fseek(f, extra_data_start + exe_data_start2, SEEK_SET);

	unsigned char mzHeader3[0x25];

	innerMz.header_paragraphs = 0x02;
	innerMz.reloc_table_offset = mzSize;

	int fullSize = expected_size + 0x20;

	innerMz.blocks_in_file = fullSize / 512 + 1;
	innerMz.bytes_in_last_block = fullSize % 512;

	innerMz.min_extra_paragraphs = fullSize / 64;	/* I don't know how to determine BSS properly :/ */

	/* Prepare new header */
	write_EXE(&mzHeader3[0], &innerMz);
	mzHeader3[0x1c] = mzHeader3[0x1d] = mzHeader3[0x1e] = mzHeader3[0x1f] = 0x00; //garbage to fill till 0x20

	/* un-LZW */
	char buffer[expected_size];

	n = load_infile(f, buffer, expected_size);

	/* show final info */
	printf("\n");
	print_EXE(&innerMz, outputFile);

	/* write it all to file */
	f = fopen(outputFile, "wb");

	fwrite(mzHeader3, 0x20, sizeof(char), f);
	fwrite(buffer, n, sizeof(char), f);

	fclose(f);

	return 0;
}

void print_EXE(struct EXE *h, const char *name) {
	int exe_data_start = h->header_paragraphs * 16L;

	int extra_data_start = h->blocks_in_file * 512L;
	if (h->bytes_in_last_block)
  		extra_data_start -= (512 - h->bytes_in_last_block);

	printf("%s\t\t\t(hex)\t\t(dec)\n", name);

	printf(".EXE size (bytes)\t\t%04x\t\t%d\n", extra_data_start, extra_data_start);
	printf("Blocks in file\t\t\t%04x\t\t%d\n", h->blocks_in_file, h->blocks_in_file);
	printf("Bytes in last block\t\t%04x\t\t%d\n", h->bytes_in_last_block, h->bytes_in_last_block);
	printf("Overlay number\t\t\t%04x\t\t%d\n", h->overlay_number, h->overlay_number);
	printf("Initial CS:IP\t\t\t%04x:%04x\n", h->cs, h->ip);
	printf("Initial SS:SP\t\t\t%04x:%04x\n", h->ss, h->sp);
	printf("Minimum allocation (para)\t%4x\t\t%d\n", h->min_extra_paragraphs, h->min_extra_paragraphs);
	printf("Maximum allocation (para)\t%4x\t\t%d\n", h->max_extra_paragraphs, h->max_extra_paragraphs);
	printf("Header size (para)\t\t%4x\t\t%d\n", h->header_paragraphs, h->header_paragraphs);
	printf("Relocation table offset\t\t%4x\t\t%d\n", h->reloc_table_offset, h->reloc_table_offset);
	printf("Relocation entries\t\t%4x\t\t%d\n", h->num_relocs, h->num_relocs);

	printf("\n");
}
void read_EXE(struct EXE *h, unsigned char *buffer) { /* This boring code is portable, unlike fread(struct..) */
	h->signature = READ_WORD(buffer, 0x00);
  	h->bytes_in_last_block = READ_WORD(buffer, 0x02);
  	h->blocks_in_file = READ_WORD(buffer, 0x04);
  	h->num_relocs = READ_WORD(buffer, 0x06);
  	h->header_paragraphs = READ_WORD(buffer, 0x08);
  	h->min_extra_paragraphs = READ_WORD(buffer, 0x0A);
  	h->max_extra_paragraphs = READ_WORD(buffer, 0x0C);
  	h->ss = READ_WORD(buffer, 0x0E);
  	h->sp = READ_WORD(buffer, 0x10);
  	h->checksum = READ_WORD(buffer, 0x12);
  	h->ip = READ_WORD(buffer, 0x14);
  	h->cs = READ_WORD(buffer, 0x16);
  	h->reloc_table_offset = READ_WORD(buffer, 0x18);
  	h->overlay_number= READ_WORD(buffer, 0x1A);
}
void write_EXE(unsigned char *buffer, struct EXE *h) { /* This boring code is portable, unlike fwrite(struct..) */
	WRITE_WORD(buffer, 0x00, h->signature);
  	WRITE_WORD(buffer, 0x02, h->bytes_in_last_block);
  	WRITE_WORD(buffer, 0x04, h->blocks_in_file);
  	WRITE_WORD(buffer, 0x06, h->num_relocs);
  	WRITE_WORD(buffer, 0x08, h->header_paragraphs);
  	WRITE_WORD(buffer, 0x0A, h->min_extra_paragraphs); 
  	WRITE_WORD(buffer, 0x0C, h->max_extra_paragraphs);
  	WRITE_WORD(buffer, 0x0E, h->ss);
  	WRITE_WORD(buffer, 0x10, h->sp);
  	WRITE_WORD(buffer, 0x12, h->checksum);
  	WRITE_WORD(buffer, 0x14, h->ip);
  	WRITE_WORD(buffer, 0x16, h->cs);
  	WRITE_WORD(buffer, 0x18, h->reloc_table_offset);
  	WRITE_WORD(buffer, 0x1A, h->overlay_number);
}

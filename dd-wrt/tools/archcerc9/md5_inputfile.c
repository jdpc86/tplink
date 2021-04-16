/*
  Copyright (c) 2014, Matthias Schiffer <mschiffer at universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/*
   mktplinkfw3

   Image generation tool for the TP-LINK SafeLoader as seen on
   TP-LINK Pharos devices (CPE210/220/510/520)
*/


#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "md5.h"


#define ALIGN(x,a) ({ typeof(a) __a = (a); (((x) + __a - 1) & ~(__a - 1)); })


/** An image partition table entry */
struct image_partition_entry {
	const char *name;
	size_t size;
	uint8_t *data;
};

/** A flash partition table entry */
struct flash_partition_entry {
	const char *name;
	uint32_t base;
	uint32_t size;
};


static const uint8_t jffs2_eof_mark[4] = {0xde, 0xad, 0xc0, 0xde};


/**
   Salt for the MD5 hash

   Fortunately, TP-LINK seems to use the same salt for most devices which use
   the new image format.
*/
static const uint8_t md5_salt[16] = {
	0x7a, 0x2b, 0x15, 0xed,
	0x9b, 0x98, 0x59, 0x6d,
	0xe5, 0x04, 0xab, 0x44,
	0xac, 0x2a, 0x9f, 0x4e,
};


/** Generates and writes the image MD5 checksum */
void put_md5(uint8_t *md5, uint8_t *buffer, unsigned int len) {
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, md5_salt, (unsigned int)sizeof(md5_salt));
	MD5_Update(&ctx, buffer, len);
	MD5_Final(md5, &ctx);
}

/** Allocates a new image partition */
struct image_partition_entry alloc_image_partition(const char *name, size_t len) {
	struct image_partition_entry entry = {name, len, malloc(len)};
	if (!entry.data)
		error(1, errno, "malloc");

	return entry;
}

/** Creates a new image partition with an arbitrary name from a file */
struct image_partition_entry read_file(const char *part_name, const char *filename, bool add_jffs2_eof) {
	struct stat statbuf;

	if (stat(filename, &statbuf) < 0)
		error(1, errno, "unable to stat file `%s'", filename);

	size_t len = statbuf.st_size;

	struct image_partition_entry entry = alloc_image_partition(part_name, len);

	FILE *file = fopen(filename, "rb");
	if (!file)
		error(1, errno, "unable to open file `%s'", filename);

	if (fread(entry.data, statbuf.st_size, 1, file) != 1)
		error(1, errno, "unable to read file `%s'", filename);


	fclose(file);

	return entry;
}

void put_partitions_plain(uint8_t *buffer, const struct image_partition_entry *parts) {
	size_t i;
	char *image_pt = (char *)buffer;
	size_t bases[1];
	size_t base = 0x0;
	for (i = 0; parts[i].name; i++) {
		memcpy(buffer + base, parts[i].data, parts[i].size);
		bases[i]=base;
		base += parts[i].size;
	}


	// *image_pt=0; 
	// memset(image_pt+1, 0xff, end-image_pt-1);
}

void * generate_md5_for_signature_image(const struct image_partition_entry *parts, size_t *len) {
	*len = 0x14;

	size_t i;
	for (i = 0; parts[i].name; i++)
		*len += parts[i].size;

	uint8_t *image = malloc(*len);
	if (!image)
		error(1, errno, "malloc");

	image[0] = *len >> 24;  //first 4, length of the firmware
	image[1] = *len >> 16;
	image[2] = *len >> 8;
	image[3] = *len;

	// memcpy(image+0x14, vendor, vendor_len);
	// memset(image+0x14+vendor_len, 0xff, 4096);

	put_partitions_plain(image + 0x14, parts);
    memset(image+0x14, 0x00, 0x1000);
	put_md5(image+0x04, image+0x14, *len-0x14);

	return image;
}

/** Frees an image partition */
void free_image_partition(struct image_partition_entry entry) {
	free(entry.data);
}


/** Generates an image for 665 according to c9v5 image, 1014 + special_config section followed by jffs2 that has bootloader, os and fs**/
static void do_md5_rsa_sig(const char *output, const char *kernel_image) {
	struct image_partition_entry parts[7] = {};

	
	// parts[0] = make_partition_table(cpe665_partitions); //we're skipping fs-uboot and softversion here so 4 parts in total
	parts[0] = read_file("file-system", kernel_image, false);
	// parts[1] = read_file("jffs2-image", kernel_image, false);
	// parts[3] = read_file("file-system", rootfs_image, add_jffs2_eof);
	// parts[4] = make_softversion(softversion, sizeof(softversion)-1);
	// parts[5] = make_merge_config(merge_config, sizeof(merge_config) - 1);
	// parts[5] = make_support_list(support_list, size - 1); // size is the size of different board passed in
	size_t len;
	void *image;
	
	image = generate_md5_for_signature_image(parts, &len);

	FILE *file = fopen(output, "wb");
	if (!file)
		error(1, errno, "unable to open output file");

	if (fwrite(image, len, 1, file) != 1)
		error(1, 0, "unable to write output file");

	fclose(file);

	free(image);

	size_t i;
	for (i = 0; parts[i].name; i++)
		free_image_partition(parts[i]);
}

/** Usage output */
void usage(const char *argv0) {
	fprintf(stderr,
		"Usage: %s [OPTIONS...]\n"
		"\n"
		"Options:\n"
		"  -B <board>      create image for the board specified with <board>\n"
		"  -k <file>       read kernel image from the file <file>\n"
		"  -o <file>       write output to the file <file>\n",
		argv0
	);
};


int main(int argc, char *argv[]) {
	const char *board = NULL, *kernel_image = NULL, *output = NULL;
	bool add_jffs2_eof = false, sysupgrade = false;

	while (true) {
		int c;

		c = getopt(argc, argv, "B:k:o:h");
		if (c == -1)
			break;

		switch (c) {
		case 'B':
			board = optarg;
			break;

		case 'k':
			kernel_image = optarg;
			break;

		case 'o':
			output = optarg;
			break;

		case 'h':
			usage(argv[0]);
			return 0;

		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (!board)
		error(1, 0, "no board has been specified");
	if (!kernel_image)
		error(1, 0, "no kernel image has been specified");
	if (!output)
		error(1, 0, "no output filename has been specified");

	if (strcmp(board, "md5_rsa_sig") == 0)
		do_md5_rsa_sig(output, kernel_image);
	else
		error(1, 0, "unsupported board %s", board);

	return 0;
}


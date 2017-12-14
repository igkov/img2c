#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

int bmp_load(pbmp_struct_t bmp, char *filename) {
	FILE *f; int i; int size;
	memset(bmp, 0x00, sizeof(bmp_struct_t));
	if ((f = fopen(filename, "rb")) == NULL) {
		return 1;
	}
	fseek(f, 0, SEEK_SET);
	fread(&bmp->BitMapHeader, sizeof(BITMAPFILEHEADER), 1, f);
	fread(&bmp->BitMapInfo, sizeof(BITMAPINFOHEADER), 1, f);
	bmp->h = bmp->BitMapInfo.biHeight;
	bmp->w = bmp->BitMapInfo.biWidth;
	bmp->byte_per_pixel = bmp->BitMapInfo.biBitCount / 8;
	bmp->bit_per_pixel  = bmp->BitMapInfo.biBitCount;
	if (bmp->byte_per_pixel) {
		bmp->line = bmp->byte_per_pixel * bmp->w;
		if (bmp->line & 0x03) {
			bmp->line = (bmp->line & ~0x0003) + 4;
		}
		bmp->rect_size      = bmp->h * bmp->line * bmp->byte_per_pixel;
	} else {
		bmp->line = bmp->w * bmp->bit_per_pixel / 8;
		if (bmp->line & 0x03) {
			bmp->line = (bmp->line & ~0x0003) + 4;
		}
		bmp->rect_size      = bmp->h * bmp->line;
	}
	bmp->palette_size   = 0;
	bmp->rect_offset    = bmp->BitMapHeader.bfOffBits;
	bmp->rect = (char*)malloc(bmp->rect_size*sizeof(uint8_t));
	if (bmp->rect == NULL) {
		return 1;
	}	
	fseek(f, bmp->rect_offset, SEEK_SET);
	fread(bmp->rect, bmp->rect_size, 1, f);
	return 0;
}

int bmp_getpixel(pbmp_struct_t bmp, int x, int y, int *value) {
	if (x >= bmp->w || y >= bmp->h) {
		return 1;
	}
	if (bmp->byte_per_pixel == 0) {
		if (bmp->bit_per_pixel == 1) {
			if (bmp->rect[bmp->line*(bmp->h-y-1) + x/8] & (1<<(7-(x&0x07)))) {
				*value = 0xFF;
			} else {
				*value = 0x00;
			}
		} else {
			return 2;
		}
	} else if (bmp->byte_per_pixel == 1) {
		if (bmp->rect != NULL) {
			*value = bmp->rect[bmp->line*(bmp->h-y-1) + x];
		}
	} else if (bmp->byte_per_pixel == 3) {
		if (bmp->rect != NULL) {
			*value  = bmp->rect[(bmp->line*(bmp->h-y-1) + x*3) + 2];
			*value <<= 8;
			*value |= bmp->rect[(bmp->line*(bmp->h-y-1) + x*3) + 1];
			*value <<= 8;
			*value |= bmp->rect[(bmp->line*(bmp->h-y-1) + x*3) + 0];
		}
	} else if (bmp->byte_per_pixel == 4) {
		if (bmp->rect != NULL) {
			*value  = bmp->rect[(bmp->line*(bmp->h-y-1) + x*4) + 2];
			*value <<= 8;
			*value |= bmp->rect[(bmp->line*(bmp->h-y-1) + x*4) + 1];
			*value <<= 8;
			*value |= bmp->rect[(bmp->line*(bmp->h-y-1) + x*4) + 0];
		}
	} else {
		return 3;
	}
	return 0;
}

int bmp_close(pbmp_struct_t bmp)
{
	free(bmp->pElementRGB);
	free(bmp->rect);
	memset(bmp, 0x00, sizeof(bmp_struct_t));
	return 0;
}

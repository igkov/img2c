/*
	img2c - bmp image to c-code convertor
	
	Конвертатор BMP-картинок в код на Си для встраивание в программы.

	e - простой вывод, только 2 цвета (по-умолчанию).
	g - в оттенках серого.
	s - упаковка 1 бит на пиксель.
	i - инверсия.
	p - упаковка тела lzss.
	
	Информация:
	https://github.com/igkov/img2c

	igorkov / fsp@igorkov.org / 2016-2017
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "bmp.h"

void help(void) {
	printf("BMP to C arrays convert utility\r\n");
	printf("img2c.exe file.bmp array_name [args]\r\n");
	printf("args:\r\n");
	printf("\tE or e - easy format (0/1)\r\n");
	printf("\tS or s - 1bit for pixel\r\n");
	printf("\tG or g - grayscale format\r\n");
	printf("\tI or i - inverse colors\r\n");
	printf("\tP or p - pack data (lzss algorithm)\r\n");
	printf("\r\nMore information on https://github.com/igkov/img2c\r\n");
	printf("igorkov / fsp@igorkov.org / 2016\r\n");
}

extern void lzss_pack(unsigned char *input, int sizein, unsigned char *output, int *sizeout);

int main(int argc, char **argv) {
	int ret;
	bmp_struct_t bmp;
	int x, y;
	int i;
	int pos;
	char *filename;
	char *iconame;
	char type = 0;
	char pack = 0;
	char negative = 0;
	
	unsigned char plain[64*64];
	unsigned char compress[64*64];
	
	if (argc == 1) {
		help();
		return 0;
	}
	if (argc < 2) {
		printf("ERROR: no input file!\r\n\r\n");
		help();
		return 0;
	}
	if (argc < 3) {
		printf("ERROR: no ico name!\r\n\r\n");
		help();
		return 0;
	}
	if (argc < 4) {
		type = 0;
	} else {
		int i = 0;
		while (argv[3][i]) {
			switch (argv[3][i]) {
			case 'E':
			case 'e':
				type = 0;
				break;
			case 'S':
			case 's':
				type = 1;
				break;
			case 'G':
			case 'g':
				type = 2;
				break;
			case 'I':
			case 'i':
				negative = 1;
				break;
			case 'P':
			case 'p':
				pack = 1;
				break;
			}
			i++;
		}
	}
	
	if (pack && type != 0 && type != 1 && type != 2) {
		printf("ERROR: pack support only for ico_t output (G/S options)!\r\n");
		return 0;
	}
	
	filename = argv[1];
	iconame = argv[2];
	
	ret = bmp_load(&bmp, filename);
	if (ret) {
		printf("ERROR: bmp_load() return %d\r\n", ret);
		return 0;
	}

	if (type == 1) {
		if (bmp.h%8) {
			// fix 2016.07.19 - Убрал проверку на ширину
			// fix 2016.07.25 - Только для высоты не кратной 8
			printf("ERROR: incorrect bmp size!\r\n");
			return 0;
		}
	}
	
	printf("// ico from file %s\n", filename);
	printf("const uint8_t %s_data[] = {\n", iconame);
	if (type == 1) {
		memset(plain, 0, sizeof(plain));
		if (bmp.w*bmp.h > sizeof(plain)) {
			pack = 0;
			printf("//WARNING: Big data (%db), not packed!\n", bmp.w*bmp.h);
		}
		for (y=0; y<bmp.h; y+=8) {
			for (x=0; x<bmp.w; x++) {
				int value = 0, b = 0;
				int i;
				for (i=0; i<8; i++) {
					ret = bmp_getpixel(&bmp, x, y+i, &value);
					if (ret) {
						printf("ERROR: bmp_getpixel() return %d\n", ret);
					}
					// bitmast
					if (value == 0xFFFFFF || value == 0xFF) {
					} else {
						if (pack == 1) {
							plain[y*bmp.w/8+x] |= (1<<i); 
						} else {
							b |= 1<<i;
						}
					}
				}
				if (pack == 0) {
					printf("0x%02x,", b);
				}
			}
		}
		if (pack == 1) {
			pos = sizeof(compress);
			lzss_pack(plain, bmp.w*bmp.h/8, compress, &pos);
			for (i=0; i<pos; i++) {
				printf("0x%02x, ", compress[i]);
			}
		} else {
			pos = bmp.w*bmp.h;
		}
		printf("};\n");
		if (pack) {
			printf("const ico_t %s = {%d, %d, %d, %s_data};\n\n", iconame, bmp.w, bmp.h, pos, iconame);
		}
	} else if (type == 2) {
		i = 0;
		if (bmp.w*bmp.h > sizeof(plain)) {
			pack = 0;
			printf("//WARNING: Big data (%db), not packed!\n", bmp.w*bmp.h);
		}
		for (y=0; y<bmp.h; y++) {
			for (x=0; x<bmp.w; x++) {
				int value = 0;
				ret = bmp_getpixel(&bmp, x, y, &value);
				if (ret) {
					printf("ERROR: bmp_getpixel() return %d\n", ret);
				}
				// Gray:
				if (bmp.byte_per_pixel == 0 || 
					bmp.byte_per_pixel == 1) {
					if (negative) {
						if (pack == 0) {
							printf("0x%02X,", 0XFF-value); // grays
						} else {
							plain[i] = 0XFF-value;
							i++;
						}
					} else {
						if (pack == 0) {
							printf("0x%02X,", value); // grays
						} else {
							plain[i] = value;
							i++;
						}
					}
				} else 
				if (bmp.byte_per_pixel == 3 || 
					bmp.byte_per_pixel == 4) {
					int gray = ((value & 0xFF) + ((value>>8) & 0xFF) + ((value>>16) & 0xFF)) / 3;
					if (negative) {
						if (pack == 0) {
							printf("0x%02X,", 0xFF-gray); // grays
						} else {
							plain[i] = 0XFF-gray;
							i++;
						}
					} else {
						if (pack == 0) {
							printf("0x%02X,", gray); // grays
						} else {
							plain[i] = gray;
							i++;
						}
					}
				}
			}
			if (pack == 0) {
				printf("\n");
			}
		}
		if (pack) {
			pos = sizeof(compress);
			lzss_pack(plain, i, compress, &pos);
			for (i=0; i<pos; i++) {
				printf("0x%02x, ", compress[i]);
			}
		} else {
			pos = bmp.w*bmp.h;
		}
		printf("};\n");
		printf("const ico_t %s = {%d, %d, %d, %s_data};\n\n", iconame, bmp.w, bmp.h, pos, iconame);
	} else {
		i = 0;
		if (bmp.w*bmp.h > sizeof(plain)) {
			pack = 0;
			printf("//WARNING: Big data (%db), not packed!\n", bmp.w*bmp.h);
		}
		for (y=0; y<bmp.h; y++) {
			for (x=0; x<bmp.w; x++) {
				int value = 0;
				ret = bmp_getpixel(&bmp, x, y, &value);
				if (ret) {
					printf("ERROR: bmp_getpixel() return %d\n", ret);
				}
				// B/W:
				if (bmp.byte_per_pixel == 0 || 
					bmp.byte_per_pixel == 1) {
					if ((value == 0xFF) ^ negative) {
						if (pack) {
							plain[i] = 1;
							i++;
						} else {
							printf("1,");
						}
					} else {
						if (pack) {
							plain[i] = 0;
							i++;
						} else {
							printf("0,");
						}
					}
				} else 
				if (bmp.byte_per_pixel == 3) {
					if ((value == 0xFFFFFF) ^ negative) {
						if (pack) {
							plain[i] = 1;
							i++;
						} else {
							printf("1,");
						}
					} else {
						if (pack) {
							plain[i] = 0;
							i++;
						} else {
							printf("0,");
						}
					}
				} else 
				if (bmp.byte_per_pixel == 4) {
					if ((value == 0xFFFFFFFF) ^ negative) {
						if (pack) {
							plain[i] = 1;
							i++;
						} else {
							printf("1,");
						}
					} else {
						if (pack) {
							plain[i] = 0;
							i++;
						} else {
							printf("0,");
						}
					}
				}
			}
			if (pack == 0) {
				printf("\n");
			}
		}
		if (pack) {
			pos = sizeof(compress);
			lzss_pack(plain, i, compress, &pos);
			for (i=0; i<pos; i++) {
				printf("0x%02x, ", compress[i]);
			}
		} else {
			pos = bmp.w*bmp.h;
		}
		printf("};\n");
		printf("const ico_t %s = {%d, %d, %d, %s_data};\n\n", iconame, bmp.w, bmp.h, pos, iconame);
	}
	
	bmp_close(&bmp);
	return 0;
}

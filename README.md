# img2c
img2c - BMP image to C-code convertor

Icons structure:
```C
typedef struct {
	uint8_t sizex;
	uint8_t sizey;
	uint16_t size;
	const uint8_t *data;
} ico_t, *pico_t;
```

Highlevel logic unpacked data if sizex*sizey != size.

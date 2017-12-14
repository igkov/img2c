gcc -O2 -s -o img2c.exe img2c.c bmp.c lzss.c

@REM examples:
img2c.exe test/ico_24bit.bmp ico16_at
img2c.exe test/ico_24bit.bmp ico16_at s
img2c.exe test/ico_24bit.bmp ico16_at g
img2c.exe test/ico_1bit.bmp ico_1bit
img2c.exe test/ico_1bit.bmp ico_1bit s
img2c.exe test/ico_1bit.bmp ico_1bit sp

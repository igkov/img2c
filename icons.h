#ifndef __ICONS_H__
#define __ICONS_H__

#include <stdint.h>

// ���� ������� ������������ ��� �����������, ����� ��������� ����������!
typedef struct {
	uint8_t sizex;
	uint8_t sizey;
	uint16_t size;
	const uint8_t *data;
} ico_t, *pico_t;

#endif 

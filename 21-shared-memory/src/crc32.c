#include <stdint.h>

uint32_t crc32c(uint32_t crc, const void *buf, size_t size)
{
	const uint8_t *p = buf;

	while (size--)
		crc = crc32Table[(crc ^ *p++) & 0xff] ^ (crc >> 8);

	return crc;
}
#pragma once

static inline void write_u16_be(
		uint8_t *buf,
		size_t offset,
		uint16_t value)
{
	buf[offset] = (value >> 8) & 0xFF;
	buf[offset + 1] = value & 0xFF;
}

static inline uint16_t read_u16_be(
		const uint8_t *buf,
		size_t offset
		)
{
	return ((uint16_t)buf[offset] << 8)
		| buf[offset + 1];
}

#include "internal.h"

uuid_t qcom_token_guid = {0x2b, 0x8c, 0x2f, 0x88, 0x46, 0x96, 0x5f, 0x43, 0x8d, 0xe5, 0xf2, 0x08, 0xff, 0x80, 0xc1, 0xbd};
uuid_t svarstore_uuid = {0x65, 0xaa, 0x0e, 0x4b, 0x1c, 0x73, 0xf4, 0x45, 0x80, 0x4d, 0xd0, 0x34, 0xa9, 0xd5, 0x0b, 0x62};

static inline int hex2bin(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -EILSEQ;
}

static int parse_hex_byte(const char *str, uint8_t *byte) {
	int h1, h2;
	h1 = hex2bin(str[0]);
	h2 = hex2bin(str[1]);
	if (h1 < 0 || h2 < 0) return -EILSEQ;
	*byte = (h1 << 4) | h2;
	return 0;
}

int svarstore_uuid_parse(const char *in, uuid_t uu) {
	int i, pos;
	if (!in || !uu) return -EINVAL;
	if (in[8] != '-' || in[13] != '-' || in[18] != '-' || in[23] != '-') return -EILSEQ;
	pos = 0;
	for (i = 3; i >= 0; i--) {
		if (parse_hex_byte(&in[pos], &uu[i]) < 0) return -EILSEQ;
		pos += 2;
	}
	pos++;
	for (i = 5; i >= 4; i--) {
		if (parse_hex_byte(&in[pos], &uu[i]) < 0) return -EILSEQ;
		pos += 2;
	}
	pos++;
	for (i = 7; i >= 6; i--) {
		if (parse_hex_byte(&in[pos], &uu[i]) < 0) return -EILSEQ;
		pos += 2;
	}
	pos++;
	for (i = 8; i < 10; i++) {
		if (parse_hex_byte(&in[pos], &uu[i]) < 0) return -EILSEQ;
		pos += 2;
	}
	pos++;
	for (i = 10; i < 16; i++) {
		if (parse_hex_byte(&in[pos], &uu[i]) < 0) return -EILSEQ;
		pos += 2;
	}
	return 0;
}

void svarstore_uuid_unparse(const uuid_t uu, char *out) {
	if (!uu || !out) return;
	snprintf(out, 37,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		uu[3], uu[2], uu[1], uu[0], uu[5], uu[4], uu[7], uu[6],               
		uu[8], uu[9], uu[10], uu[11], uu[12], uu[13], uu[14], uu[15]
	);
}

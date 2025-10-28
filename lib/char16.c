#include "internal.h"

size_t char16_strlen(const char16_t *s) {
	size_t len = 0;
	while (s[len]) len++;
	return len;
}

int char16_strcmp(const char16_t *s1, const char16_t *s2) {
	while (*s1 && (*s1 == *s2)) s1++, s2++;
	return (int)(*s1 - *s2);
}

int char16_strncmp(const char16_t *s1, const char16_t *s2, size_t n) {
	size_t i;
	for (i = 0; i < n; i++) {
		if (s1[i] != s2[i]) return (int)(s1[i] - s2[i]);
		if (!s1[i]) break;
	}
	return 0;
}

char16_t *char16_strcpy(char16_t *dest, const char16_t *src) {
	char16_t *d = dest;
	while ((*d++ = *src++));
	return dest;
}

char16_t *char16_strncpy(char16_t *dest, const char16_t *src, size_t n) {
	char16_t *d = dest;
	size_t i;
	for (i = 0; i < n; i++) if (!(*d++ = *src++)) {
		while (++i < n) *d++ = 0;
		break;
	}
	return dest;
}

ssize_t char16_to_char8(char *dst, size_t dst_size, const char16_t *src, size_t src_size) {
	size_t i, len = 0;
	if (!dst || !src) return -EINVAL;
	if (dst_size == 0) return -EINVAL;
	if (src_size != 0) len = src_size;
	else while (src[len]) len++;
	if (len + 1 > dst_size) return -ENOSPC;
	for (i = 0; i < len; i++)
		dst[i] = src[i] > 0xFF ? '?' : (char)src[i];
	dst[len] = 0;
	return (ssize_t)len;
}

ssize_t char8_to_char16(char16_t *dst, size_t dst_size, const char *src, size_t src_size) {
	size_t i, len = 0;
	if (!dst || !src) return -EINVAL;
	if (dst_size == 0) return -EINVAL;
	if (src_size != 0) len = src_size;
	else while (src[len]) len++;
	if (len + 1 > dst_size) return -ENOSPC;
	for (i = 0; i < len; i++)
		dst[i] = (char16_t)(unsigned char)src[i];
	dst[len] = 0;
	return (ssize_t)len;
}

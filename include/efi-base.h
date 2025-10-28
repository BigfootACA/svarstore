#ifndef _EFI_BASE_H_
#define _EFI_BASE_H_
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#define ALIGN_VALUE(Value, Alignment) (((Value) + (Alignment) - 1) & ~((Alignment) - 1))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#define EFIVARS_PATH "/sys/firmware/efi/efivars"
#define EFI_PTAB_HEADER_ID 0x5452415020494645  /* "EFI PART" */
#define EFI_VARIABLE_NON_VOLATILE                            0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS                      0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS                          0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD                   0x00000008
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS   0x00000020
#define EFI_VARIABLE_APPEND_WRITE                            0x00000040
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS              0x00000010

typedef uint8_t uuid_t[16];
typedef unsigned short char16_t;

typedef struct efi_table_header {
	uint64_t signature;
	uint32_t revision;
	uint32_t headersize;
	uint32_t crc32;
	uint32_t reserved;
} efi_table_header;
static_assert(sizeof(efi_table_header) == 24, "efi_table_header size mismatch");

typedef struct mbr_part {
	uint8_t bootind;
	uint8_t start_head;
	uint8_t start_sector;
	uint8_t start_track;
	uint8_t osind;
	uint8_t end_head;
	uint8_t end_sector;
	uint8_t end_track;
	uint8_t start_lba[4];
	uint8_t size_lba[4];
} __attribute__((packed)) mbr_part;
static_assert(sizeof(mbr_part) == 16, "mbr_part size mismatch");

typedef struct mbr_head {
	uint8_t bootcode[440];
	uint8_t id[4];
	uint8_t unknown[2];
	mbr_part partition[4];
	uint16_t sign;
} __attribute__((packed)) mbr_head;
static_assert(sizeof(mbr_head) == 512, "mbr_head size mismatch");

typedef struct efi_part_table {
	efi_table_header header;
	uint64_t self_lba;
	uint64_t alt_lba;
	uint64_t first_usable;
	uint64_t last_usable;
	uuid_t guid;
	uint64_t part_lba;
	uint32_t part_cnt;
	uint32_t part_size;
	uint32_t part_crc32;
} __attribute__((packed)) efi_part_table;
static_assert(sizeof(efi_part_table) == 92, "efi_part_table size mismatch");

typedef struct efi_part_entry {
	uuid_t type;
	uuid_t uuid;
	uint64_t start;
	uint64_t end;
	uint64_t attr;
	char16_t name[36];
} __attribute__((packed)) efi_part_entry;
static_assert(sizeof(efi_part_entry) == 128, "efi_part_entry size mismatch");

extern size_t char16_strlen(const char16_t *s);
extern int char16_strcmp(const char16_t *s1, const char16_t *s2);
extern int char16_strncmp(const char16_t *s1, const char16_t *s2, size_t n);
extern char16_t *char16_strcpy(char16_t *dest, const char16_t *src);
extern char16_t *char16_strncpy(char16_t *dest, const char16_t *src, size_t n);
extern ssize_t char16_to_char8(char *dst, size_t dst_size, const char16_t *src, size_t src_size);
extern ssize_t char8_to_char16(char16_t *dst, size_t dst_size, const char *src, size_t src_size);
extern int efi_table_get_crc32(efi_table_header *hdr, uint32_t *crc32_out);
extern uint32_t svarstore_crc32(uint32_t crc, const uint8_t *buf, size_t len);
extern int svarstore_uuid_parse(const char *in, uuid_t uu);
extern void svarstore_uuid_unparse(const uuid_t uu, char *out);
#endif

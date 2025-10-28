#include "internal.h"

const simple_varstore_entry svarstore_zero_entry = {0};

int svarstore_init_store(simple_varstore *stor) {
	efi_table_header *hdr;
	if (!stor || !stor->buffer || stor->size < MIN_STOR_SIZE) return -EINVAL;
	memset(stor->buffer, 0, stor->size);
	hdr = (efi_table_header *)stor->buffer;
	hdr->signature = SIMPLE_VARSTORE_SIGNATURE;
	hdr->revision = SIMPLE_VARSTORE_VERSION;
	hdr->headersize = sizeof(efi_table_header) + sizeof(simple_varstore_entry);
	return efi_table_get_crc32(hdr, &hdr->crc32);
}

bool svarstore_is_valid(simple_varstore *stor) {
	uint32_t crc32 = 0;
	efi_table_header *hdr;
	if (!stor || !stor->buffer || stor->size < MIN_STOR_SIZE) return false;
	hdr = (efi_table_header *)stor->buffer;
	if (
		hdr->signature != SIMPLE_VARSTORE_SIGNATURE ||
		hdr->revision != SIMPLE_VARSTORE_VERSION ||
		hdr->headersize < sizeof(efi_table_header) ||
		hdr->headersize > stor->size
	)return false;
	if (efi_table_get_crc32(hdr, &crc32) != 0) return false;
	if (crc32 != hdr->crc32) return false;
	return true;
}

size_t svarstore_get_store_size(simple_varstore *stor) {
	if (!svarstore_is_valid(stor)) return 0;
	return MIN(stor->size, ((efi_table_header *)stor->buffer)->headersize);
}

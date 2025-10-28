#include "internal.h"

int svarstore_get_next_var_name(
	simple_varstore	 *stor,
	size_t *name_size,
	char16_t *name,
	uuid_t *guid
) {
	int ret;
	uint8_t *buffer;
	char16_t *entry_name;
	efi_table_header *hdr;
	bool find_next = false;
	simple_varstore_entry *entry;
	size_t offset, pdata_size;
	if (!name_size || !name || !guid) return -EINVAL;
	if (!svarstore_is_valid(stor) && (ret = svarstore_init_store(stor)) != 0) return ret;
	hdr = (efi_table_header *)stor->buffer;
	buffer = (uint8_t *)stor->buffer;
	if (!name[0]) find_next = true;
	offset = sizeof(efi_table_header);
	while (true) {
		offset = ALIGN_VALUE(offset, 16);
		if (hdr->headersize - offset < sizeof(simple_varstore_entry)) break;
		entry = (simple_varstore_entry *)(buffer + offset);
		offset += sizeof(simple_varstore_entry);
		if (memcmp(entry, &svarstore_zero_entry, sizeof(simple_varstore_entry)) == 0) break;
		if (hdr->headersize - offset < entry->name_len) break;
		entry_name = (char16_t *)(buffer + offset);
		offset = ALIGN_VALUE(offset + entry->name_len, 16);
		pdata_size = entry->data_len + 4;
		if (hdr->headersize - offset < pdata_size) break;
		offset = ALIGN_VALUE(offset + pdata_size, 16);
		if (find_next) {
			if (*name_size < entry->name_len) {
				*name_size = entry->name_len;
				return -ENOBUFS;
			}
			memset(name, 0, *name_size);
			memcpy(name, entry_name, entry->name_len);
			memcpy(*guid, entry->guid, sizeof(uuid_t));
			*name_size = entry->name_len;
			return 0;
		}
		if (
			memcmp(entry->guid, *guid, sizeof(uuid_t)) == 0 &&
			char16_strncmp(name, entry_name, entry->name_len / sizeof(char16_t)) == 0
		) find_next = true;
	}
	return -ENOENT;
}

int svarstore_getvar(
	simple_varstore *stor,
	char16_t *name,
	uuid_t *guid,
	uint32_t *attributes,
	size_t *data_size,
	void *data
) {
	char16_t *entry_name;
	int ret;
	efi_table_header *hdr;
	uint8_t *buffer, *entry_data;
	simple_varstore_entry *entry;
	size_t offset, pdata_size;
	if (!name || !guid || !data_size) return -EINVAL;
	if (!svarstore_is_valid(stor) && (ret = svarstore_init_store(stor)) != 0) return ret;
	hdr = (efi_table_header *)stor->buffer;
	buffer = (uint8_t *)stor->buffer;
	offset = sizeof(efi_table_header);
	while (true) {
		offset = ALIGN_VALUE(offset, 16);
		if (hdr->headersize - offset < sizeof(simple_varstore_entry)) break;
		entry = (simple_varstore_entry *)(buffer + offset);
		if (memcmp(entry, &svarstore_zero_entry, sizeof(simple_varstore_entry)) == 0) break;
		offset += sizeof(simple_varstore_entry);
		if (hdr->headersize - offset < entry->name_len) break;
		entry_name = (char16_t *)(buffer + offset);
		offset = ALIGN_VALUE(offset + entry->name_len, 16);
		pdata_size = entry->data_len + 4;
		if (hdr->headersize - offset < pdata_size) break;
		entry_data = buffer + offset;
		offset = ALIGN_VALUE(offset + pdata_size, 16);
		if (memcmp(entry->guid, *guid, sizeof(uuid_t)) != 0) continue;
		if (char16_strcmp(name, entry_name) != 0) continue;
		if (attributes)
			*attributes = entry->attributes;
		if (*data_size < entry->data_len) {
			*data_size = entry->data_len;
			return -ENOBUFS;
		}
		if (data && entry->data_len > 0)
			memcpy(data, entry_data, entry->data_len);
		*data_size = entry->data_len;
		return 0;
	}
	return -ENOENT;
}

int svarstore_setvar(
	simple_varstore *stor,
	char16_t *name,
	uuid_t *guid,
	uint32_t attributes,
	size_t data_size,
	void *data
) {
	char16_t *entry_name;
	int ret;
	efi_table_header *hdr;
	bool found = false;
	uint8_t *buffer, *entry_data;
	simple_varstore_entry *entry;
	size_t move_from, move_to, move_size;
	size_t name_len, pdata_size;
	size_t found_entry_size = 0, new_entry_size;
	size_t offset, found_offset = 0, pentry_offset;
	if (!name || !guid) return -EINVAL;
	if (!svarstore_is_valid(stor) && (ret = svarstore_init_store(stor)) != 0) return ret;
	hdr = (efi_table_header *)stor->buffer;
	buffer = (uint8_t *)stor->buffer;
	name_len = (char16_strlen(name) + 1) * sizeof(char16_t);
	new_entry_size = sizeof(simple_varstore_entry) +
		ALIGN_VALUE(name_len, 16) +
		ALIGN_VALUE(data_size + 4, 16);
	offset = sizeof(efi_table_header);
	while (true) {
		offset = ALIGN_VALUE(offset, 16);
		if (hdr->headersize - offset < sizeof(simple_varstore_entry)) return -ENOSPC;
		entry = (simple_varstore_entry *)(buffer + offset);
		if (memcmp(entry, &svarstore_zero_entry, sizeof(simple_varstore_entry)) == 0) break;
		offset += sizeof(simple_varstore_entry);
		if (hdr->headersize - offset < entry->name_len) return -ENOSPC;
		entry_name = (char16_t *)(buffer + offset);
		offset = ALIGN_VALUE(offset + entry->name_len, 16);
		pdata_size = entry->data_len + 4;
		if (hdr->headersize - offset < pdata_size) return -ENOSPC;
		entry_data = buffer + offset;
		offset = ALIGN_VALUE(offset + pdata_size, 16);
		if (
			!found &&
			memcmp(entry->guid, *guid, sizeof(uuid_t)) == 0 &&
			char16_strcmp(name, entry_name) == 0
		){
			found = true;
			found_offset = (uint8_t *)entry - buffer;
			found_entry_size = offset - found_offset;
		}
	}
	if (data_size == 0) {
		if (!found) return -ENOENT;
		move_from = found_offset + found_entry_size;
		move_size = hdr->headersize - move_from;
		move_to = found_offset;
		if (move_size > 0)
			memmove(buffer + move_to, buffer + move_from, move_size);
		hdr->headersize -= found_entry_size;
		memset(buffer + hdr->headersize, 0, found_entry_size);
		return efi_table_get_crc32(hdr, &hdr->crc32);
	}
	if (found) {
		if (new_entry_size == found_entry_size) {
			entry = (simple_varstore_entry *)(buffer + found_offset);
			entry->attributes = attributes;
			entry->data_len = data_size;
			entry_data = buffer + found_offset +
				sizeof(simple_varstore_entry) +
				ALIGN_VALUE(name_len, 16);
			memcpy(entry_data, data, data_size);
			memset(entry_data + data_size, 0, 4);
			return efi_table_get_crc32(hdr, &hdr->crc32);
		} else {
			move_from = found_offset + found_entry_size;
			move_size = hdr->headersize - move_from;
			move_to = found_offset;
			if (move_size > 0)
				memmove(buffer + move_to, buffer + move_from, move_size);
			hdr->headersize -= found_entry_size;
		}
	}
	offset = ALIGN_VALUE(offset, 16);
	pentry_offset = offset;
	if (offset + new_entry_size + sizeof(simple_varstore_entry) > stor->size)
		return -ENOSPC;
	entry = (simple_varstore_entry *)(buffer + offset);
	offset += sizeof(simple_varstore_entry);
	entry_name = (char16_t *)(buffer + offset);
	offset = ALIGN_VALUE(offset + name_len, 16);
	entry_data = buffer + offset;
	offset = ALIGN_VALUE(offset + data_size + 4, 16);
	if (offset - pentry_offset != new_entry_size) return -ENOSPC;
	memcpy(entry->guid, *guid, sizeof(uuid_t));
	entry->attributes = attributes;
	entry->name_len = (uint32_t)name_len;
	entry->data_len = data_size;
	memcpy(entry_name, name, name_len);
	memcpy(entry_data, data, data_size);
	memset(entry_data + data_size, 0, 4);
	memset(buffer + offset, 0, sizeof(simple_varstore_entry));
	offset += sizeof(simple_varstore_entry);
	hdr->headersize = offset;
	return efi_table_get_crc32(hdr, &hdr->crc32);
}

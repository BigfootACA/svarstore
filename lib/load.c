#include "internal.h"

static int parse_var(int fd, uint32_t *attr, size_t *size, uint8_t **data, size_t *data_size) {
	struct stat st;
	ssize_t ret;
	void *nval;
	if (fstat(fd, &st) != 0) return -errno;
	if ((size_t)st.st_size < sizeof(*attr)) return -ENODATA;
	*size = st.st_size - sizeof(*attr);
	if ((ret = pread(fd, attr, sizeof(*attr), 0)) != sizeof(*attr))
		return ret < 0 ? -errno : -EIO;
	if (*data_size < *size) {
		nval = (*data) ? realloc(*data, *size) : malloc(*size);
		if (!nval) return -ENOMEM;
		*data = nval, *data_size = *size;
	}
	if ((ret = pread(fd, *data, *size, sizeof(*attr))) != (ssize_t)(*size))
		return ret < 0 ? -errno : -EIO;
	return 0;
}

int svarstore_vars_to_store(simple_varstore *stor) {
	DIR *dir;
	uuid_t guid;
	uint32_t attributes;
	int fd, dfd, ret = 0;
	efi_table_header *hdr;
	struct dirent *entry_dir;
	simple_varstore_entry *entry;
	char16_t *var_name = NULL, *entry_name;
	uint8_t *data = NULL, *buffer, *entry_data;
	size_t new_entry_size, data_size = 0, data_size_buffer = 0;
	size_t offset, name_len, elen;
	ssize_t conv_ret;
	char *dash_pos;
	char guid_str[40];
	if (!stor) return -EINVAL;
	if ((ret = svarstore_init_store(stor))) return ret;
	hdr = (efi_table_header *)stor->buffer;
	buffer = (uint8_t *)stor->buffer;
	offset = sizeof(efi_table_header);
	if (!(dir = opendir(EFIVARS_PATH))) return -errno;
	if (!(var_name = malloc(256 * sizeof(char16_t)))) {
		ret = -ENOMEM;
		goto cleanup;
	}
	if ((dfd = dirfd(dir)) < 0) {
		ret = -errno;
		goto cleanup;
	}
	while((entry_dir = readdir(dir))) {
		if (entry_dir->d_name[0] == '.') continue;
		if ((elen = strlen(entry_dir->d_name)) < 38) continue;
		dash_pos = entry_dir->d_name + elen - 37;
		if (*dash_pos != '-') continue;
		strncpy(guid_str, dash_pos + 1, sizeof(guid_str) - 1);
		guid_str[36] = 0;
		if (svarstore_uuid_parse(guid_str, guid) != 0) continue;
		if ((fd = openat(dfd, entry_dir->d_name, O_RDONLY)) < 0) continue;
		ret = parse_var(fd, &attributes, &data_size, &data, &data_size_buffer);
		close(fd);
		if (ret != 0) continue;
		conv_ret = char8_to_char16(var_name, 256, entry_dir->d_name, elen - 37);
		if (conv_ret < 0) continue;
		name_len = (conv_ret + 1) * sizeof(char16_t);
		if (!(attributes & EFI_VARIABLE_NON_VOLATILE)) continue;
		new_entry_size = sizeof(simple_varstore_entry) +
			ALIGN_VALUE(name_len, 16) +
			ALIGN_VALUE(data_size + 4, 16);
		offset = ALIGN_VALUE(offset, 16);
		if (offset + new_entry_size + sizeof(simple_varstore_entry) > stor->size) {
			ret = -ENOSPC;
			break;
		}
		entry = (simple_varstore_entry *)(buffer + offset);
		offset += sizeof(simple_varstore_entry);
		entry_name = (char16_t *)(buffer + offset);
		offset = ALIGN_VALUE(offset + name_len, 16);
		entry_data = buffer + offset;
		offset = ALIGN_VALUE(offset + data_size + 4, 16);
		memcpy(entry->guid, guid, sizeof(uuid_t));
		entry->attributes = attributes;
		entry->name_len = (uint32_t)name_len;
		entry->data_len = data_size;
		memcpy(entry_name, var_name, name_len);
		memcpy(entry_data, data, data_size);
		memset(entry_data + data_size, 0, 4);
	}
	offset = ALIGN_VALUE(offset, 16);
	memset(buffer + offset, 0, sizeof(simple_varstore_entry));
	offset += sizeof(simple_varstore_entry);
	hdr->headersize = offset;
	efi_table_get_crc32(hdr, &hdr->crc32);
cleanup:
	if (dir) closedir(dir);
	if (var_name) free(var_name);
	if (data) free(data);
	return ret;
}

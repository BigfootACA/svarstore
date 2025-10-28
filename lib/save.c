#include "internal.h"

static int try_open(const char *path) {
	int fd, flags = 0;
	if ((fd = open(path, O_RDONLY)) >= 0) {
		if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == 0) {
			flags &= ~FS_IMMUTABLE_FL;
			ioctl(fd, FS_IOC_SETFLAGS, &flags);
		}
		close(fd);
	}
	if ((fd = open(path, O_WRONLY)) >= 0) return fd;
	if (errno == ENOENT && (fd = open(
		path, O_WRONLY | O_CREAT, 0644
	)) >= 0) return fd;
	return -1;
}

static bool write_var(const char *name, simple_varstore_entry *entry, uint8_t *entry_data) {
	int fd;
	ssize_t total;
	bool success = false;
	struct iovec iov[2];
	char filepath[4096], guid_str[40];
	svarstore_uuid_unparse(entry->guid, guid_str);
	snprintf(filepath, sizeof(filepath), EFIVARS_PATH "/%s-%s", name, guid_str);
	if ((fd = try_open(filepath)) < 0) return false;
	iov[0].iov_base = &entry->attributes;
	iov[0].iov_len = sizeof(uint32_t);
	iov[1].iov_base = entry_data;
	iov[1].iov_len = entry->data_len;
	total = sizeof(uint32_t) + entry->data_len;
	if ((writev(fd, iov, 2)) != total) goto done;
	success = true;
done:
	close(fd);
	return success;
}

int svarstore_store_to_vars(simple_varstore *stor) {
	ssize_t conv_ret;
	char16_t *entry_name;
	efi_table_header *hdr;
	size_t offset, pdata_size;
	uint8_t *buffer, *entry_data;
	simple_varstore_entry *entry;
	char var_name_utf8[256];
	int success = 0, total = 0;
	if (!svarstore_is_valid(stor)) return -EINVAL;
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
		if (!entry_name[0] || entry->data_len > UINT32_MAX) continue;
		total++;
		conv_ret = char16_to_char8(var_name_utf8, sizeof(var_name_utf8), entry_name, 0);
		if (conv_ret < 0 || !var_name_utf8[0]) continue;
		if (write_var(var_name_utf8, entry, entry_data)) success++;
	}
	return total != 0 && success == 0 ? -EIO : 0;
}

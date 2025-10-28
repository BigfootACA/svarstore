#include <sys/ioctl.h>
#include <sys/poll.h>
#include <linux/fs.h>
#include <mtd/mtd-user.h>
#include "internal.h"

struct blkdev {
	char *path;
	int fd;
	enum blkdev_type {
		DEV_NONE,
		DEV_BLK,
		DEV_MTD,
		DEV_MTDBLK,
	} type;
	bool cur_rw;
	bool allow_rw;
	size_t bsize;
};

static void waitfd(int fd, int op) {
	struct pollfd pfd = {
		.fd = fd,
		.events = op,
	};
	while (poll(&pfd, 1, -1) < 0)
		if (errno != EINTR) break;
}

static int xpread(struct blkdev *dev, void *buffer, size_t size, uint64_t offset) {
	int retry_cnt = 0;
	size_t total_read = 0;
	while (total_read < size) {
		errno = 0;
		void *dst = (uint8_t *)buffer + total_read;
		size_t chunk = MIN(size - total_read, 4096);
		uint64_t off = offset + total_read;
		ssize_t ret = pread(dev->fd, dst, chunk, off);
		if (ret < 0) switch (errno) {
			case EAGAIN: {
				waitfd(dev->fd, POLLOUT);
				continue;
			}
			case ETIMEDOUT: case EINTR: {
				if (++retry_cnt >= 3) return -EIO;
				continue;
			}
			default: return -errno;
		} else if (ret == 0) return -EIO;
		retry_cnt = 0, total_read += ret;
	}
	return 0;
}

static int xpwrite(struct blkdev *dev, const void *buffer, size_t size, uint64_t offset) {
	int retry_cnt = 0;
	size_t total_written = 0;
	while (total_written < size) {
		errno = 0;
		void *dst = (uint8_t *)buffer + total_written;
		size_t chunk = MIN(size - total_written, 4096);
		uint64_t off = offset + total_written;
		ssize_t ret = pwrite(dev->fd, dst, chunk, off);
		if (ret < 0) switch (errno) {
			case EAGAIN: {
				waitfd(dev->fd, POLLOUT);
				continue;
			}
			case ETIMEDOUT: case EINTR: {
				if (++retry_cnt >= 3) return -EIO;
				continue;
			}
			default: return -errno;
		} else if (ret == 0) return -EIO;
		retry_cnt = 0, total_written += ret;
	}
	return 0;
}

static int blk_read(struct blkdev *dev, uint64_t offset, void *buffer, size_t size) {
	if (!dev || !buffer || size == 0) return -EINVAL;
	return xpread(dev, buffer, size, offset);
}

static int blk_write(struct blkdev *dev, uint64_t offset, const void *buffer, size_t size) {
	if (!dev || !buffer || size == 0) return -EINVAL;
	if (!dev->allow_rw) return -EROFS;
	if (!dev->cur_rw) {
		int fd = open(dev->path, O_RDWR);
		if (fd < 0) return -errno;
		close(dev->fd);
		dev->fd = fd, dev->cur_rw = true;
	}
	if (dev->type == DEV_MTD) {
		uint64_t erase_start = (offset / dev->bsize) * dev->bsize;
		uint64_t erase_end = ((offset + size + dev->bsize - 1) / dev->bsize) * dev->bsize;
		size_t erase_len = erase_end - erase_start;
		void *erase_buf = NULL;
		ssize_t ret;
		if (!(erase_buf = malloc(erase_len))) return -ENOMEM;
		if ((ret = xpread(dev, erase_buf, erase_len, erase_start)) < 0) {
			free(erase_buf);
			return ret;
		}
		memcpy((uint8_t *)erase_buf + (offset - erase_start), buffer, size);
		struct erase_info_user erase = {
			.start = erase_start,
			.length = erase_len,
		};
		if (ioctl(dev->fd, MEMERASE, &erase) < 0) {
			free(erase_buf);
			return -errno;
		}
		ret = xpwrite(dev, erase_buf, erase_len, erase_start);
		free(erase_buf);
		return ret;
	} else if (dev->type == DEV_BLK || dev->type == DEV_MTDBLK)
		return xpwrite(dev, buffer, size, offset);
	return -ENOTSUP;
}

static struct blkdev *blk_open(const char *path, bool rw, enum blkdev_type type) {
	struct blkdev dev = {0}, *ret;
	if (!path) return NULL;
	dev.allow_rw = rw;
	dev.type = type;
	if ((dev.fd = open(path, O_RDONLY)) < 0) return NULL;
	if (!(dev.path = strdup(path))) goto fail;
	if (type == DEV_MTD) {
		struct mtd_info_user mtdinfo;
		if (ioctl(dev.fd, MEMGETINFO, &mtdinfo) < 0) goto fail;
		dev.bsize = mtdinfo.erasesize;
	} else if (type == DEV_BLK || type == DEV_MTDBLK) {
		uint32_t bsize = 0;
		if (ioctl(dev.fd, BLKSSZGET, &bsize) < 0) goto fail;
		dev.bsize = bsize;
	} else goto fail;
	if ((ret = malloc(sizeof(struct blkdev)))) {
		memcpy(ret, &dev, sizeof(struct blkdev));
		return ret;
	}
fail:
	if (dev.path) free(dev.path);
	close(dev.fd);
	return NULL;
}

static void blk_close(struct blkdev *dev) {
	if (!dev) return;
	if (dev->fd >= 0) close(dev->fd);
	free(dev);
}

static int try_probe_gpt(struct blkdev *dev, efi_part_table *gpt) {
	int ret;
	size_t vbsize;
	if (!dev || !gpt) return -EINVAL;
	if ((ret = blk_read(dev, dev->bsize, gpt, sizeof(efi_part_table))) < 0) return ret;
	if (gpt->header.signature == EFI_PTAB_HEADER_ID) return 0;
	if (dev->bsize == 512) vbsize = 4096;
	else if (dev->bsize == 4096) vbsize = 512;
	else return -ENOTSUP;
	if ((ret = blk_read(dev, vbsize, gpt, sizeof(efi_part_table))) < 0) return ret;
	if (gpt->header.signature != EFI_PTAB_HEADER_ID) return -ENOTSUP;
	dev->bsize = vbsize;
	return 0;
}

static int blk_lookup_by_type(struct blkdev *dev, uuid_t type, uint64_t *offset, size_t *size) {
	int ret;
	static const uuid_t empty = {0};
	mbr_head mbr;
	efi_part_table gpt;
	efi_part_entry *entry;
	char *buffer = NULL;
	uint32_t i, saved_crc, calc_crc;
	if (!dev || !offset || !size) return -EINVAL;
	if ((ret = blk_read(dev, 0, &mbr, sizeof(mbr))) < 0) return ret;
	if (mbr.sign != 0xAA55) return -ENOTSUP;
	if ((ret = try_probe_gpt(dev, &gpt)) < 0) return ret;
	if (gpt.part_size < sizeof(efi_part_entry)) return -EUCLEAN;
	saved_crc = gpt.header.crc32;
	gpt.header.crc32 = 0;
	calc_crc = svarstore_crc32(0, (const uint8_t *)&gpt, gpt.header.headersize);
	if (saved_crc != calc_crc) return -EUCLEAN;
	gpt.header.crc32 = saved_crc;
	if (!(buffer = malloc(gpt.part_cnt * gpt.part_size))) return -ENOMEM;
	if ((ret = blk_read(dev, gpt.part_lba * dev->bsize, buffer, gpt.part_cnt * gpt.part_size)) < 0) {
		free(buffer);
		return ret;
	}
	calc_crc = svarstore_crc32(0, (const uint8_t *)buffer, gpt.part_cnt * gpt.part_size);
	if (gpt.part_crc32 != calc_crc) {
		free(buffer);
		return -EUCLEAN;
	}
	for (i = 0; i < gpt.part_cnt; i++) {
		entry = (efi_part_entry *)(buffer + (i * gpt.part_size));
		if (memcmp(entry->type, empty, sizeof(uuid_t)) == 0) break;
		svarstore_uuid_unparse(entry->type, buffer);
		if (memcmp(entry->type, type, sizeof(uuid_t)) != 0) continue;
		*offset = entry->start * dev->bsize;
		*size = (entry->end - entry->start + 1) * dev->bsize;
		free(buffer);
		return 0;
	}
	free(buffer);
	return -ENOENT;
}

static int blkdev_all_lookup_by_type(
	struct blkdev **dev,
	bool rw,
	uuid_t type,
	uint64_t *offset,
	size_t *size
) {
	int ret;
	DIR *dir;
	char path[512];
	struct dirent *entry;
	struct blkdev *blkdev;
	if (!dev || !offset || !size) return -EINVAL;
	if (!(dir = opendir("/dev"))) return -errno;
	while ((entry = readdir(dir))) {
		enum blkdev_type btype = DEV_NONE;
		snprintf(path, sizeof(path), "/dev/%s", entry->d_name);
		if (entry->d_name[0] == '.') continue;
		if (entry->d_type == DT_CHR) {
			if (strncmp(entry->d_name, "mtd", 3) == 0) btype = DEV_MTD;
			if (strncmp(entry->d_name + strlen(entry->d_name) - 2, "ro", 2) == 0) continue;
		}
		if (entry->d_type == DT_BLK)
			btype = strncmp(entry->d_name, "mtd", 3) == 0 ? DEV_MTDBLK : DEV_BLK;
		if (btype == DEV_NONE) continue;
		if (!(blkdev = blk_open(path, rw, btype))) continue;
		if ((ret = blk_lookup_by_type(blkdev, type, offset, size)) == 0) {
			*dev = blkdev;
			closedir(dir);
			return 0;
		}
		blk_close(blkdev);
	}
	closedir(dir);
	return -ENOENT;
}

int blk_load_by_type(uuid_t type, void **data, size_t *size) {
	struct blkdev *dev = NULL;
	uint64_t offset = 0;
	size_t part_size = 0;
	int ret;
	if (!data || !size) return -EINVAL;
	*data = NULL, *size = 0;
	if ((ret = blkdev_all_lookup_by_type(
		&dev, false, type, &offset, &part_size
	)) != 0) return ret;
	if (part_size == 0) {
		ret = -ENODATA;
		goto fail;
	}
	if (!(*data = malloc(part_size))) {
		ret = -ENOMEM;
		goto fail;
	}
	if ((ret = blk_read(dev, offset, *data, part_size)) != 0) goto fail;
	*size = part_size;
	blk_close(dev);
	return ret;
fail:
	if (*data) {
		free(*data);
		*data = NULL;
	}
	blk_close(dev);
	return ret;
}

int blk_save_by_type(uuid_t type, const void *data, size_t size) {
	int ret;
	uint64_t offset = 0;
	size_t part_size = 0;
	struct blkdev *dev = NULL;
	if (!data || size == 0) return -EINVAL;
	if ((ret = blkdev_all_lookup_by_type(
		&dev, true, type, &offset, &part_size
	)) != 0) return ret;
	if (size > part_size) ret = -ENOSPC;
	else ret = blk_write(dev, offset, data, size);
	blk_close(dev);
	return ret;
}

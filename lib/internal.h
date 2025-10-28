#ifndef INTERNAL_H
#define INTERNAL_H
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/fs.h>
#include "svarstore.h"
#include "efi-base.h"
#include "blk.h"

extern uuid_t svarstore_uuid;
extern uuid_t qcom_token_guid;
extern const simple_varstore_entry svarstore_zero_entry;
#endif

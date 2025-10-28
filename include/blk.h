#ifndef _BLK_H_
#define _BLK_H_
#include "efi-base.h"
extern int blk_load_by_type(uuid_t type, void **data, size_t *size);
extern int blk_save_by_type(uuid_t type, const void *data, size_t size);
#endif

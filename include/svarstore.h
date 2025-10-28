#ifndef _SIMPLEVARSTORE_H_
#define _SIMPLEVARSTORE_H_
#include "efi-base.h"

#define MIN_STOR_SIZE 0x2000
#define SIMPLE_VARSTORE_SIGNATURE 0x524f545352415653 /* "SVARSTOR" */
#define SIMPLE_VARSTORE_VERSION   0x00010000

typedef struct simple_varstore {
	void *buffer;
	size_t size;
} simple_varstore;

typedef struct simple_varstore_entry {
	uuid_t guid;
	uint32_t attributes;
	uint32_t name_len;
	uint64_t data_len;
} simple_varstore_entry;

extern int svarstore_init_store(simple_varstore *stor);
extern bool svarstore_is_valid(simple_varstore *stor);
extern int svarstore_get_next_var_name(simple_varstore *stor, size_t *name_size, char16_t *name, uuid_t *guid);
extern int svarstore_getvar(simple_varstore *stor, char16_t *name, uuid_t *guid, uint32_t *attributes, size_t *data_size, void *data);
extern int svarstore_setvar(simple_varstore *stor, char16_t *name, uuid_t *guid, uint32_t attributes, size_t data_size, void *data);
extern size_t svarstore_get_store_size(simple_varstore *stor);
extern int svarstore_vars_to_store(simple_varstore *stor);
extern int svarstore_store_to_vars(simple_varstore *stor);
extern simple_varstore *svarstore_new_store(void *buff, size_t size);
extern simple_varstore *svarstore_create_store(size_t size);
extern simple_varstore *svarstore_load_store(void);
extern int svarstore_save_store(simple_varstore *stor);
extern void svarstore_free_store(simple_varstore *stor);
extern simple_varstore* svarstore_dup_store(simple_varstore *orig);
#endif

#include "internal.h"

int cmd_erase(void) {
	int ret;
	simple_varstore *stor;
	if (!(stor = svarstore_create_store(MIN_STOR_SIZE))) return -1;
	ret = svarstore_save_store(stor);
	svarstore_free_store(stor);
	return ret;
}

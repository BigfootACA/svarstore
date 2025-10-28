#include "internal.h"

int cmd_flush(void) {
	simple_varstore *stor;
	int ret;
	if (!(stor = svarstore_load_store())) return -1;
	printf("flushing EFI variables to store...\n");
	if ((ret = svarstore_vars_to_store(stor)) < 0) {
		fprintf(stderr, "failed to flush variables: %s\n", strerror(-ret));
		svarstore_free_store(stor);
		return -1;
	}
	if ((ret = svarstore_save_store(stor)) < 0) {
		svarstore_free_store(stor);
		return -1;
	}
	printf("variables flushed successfully\n");
	svarstore_free_store(stor);
	return 0;
}

#include "internal.h"

int cmd_reload(void) {
	simple_varstore *stor;
	int ret;
	if (!(stor = svarstore_load_store())) return -1;
	printf("reloading EFI variables from store...\n");
	if ((ret = svarstore_store_to_vars(stor)) < 0) {
		fprintf(stderr, "failed to reload variables: %s\n", strerror(-ret));
		svarstore_free_store(stor);
		return -1;
	}
	printf("variables reloaded successfully\n");
	svarstore_free_store(stor);
	return 0;
}

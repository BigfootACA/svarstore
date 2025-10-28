#include "internal.h"

static volatile sig_atomic_t running = 1;

static void signal_handler(int sig) {
	(void)sig;
	running = 0;
}

int cmd_daemon(void) {
	simple_varstore *stor, *stor2;
	int ret;
	printf("starting daemon mode...\n");
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	if (!(stor = svarstore_load_store()))
		stor = svarstore_create_store(MIN_STOR_SIZE);
	if (!stor) {
		printf("failed to create or load variable store\n");
		return -1;
	}
	printf("initial sync: loading variables from store...\n");
	if ((ret = svarstore_store_to_vars(stor)) < 0)
		fprintf(stderr, "warning: initial sync failed: %s\n", strerror(-ret));
	stor2 = svarstore_dup_store(stor);
	printf("svarstore daemon running\n");
	while (running) {
		sleep(30);
		if ((ret = svarstore_vars_to_store(stor)) < 0) {
			fprintf(stderr, "warning: failed to serialize variables: %s\n", strerror(-ret));
			continue;
		}
		if (stor2) {
			if (memcmp(stor->buffer, stor2->buffer, stor->size) == 0) continue;
			svarstore_free_store(stor2);
		}
		stor2 = svarstore_dup_store(stor);
		if ((ret = svarstore_save_store(stor)) < 0)
			fprintf(stderr, "warning: failed to save store: %s\n", strerror(-ret));
	}
	printf("\nShutting down daemon...\n");
	if (svarstore_vars_to_store(stor) == 0)
		svarstore_save_store(stor);
	svarstore_free_store(stor);
	svarstore_free_store(stor2);
	return 0;
}

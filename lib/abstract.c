#include "internal.h"

simple_varstore *svarstore_new_store(void *buff, size_t size) {
	simple_varstore *stor;
	if (!(stor = malloc(sizeof(simple_varstore)))) {
		fprintf(stderr, "failed to allocate memory for variable store\n");
		return NULL;
	}
	stor->buffer = buff;
	stor->size = size;
	return stor;
}

simple_varstore *svarstore_create_store(size_t size) {
	simple_varstore *stor;
	void *buffer = NULL;
	if (size != 0 && !(buffer = malloc(size))) {
		fprintf(stderr, "failed to allocate memory for variable store\n");
		return NULL;
	}
	if (!(stor = svarstore_new_store(buffer, size))) {
		if (buffer) free(buffer);
		return NULL;
	}
	if (size > 0 && svarstore_init_store(stor) != 0) {
		if (buffer) free(buffer);
		free(stor);
		fprintf(stderr, "failed to initialize variable store\n");
		return NULL;
	}
	return stor;
}

simple_varstore *svarstore_load_store(void) {
	simple_varstore *stor;
	void *buffer = NULL;
	size_t size = 0;
	int ret;
	if ((ret = blk_load_by_type(svarstore_uuid, &buffer, &size)) < 0 || !buffer) {
		fprintf(stderr, "failed to load variable store: %s\n", strerror(-ret));
		return NULL;
	}
	if (!(stor = svarstore_new_store(buffer, size))) {
		free(buffer);
		return NULL;
	}
	if (!svarstore_is_valid(stor)) {
		fprintf(stderr, "Invalid variable store\n");
		free(buffer);
		free(stor);
		return NULL;
	}
	return stor;
}

int svarstore_save_store(simple_varstore *stor) {
	int ret;
	if (!svarstore_is_valid(stor)) {
		fprintf(stderr, "invalid store\n");
		return -1;
	}
	if ((ret = blk_save_by_type(svarstore_uuid, stor->buffer, stor->size)) < 0) {
		fprintf(stderr, "failed to save variable store: %s\n", strerror(-ret));
		return ret;
	}
	return 0;
}

void svarstore_free_store(simple_varstore *stor) {
	if (!stor) return;
	if (stor->buffer) free(stor->buffer);
	free(stor);
}

simple_varstore* svarstore_dup_store(simple_varstore *orig) {
	void *buffer = NULL;
	if (!orig) return NULL;
	if (orig->buffer && !(buffer = malloc(orig->size))) return NULL;
	if (buffer) memcpy(buffer, orig->buffer, orig->size);
	return svarstore_new_store(buffer, orig->size);
}

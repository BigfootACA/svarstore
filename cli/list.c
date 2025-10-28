#include "internal.h"

int cmd_list(void) {
	simple_varstore *stor;
	char16_t name[256];
	char name_utf8[256];
	uuid_t guid;
	size_t name_size;
	int ret;
	char uuid_str[37];
	if (!(stor = svarstore_load_store())) return -1;
	name_size = sizeof(name) / sizeof(char16_t);
	name[0] = 0;
	while ((ret = svarstore_get_next_var_name(stor, &name_size, name, &guid)) == 0) {
		svarstore_uuid_unparse(guid, uuid_str);
		memset(name_utf8, 0, sizeof(name_utf8));
		char16_to_char8(name_utf8, sizeof(name_utf8), name, 0);
		printf("%s %s\n", uuid_str, name_utf8);
		name_size = sizeof(name) / sizeof(char16_t);
	}
	if (ret != -ENOENT) {
		fprintf(stderr, "error listing variables: %s\n", strerror(-ret));
		svarstore_free_store(stor);
		return -1;
	}
	svarstore_free_store(stor);
	return 0;
}

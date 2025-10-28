#include "internal.h"

int cmd_order_set(const char *order_str) {
	simple_varstore *stor;
	boot_dev_order order;
	int ret;
	if (!order_str) {
		fprintf(stderr, "error: order list is required\n");
		return -1;
	}
	order = boot_dev_order_from_string(order_str);
	if (!is_boot_dev_order_valid(&order)) {
		fprintf(stderr, "error: invalid boot device order\n");
		return -1;
	}
	if (!(stor = svarstore_load_store())) return -1;
	if ((ret = set_boot_dev_order(stor, &order)) < 0) {
		fprintf(stderr, "failed to set boot device order: %s\n", strerror(-ret));
		svarstore_free_store(stor);
		return -1;
	}
	if ((ret = svarstore_save_store(stor)) < 0) {
		svarstore_free_store(stor);
		return -1;
	}
	print_boot_dev_order(&order);
	svarstore_free_store(stor);
	return 0;
}

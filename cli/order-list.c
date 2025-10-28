#include "internal.h"

void print_boot_dev_order(boot_dev_order *order) {
	for (size_t i = 0; i < ARRAY_SIZE(order->orders); i++) {
		if (order->orders[i] != DEV_NONE) printf(
			"%zu %s\n", i + 1,
			boot_dev_type_to_string(order->orders[i])
		);
	}
}

int cmd_order_list(void) {
	int ret;
	simple_varstore *stor;
	boot_dev_order order;
	if (!(stor = svarstore_load_store())) return -1;
	if ((ret = get_boot_dev_order(stor, &order)) < 0) {
		fprintf(stderr, "failed to get boot device order: %s\n", strerror(-ret));
		svarstore_free_store(stor);
		return -1;
	}
	print_boot_dev_order(&order);
	svarstore_free_store(stor);
	return 0;
}

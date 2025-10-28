#include "internal.h"

int cmd_order_replace(const char *replace_str) {
	simple_varstore *stor;
	boot_dev_order order;
	int ret, position;
	char *equal_sign, *type_str;
	boot_dev_type type;
	if (!replace_str) {
		fprintf(stderr, "error: replacement specification is required\n");
		return -1;
	}
	if (!(equal_sign = strchr(replace_str, '='))) {
		fprintf(stderr, "error: invalid format, expected N=type\n");
		return -1;
	}
	position = atoi(replace_str);
	if (position < 1 || position > (int)ARRAY_SIZE(order.orders)) {
		fprintf(stderr, "error: position must be between 1 and %zu\n", ARRAY_SIZE(order.orders));
		return -1;
	}
	type_str = equal_sign + 1;
	type = boot_dev_type_from_string(type_str);
	if (type != DEV_NONE && !is_boot_dev_type_valid(type)) {
		fprintf(stderr, "error: invalid device type '%s'\n", type_str);
		return -1;
	}
	if (!(stor = svarstore_load_store())) return -1;
	if ((ret = get_boot_dev_order(stor, &order)) < 0) {
		fprintf(stderr, "failed to get boot device order: %s\n", strerror(-ret));
		svarstore_free_store(stor);
		return -1;
	}
	order.orders[position - 1] = type;
	if (!is_boot_dev_order_valid(&order)) {
		fprintf(stderr, "error: resulting boot device order is invalid\n");
		svarstore_free_store(stor);
		return -1;
	}
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

#include "internal.h"

int main(int argc, char **argv) {
	int opt, ret = 0;
	bool list = false, flush = false, reload = false, erase = false, daemon = false;
	bool order_list = false, order_set = false, order_replace = false, found = false;
	char *order_set_arg = NULL, *order_replace_arg = NULL;
	static struct option long_options[] = {
		{"list",          no_argument,       0, 'l'},
		{"flush",         no_argument,       0, 'f'},
		{"reload",        no_argument,       0, 'r'},
		{"erase",         no_argument,       0, 'e'},
		{"daemon",        no_argument,       0, 'd'},
		{"order-list",    no_argument,       0, 'o'},
		{"order-set",     required_argument, 0, 's'},
		{"order-replace", required_argument, 0, 'p'},
		{"help",          no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};
	if (argc < 2) return usage(1);
	while ((opt = getopt_long(argc, argv, "lfredh", long_options, NULL)) != -1) switch (opt) {
		case 'l':
			if (found) goto dup;
			list = true, found = true;
		break;
		case 'f':
			if (found) goto dup;
			flush = true, found = true;
		break;
		case 'r':
			if (found) goto dup;
			reload = true, found = true;
		break;
		case 'e':
			if (found) goto dup;
			erase = true, found = true;
		break;
		case 'd':
			if (found) goto dup;
			daemon = true, found = true;
		break;
		case 'o':
			if (found) goto dup;
			order_list = true, found = true;
		break;
		case 's':
			if (found) goto dup;
			order_set = true, found = true;
			order_set_arg = optarg;
		break;
		case 'p':
			if (found) goto dup;
			order_replace = true, found = true;
			order_replace_arg = optarg;
		break;
		case 'h': return usage(0);
		default: return usage(1);
	}
	if (!found) return usage(1);
	if (list) ret = cmd_list();
	if (flush) ret = cmd_flush();
	if (reload) ret = cmd_reload();
	if (erase) ret = cmd_erase();
	if (daemon) ret = cmd_daemon();
	if (order_list) ret = cmd_order_list();
	if (order_set) ret = cmd_order_set(order_set_arg);
	if (order_replace) ret = cmd_order_replace(order_replace_arg);
	return ret;
dup:
	fprintf(stderr, "error: multiple commands specified\n");
	return usage(1);
}

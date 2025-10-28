#include "internal.h"

int usage(int e) {
	fprintf(
		e == 0 ? stdout : stderr,
		"Usage: svarstore [OPTIONS]\n"
		"Simple Variable Store - EFI Variable Store Management Tool\n\n"
		"Options:\n"
		"  -l, --list                  List all EFI variables in the store\n"
		"  -f, --flush                 Flush changes to the EFI variable store\n"
		"  -r, --reload                Reload EFI variables from the store\n"
		"  -e, --erase                 Erase all EFI variables in the store\n"
		"  -d, --daemon                Start a daemon to monitor and sync EFI variables\n"
		"  --order-list                List the current boot device order\n"
		"  --order-set <list>          Set the boot device order (comma-separated list)\n"
		"  --order-replace <N>=<type>  Replace the boot device at position N(1-12) with <type>\n"
		"  -h, --help                  Display this help message\n\n"
	);
	return e;
}

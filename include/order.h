#ifndef BOOT_DEV_ORDER_H
#define BOOT_DEV_ORDER_H
#include "svarstore.h"

typedef enum boot_dev_type {
	DEV_NONE = 0,
	DEV_CDROM,
	DEV_EMMC,
	DEV_FLOPPY,
	DEV_IDE,
	DEV_NET,
	DEV_NVME,
	DEV_SATA,
	DEV_SCSI,
	DEV_SD,
	DEV_SPINOR,
	DEV_UFS,
	DEV_USB,
	DEV_MAX
} boot_dev_type;

typedef struct boot_dev_order {
	boot_dev_type orders[DEV_MAX - 1];
} boot_dev_order;

extern const char* boot_dev_type_to_string(boot_dev_type type);
extern const char* boot_dev_type_to_description(boot_dev_type type);
extern boot_dev_type boot_dev_type_from_string(const char* str);
extern bool is_boot_dev_type_supported(boot_dev_type type);
extern bool is_boot_dev_type_valid(boot_dev_type type);
extern bool is_boot_dev_order_valid(boot_dev_order *order);
extern boot_dev_order boot_dev_order_from_string(const char* str);
extern int get_boot_dev_order(simple_varstore *store, boot_dev_order *order);
extern int set_boot_dev_order(simple_varstore *store, boot_dev_order *order);
#endif

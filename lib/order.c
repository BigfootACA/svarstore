#include "order.h"
#include "internal.h"

const char* boot_dev_type_to_string(boot_dev_type type) {
	switch (type) {
		case DEV_CDROM: return "CDROM";
		case DEV_EMMC: return "eMMC";
		case DEV_FLOPPY: return "Floppy";
		case DEV_IDE: return "IDE";
		case DEV_NET: return "Network";
		case DEV_NVME: return "NVMe";
		case DEV_SATA: return "SATA";
		case DEV_SCSI: return "SCSI";
		case DEV_SD: return "SD";
		case DEV_SPINOR: return "SPI-NOR";
		case DEV_UFS: return "UFS";
		case DEV_USB: return "USB";
		default: return "None";
	}
}

const char* boot_dev_type_to_description(boot_dev_type type) {
	switch (type) {
		case DEV_CDROM: return "CD-ROM Drive";
		case DEV_EMMC: return "eMMC Module";
		case DEV_FLOPPY: return "Floppy Drive";
		case DEV_IDE: return "IDE Disk";
		case DEV_NET: return "Network Boot";
		case DEV_NVME: return "NVMe Disk";
		case DEV_SATA: return "SATA Disk";
		case DEV_SCSI: return "SCSI Disk";
		case DEV_SD: return "SD Card";
		case DEV_SPINOR: return "SPI NOR Flash";
		case DEV_UFS: return "UFS Module";
		case DEV_USB: return "USB Mass Storage";
		default: return NULL;
	}
}


boot_dev_type boot_dev_type_from_string(const char* str) {
	if (strcasecmp(str, "CDROM") == 0) return DEV_CDROM;
	if (strcasecmp(str, "eMMC") == 0) return DEV_EMMC;
	if (strcasecmp(str, "Floppy") == 0) return DEV_FLOPPY;
	if (strcasecmp(str, "IDE") == 0) return DEV_IDE;
	if (strcasecmp(str, "Network") == 0) return DEV_NET;
	if (strcasecmp(str, "NVMe") == 0) return DEV_NVME;
	if (strcasecmp(str, "SATA") == 0) return DEV_SATA;
	if (strcasecmp(str, "SCSI") == 0) return DEV_SCSI;
	if (strcasecmp(str, "SD") == 0) return DEV_SD;
	if (strcasecmp(str, "SPI-NOR") == 0) return DEV_SPINOR;
	if (strcasecmp(str, "SPINOR") == 0) return DEV_SPINOR;
	if (strcasecmp(str, "UFS") == 0) return DEV_UFS;
	if (strcasecmp(str, "USB") == 0) return DEV_USB;
	return DEV_NONE;
}

boot_dev_order boot_dev_order_from_string(const char* str) {
	boot_dev_order order;
	char* token;
	char* str_copy;
	size_t index = 0;
	memset(&order, 0, sizeof(boot_dev_order));
	if (!str) return order;
	str_copy = strdup(str);
	if (!str_copy) return order;
	token = strtok(str_copy, ",");
	while (token && index < ARRAY_SIZE(order.orders)) {
		boot_dev_type type = boot_dev_type_from_string(token);
		if (type != DEV_NONE)
			order.orders[index++] = type;
		token = strtok(NULL, ",");
	}
	free(str_copy);
	return order;
}

bool is_boot_dev_type_valid(boot_dev_type type) {
	switch (type) {
		case DEV_CDROM:
		case DEV_EMMC:
		case DEV_FLOPPY:
		case DEV_IDE:
		case DEV_NET:
		case DEV_NVME:
		case DEV_SATA:
		case DEV_SCSI:
		case DEV_SD:
		case DEV_SPINOR:
		case DEV_UFS:
		case DEV_USB:
			return true;
		default:
			return false;
	}
}

bool is_boot_dev_order_valid(boot_dev_order *order) {
	bool has_valid = false;
	for (size_t i = 0; i < ARRAY_SIZE(order->orders); i++) {
		if (order->orders[i] == DEV_NONE) continue;
		if (!is_boot_dev_type_valid(order->orders[i])) return false;
		has_valid = true;
	}
	return has_valid;
}

int get_boot_dev_order(simple_varstore *store, boot_dev_order *order) {
	int ret;
	size_t size;
	uintptr_t value = 0;
	boot_dev_order vorder;
	if (!store || !order) return -EINVAL;
	memset(order, 0, sizeof(boot_dev_order));
	size = sizeof(uintptr_t);
	ret = svarstore_getvar(store, u"EnableBootDevOrder", &qcom_token_guid, NULL, &size, &value);
	if (ret == 0 && value == 0) return -ENODATA;
	size = sizeof(boot_dev_order);
	ret = svarstore_getvar(store, u"BootDevOrder", &qcom_token_guid, NULL, &size, &vorder);
	if (ret != 0 || size != sizeof(boot_dev_order)) return ret;
	memcpy(order, &vorder, sizeof(boot_dev_order));
	return 0;
}

int set_boot_dev_order(simple_varstore *store, boot_dev_order *order) {
	uintptr_t enable = 1;
	if (!store || !order) return -EINVAL;
	svarstore_setvar(
		store, u"EnableBootDevOrder", &qcom_token_guid,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
		sizeof(enable), &enable
	);
	return svarstore_setvar(
		store, u"BootDevOrder", &qcom_token_guid,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
		sizeof(boot_dev_order), order
	);
}

#ifndef INTERNAL_H
#define INTERNAL_H
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include "svarstore.h"
#include "blk.h"
#include "order.h"
#include "efi-base.h"

extern int usage(int e);
extern int cmd_daemon(void);
extern int cmd_erase(void);
extern int cmd_flush(void);
extern int cmd_list(void);
extern void print_boot_dev_order(boot_dev_order *order);
extern int cmd_order_list(void);
extern int cmd_order_replace(const char *replace_str);
extern int cmd_order_set(const char *order_str);
extern int cmd_reload(void);
#endif

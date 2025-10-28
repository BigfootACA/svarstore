svarstore_cli_src = files([
	'daemon.c',
	'erase.c',
	'flush.c',
	'list.c',
	'main.c',
	'order-list.c',
	'order-replace.c',
	'order-set.c',
	'reload.c',
	'usage.c',
])

executable(
	'svarstore',
	svarstore_cli_src,
	include_directories: incs,
	link_with: [libsvarstore_static],
	install: true,
)

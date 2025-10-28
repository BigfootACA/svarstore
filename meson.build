project(
	'svarstore', 'c',
	version: '0.1',
	default_options: [
		'warning_level=3',
		'werror=true',
		'optimization=3',
		'b_lto=true',
	],
)

incs = []
incs += include_directories('include')

install_headers(
	'include/blk.h',
	'include/efi-base.h',
	'include/order.h',
	'include/svarstore.h',
	subdir: 'svarstore',
)

subdir('lib')
subdir('cli')

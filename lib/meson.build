svarstore_lib_srcs = files([
	'abstract.c',
	'basic.c',
	'blk.c',
	'char16.c',
	'crc32.c',
	'getset.c',
	'load.c',
	'order.c',
	'save.c',
	'uuid.c',
])

libsvarstore_shared = shared_library(
	'svarstore',
	svarstore_lib_srcs,
	include_directories: incs,
	install: true,
	version: meson.project_version(),
	soversion: '0',
)

libsvarstore_static = static_library(
	'svarstore',
	svarstore_lib_srcs,
	include_directories: incs,
	install: true,
)

libsvarstore = libsvarstore_shared

pkg = import('pkgconfig')
pkg.generate(
	libsvarstore_shared,
	name: 'libsvarstore',
	description: 'Simple Variable Store library for EFI variable management',
	url: 'https://github.com/BigfootACA/svarstore',
	version: meson.project_version(),
)

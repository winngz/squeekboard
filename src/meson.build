sources = [
  'server-service.c',
  'server-context-service.c',
  'server-main.c',
  '../eek/eek.c',
  '../eek/eek-container.c',
  '../eek/eek-element.c',
  '../eek/eek-gtk-keyboard.c',
  '../eek/eek-gtk-renderer.c',
  '../eek/eek-key.c',
  '../eek/eek-keyboard.c',
  '../eek/eek-keyboard-drawing.c',
  '../eek/eek-keysym.c',
  '../eek/eek-layout.c',
  '../eek/eek-renderer.c',
  '../eek/eek-section.c',
  '../eek/eek-serializable.c',
  '../eek/eek-symbol.c',
  '../eek/eek-symbol-matrix.c',
  '../eek/eek-text.c',
  '../eek/eek-theme.c',
  '../eek/eek-theme-context.c',
  '../eek/eek-theme-node.c',
  '../eek/eek-types.c',
  '../eek/eek-xml-layout.c',
  enums,
  keysym_entries,
  marshalers,
  '../eekboard/key-emitter.c',
  '../eekboard/eekboard-context-service.c',
  '../eekboard/eekboard-context.c',
  '../eekboard/eekboard-service.c',
#  '../eekboard/eekboard-xklutil.c',
]

cc = meson.get_compiler('c')


deps = [
#  dependency('glib-2.0', version: '>=2.26.0'),
  dependency('gio-2.0', version: '>=2.26.0'),
  dependency('gtk+-3.0', version: '>=3.0'),
  dependency('libcroco-0.6'),
  cc.find_library('m'),
#  dependency('libxklavier'), # FIXME remove
]

# Replacement for eekboard-server
squeekboard = executable('squeekboard',
  sources,
  include_directories: [include_directories('..'), include_directories('../eek')],
  dependencies: deps,
  install: true,
  c_args: [
    '-DTHEMESDIR="' + pkgdatadir + '/themes"',
    '-DKEYBOARDSDIR="' + pkgdatadir + '/keyboards"',
    '-DEEKBOARD_COMPILATION=1',
    '-DEEK_COMPILATION=1'],
)

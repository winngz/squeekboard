if get_option('tests')

test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=gc-friendly,fatal-warnings',
  'GSETTINGS_BACKEND=memory',
  'MALLOC_CHECK_=2'
]

test_cflags = [
  '-I@0@/../src'.format(meson.current_source_dir()),
  '-I@0@/../src'.format(meson.current_build_dir()),
  '-DEEK_COMPILATION',
  '-DEEKBOARD_COMPILATION'
]

test_link_args = [
  '-fPIC',
]

tests = [
  'eek-simple-test',
  'eek-xml-test'
]

foreach name : tests

    test_sources = [name + '.c']

    t = executable(
        name,
        test_sources,
        squeekboard_resources,
        link_with: libsqueekboard,
        c_args : test_cflags,
        link_args: test_link_args,
        dependencies: deps,       # from src/meson.build
        include_directories: [
            include_directories('..'),
            include_directories('../eek')
        ]
    )

    test(name, t, env: test_env)

endforeach

endif
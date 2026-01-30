project(
    'vega',
    'cpp',
    version : '0.0.1',
    default_options : [
        'cpp_std=c++23',
        'warning_level=3',
        'buildtype=debugoptimized'
    ]
)

# Dependencies
vega_deps = []

if host_machine.system() == 'linux'
    # liburing
    vega_deps += dependency('liburing', fallback: 'liburing')
endif


vega_sources = files()
subdir('vega')


libvega = library('vega',
    vega_sources,
    include_directories : include_directories('.'),
    dependencies : vega_deps,
)


vega_dep = declare_dependency(
    link_with : libvega,
    include_directories : include_directories('.'),
    dependencies : vega_deps,
)


# Tests
subdir('test')

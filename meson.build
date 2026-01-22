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


vega_sources = files()
subdir('vega')


libvega = library('vega',
    vega_sources,
    include_directories : include_directories('.'),
)


vega_dep = declare_dependency(
    link_with : libvega,
    include_directories : include_directories('.')
)


# Tests
subdir('test')

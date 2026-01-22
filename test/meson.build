
test_env = environment()
test_env.prepend('PATH', meson.project_build_root())


test(
    'delay',
    executable(
        'delay',
        'delay.cc',
        dependencies: vega_dep
    ),
    env: test_env
)


test(
    'setTimeout',
    executable(
        'setTimeout',
        'setTimeout.cc',
        dependencies: vega_dep
    ),
    env: test_env
)

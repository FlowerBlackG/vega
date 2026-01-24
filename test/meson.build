
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


test(
    'blockedMain',
    executable(
        'blockedMain',
        'blockedMain.cc',
        dependencies: vega_dep
    ),
    env: test_env
)


test(
    'promiseCreate',
    executable(
        'promiseCreate',
        'promiseCreate.cc',
        dependencies: vega_dep
    ),
    env: test_env
)



test(
    'throws',
    executable(
        'throws',
        'throws.cc',
        dependencies: vega_dep
    ),
    env: test_env
)



test(
    'mockNio',
    executable(
        'mockNio',
        'mockNio.cc',
        dependencies: vega_dep
    ),
    env: test_env
)


test(
    'multipleWorkers',
    executable(
        'multipleWorkers',
        'multipleWorkers.cc',
        dependencies: vega_dep
    ),
    env: test_env
)


test(
    'promiseAll',
    executable(
        'promiseAll',
        'promiseAll.cc',
        dependencies: vega_dep
    ),
    env: test_env
)


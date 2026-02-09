
if host_machine.system() == 'linux'
    test(
        'ioUringHttpGet',
        executable(
            'ioUringHttpGet',
            'ioUringHttpGet.cc',
            dependencies: vega_dep
        ),
        env: test_env
    )
endif

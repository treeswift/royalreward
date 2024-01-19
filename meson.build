project('royalreward', 'cpp',
    version : '0.0.2',
    default_options : ['warning_level=3', 'cpp_std=c++14'])

add_global_arguments('-Wno-narrowing', language: 'cpp')

executable('gen',
            'algos/ground/aynrand.cpp',
            'algos/ground/choices.cpp',
            'algos/ground/genesis.cpp',
            'algos/ground/geometry.cpp',
            'algos/ground/geo_formation.cpp',
            'algos/ground/geo_landscape.cpp',
            'algos/ground/geo_power.cpp',
            'algos/ground/geo_wealth.cpp',
            'algos/ground/goldenkey.cpp',
            'algos/ground/maps.cpp',
            'algos/ground/gen.cpp',
            install : true)

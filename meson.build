project('royalreward', 'cpp',
    version : '0.0.2',
    default_options : ['warning_level=3', 'cpp_std=c++14'])

add_global_arguments('-Wno-narrowing', language: 'cpp')

subdir('algos/ground')
gen = static_library('gen', gen_files,
    include_directories: 'algos/ground',
)

executable('gen', files('backyard/gen.cpp'),
    include_directories: 'algos/ground',
    link_with: [gen],
    install : false)

subdir('algos/mission')
chr = static_library('chr', chr_files,
    include_directories: 'algos/mission',
)

subdir('modena')
executable('mod', mod_files,
    include_directories: [
        'algos/ground',
        'algos/mission',
    ],
    link_with: [gen, chr],
    install : true)

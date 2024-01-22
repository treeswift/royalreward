project('royalreward', 'cpp',
    version : '0.0.2',
    default_options : ['warning_level=3', 'cpp_std=c++17'])

add_global_arguments('-Wno-narrowing', language: 'cpp')
add_global_arguments('-Wno-char-subscripts', language: 'cpp')
add_global_arguments('-Wno-stringop-truncation', language: 'cpp')

subdir('algos/ground')
gen = static_library('gen', gen_files,
    include_directories: 'algos/ground',
)

executable('gen', files('backyard/gen.cpp'),
    include_directories: 'algos/ground',
    link_with: [gen],
    install : false)

ci = [
    'algos/ground',
    'algos/mission',
]

subdir('algos/mission')
chr = static_library('chr', chr_files,
    include_directories: ci,
)

subdir('modena')
executable('dat', dat_files,
    include_directories: ci,
    link_with: [gen, chr],
    install : true)

executable('mod', mod_files,
    include_directories: ci,
    link_with: [gen, chr],
    dependencies: dependency('boost', modules: ['system', 'filesystem']),
    install : true)

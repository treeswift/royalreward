project('royalreward', 'cpp',
    version : '0.0.2',
    default_options : ['warning_level=3', 'cpp_std=c++17'])

add_global_arguments('-Wno-narrowing', language: 'cpp')
add_global_arguments('-Wno-char-subscripts', language: 'cpp')
add_global_arguments('-Wno-stringop-truncation', language: 'cpp')

subdir('boring')
hlp = static_library('hlp', boring_files,
    include_directories: 'boring')

subdir('algos/ground')
gen = static_library('gen', gen_files,
    link_with: [hlp],
    include_directories: ['boring', 'algos/ground'],
)

executable('gen', files('backyard/gen.cpp'),
    include_directories: ['boring', 'algos/ground'],
    link_with: [gen],
    install : false)

ci = [
    'algos/ground',
    'algos/mission',
]

subdir('algos/mission')
war = static_library('war', war_files,
    include_directories: ['boring'] + ci,
)

dep = dependency('boost', modules: ['system', 'filesystem'])

subdir('modena')

old = static_library('old', old_files,
    include_directories: ['boring'] + ci,
)

incorporated = 'modena/3rdparty'
subdir(incorporated)
verbosity = get_option('debug') ? [] : [
        '-DDISABLE_PRINTF=1' # stdout is silenced, stderr isn't
]

upk = [
    static_library('unexepack', unexepack, cpp_args: verbosity),
    static_library('unexecomp', unexecomp, cpp_args: verbosity),
]

lib = [gen, war, old]

executable('dat', dat_files,
    include_directories: ci,
    link_with: lib,
    install : true)

executable('ham', ham_files,
    include_directories: ci,
    link_with: lib + upk,
    dependencies: dep,
    install : true)

executable('mod', mod_files,
    include_directories: ci + [incorporated],
    link_with: lib + upk,
    dependencies: dep,
    install : true)

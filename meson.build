project('liquid-resize', 'cpp',
        default_options : ['cpp_std=c++17'])

src = ['src/main.cpp', 'src/liquid-resize.cpp']
incdir = include_directories('include')
oiio = dependency('OpenImageIO')

executable('liquid-resize', 
            sources : src, 
            include_directories : incdir,
            dependencies : oiio)
project('liquid-resize', 'cpp')

src = ['src/main.cpp', 'src/liquid-resize.cpp']
incdir = include_directories('include')
oiio = dependency('OpenImageIO')

executable('liquid-resize', 
            sources : src, 
            include_directories : incdir,
            dependencies : oiio)
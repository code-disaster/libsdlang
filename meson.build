project('libsdlang', ['c'])

conf_data = configuration_data()

lib = shared_library(
  'sdlang',
  [
    'sdlang.c'
  ],
  install: true
)

install_headers('sdlang.h')

pkg = import('pkgconfig')
pkg.generate(
  libraries: ['-lsdlang'],
  subdirs: [],
  version: '0.1.0',
  name: 'sdlang',
  filebase: 'sdlang',
  description: 'A compact C library to parse SDLang (http://sdlang.org/) text.'
)

libsdlang_d = declare_dependency(link_with: lib, include_directories: include_directories('.'))

executable(
  'test-parser',
  [
    'samples/parser.c'
  ],
  install: false,
  dependencies: [
    libsdlang_d
  ]
)
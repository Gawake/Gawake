subdir('database-connection')

gawake_sources = [
  'main.c',
  'gawake-application.c',
  'gawake-window.c',
  'rule-row.c',
  'rule-setup-dialog.c',
  'rule-setup-dialog-add.c',
  'rule-setup-dialog-edit.c',
  'rule-face.c',
  'days-row.c'
]

gawake_sources += database_connection_sources

gawake_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.2') # originally 1.4
]

gawake_deps += database_connection_deps

gawake_sources += gnome.compile_resources('gawake-resources',
  'gawake.gresource.xml',
  c_name: 'gawake'
)

executable('gawake', gawake_sources,
  dependencies: gawake_deps,
       install: true,
)
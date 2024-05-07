project('gtk-vulkan-ui', 'c',
    default_options: ['c_std=gnu2x'])

gtk_dep = dependency('gtk4')
gtkwin32 = dependency('gtk4-win32')
vulkan_dep = dependency('vulkan')
executable('gtk-vulkan-ui', 'gtk-vulkan-ui.c', 'vulkan-test.c',
    c_args: ['-ftrivial-auto-var-init=zero'],
    dependencies: [gtk_dep,vulkan_dep,gtkwin32])
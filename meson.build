project('mauikit-terminal', 'cpp', version: '4.0.5', meson_version: '>=1.0.0', default_options: ['cpp_std=c++20', 'buildtype=release'])

cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.get_id() == 'gcc' and cpp_compiler.has_argument('-Wno-sfinae-incomplete')
    add_project_arguments('-Wno-sfinae-incomplete', language: 'cpp')
endif
qt6 = import('qt6')
qt = dependency('qt6', version: '>=6.4', modules: ['Core', 'Core5Compat', 'Gui', 'Quick', 'Qml', 'Widgets'])
qt_moc_args = ['-I' + qt.get_variable('includedir')]
kf6_core = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KCoreAddons'), dependencies: [cpp_compiler.find_library('KF6CoreAddons')])
kf6_config = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KConfig', get_option('prefix') / 'include' / 'KF6' / 'KConfigCore'), dependencies: [cpp_compiler.find_library('KF6ConfigCore')])
kf6_i18n = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KI18n'), dependencies: [cpp_compiler.find_library('KF6I18n')])
kf6_pty = declare_dependency(include_directories: include_directories(get_option('prefix') / 'include' / 'KF6' / 'KPty'), dependencies: [cpp_compiler.find_library('KF6Pty')])
mauikit = dependency('MauiKit4', version: '>=4.0.5')
cpp_args = host_machine.cpu_family() == 'x86_64' ? ['-march=x86-64-v3'] : []
export_header = configure_file(
    input: 'meson-content.in',
    output: 'terminal_export.h',
    configuration: {'CONTENT': ('#pragma once\n#define TERMINAL_EXPORT __attribute__((visibility("default")))\n')}
)
terminal_version_header = configure_file(
    input: 'meson-content.in',
    output: 'terminal_version.h',
    configuration: {'CONTENT': ('''#pragma once
#define Terminal_VERSION_STRING "4.0.5"
#define GIT_BRANCH "Stable"
#define GIT_COMMIT_HASH "4.0.5"
''')}
)
lib_sources = run_command('find', 'src/lib', '-type', 'f', '-name', '*.cpp', check: true).stdout().strip().split('\n')
terminal_moc = qt6.preprocess(moc_extra_arguments: qt_moc_args, moc_headers: ['src/lib/ColorScheme.h', 'src/lib/Emulation.h', 'src/lib/Filter.h', 'src/lib/HistorySearch.h', 'src/lib/Pty.h', 'src/lib/ScreenWindow.h', 'src/lib/Session.h', 'src/lib/TerminalDisplay.h', 'src/lib/Vt102Emulation.h', 'src/lib/customcolorscheme.h', 'src/lib/ksession.h'])
terminal = shared_library('MauiKitTerminal4', files('src/code/moduleinfo.cpp') + lib_sources + terminal_moc + [export_header, terminal_version_header], dependencies: [qt, kf6_core, kf6_config, kf6_i18n, kf6_pty, mauikit], include_directories: include_directories('src', 'src/lib'), cpp_args: cpp_args, version: '4.0.5', soversion: '4', install: true)
plugin_sources = qt6.preprocess(moc_extra_arguments: qt_moc_args, moc_headers: ['src/code/terminal_plugin.h', 'src/code/colorschemesmodel.h'])
plugin = shared_module('MauiKitTerminal4plugin', ['src/code/terminal_plugin.cpp', 'src/code/colorschemesmodel.cpp'] + plugin_sources, link_with: terminal, dependencies: [qt, kf6_core, kf6_i18n, mauikit], include_directories: include_directories('src', 'src/lib'), install: true, install_dir: get_option('libdir') / 'qt6/qml/org/mauikit/terminal')
qml_dir = get_option('libdir') / 'qt6/qml/org/mauikit/terminal'
install_data(configure_file(
    input: 'meson-content.in',
    output: 'qmldir',
    configuration: {'CONTENT': ('module org.mauikit.terminal\ndepends org.mauikit.controls\nplugin MauiKitTerminal4plugin\n')}
), install_dir: qml_dir)
install_subdir('src/controls.6', install_dir: qml_dir, strip_directory: true)
install_subdir('src/lib/color-schemes', install_dir: qml_dir)
install_subdir('src/lib/kb-layouts', install_dir: qml_dir)
install_headers(files('src/code/moduleinfo.h') + [export_header, terminal_version_header], subdir: 'MauiKit4/Terminal')
pkg = import('pkgconfig')
pkg.generate(terminal, name: 'MauiKit Terminal 4', filebase: 'MauiKitTerminal4', version: meson.project_version(), description: 'MauiKit terminal library', requires: 'MauiKit4 Qt6Core Qt6Core5Compat Qt6Gui Qt6Qml Qt6Quick Qt6Widgets')

cmake_targets = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitTerminal4Targets.cmake',
    configuration: {'CONTENT': (('''get_filename_component(_MAUIKIT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
if(NOT TARGET MauiKitTerminal4)
  add_library(MauiKitTerminal4 SHARED IMPORTED)
  set_target_properties(MauiKitTerminal4 PROPERTIES
    IMPORTED_LOCATION "${_MAUIKIT_PREFIX}/@LIBDIR@/libMauiKitTerminal4.so.4.0.5"
    INTERFACE_INCLUDE_DIRECTORIES "${_MAUIKIT_PREFIX}/include/MauiKit4/Terminal"
    INTERFACE_LINK_LIBRARIES "Qt6::Core;Qt6::Core5Compat;Qt6::Gui;Qt6::Qml;Qt6::Quick;Qt6::Widgets;KF6::ConfigCore;KF6::ConfigGui;KF6::CoreAddons;KF6::I18n;KF6::Pty;MauiKit4")
endif()
if(NOT TARGET MauiKit4::Terminal)
  add_library(MauiKit4::Terminal ALIAS MauiKitTerminal4)
endif()
if(NOT TARGET MauiKit::Terminal)
  add_library(MauiKit::Terminal ALIAS MauiKitTerminal4)
endif()
''')).replace('@LIBDIR@', (get_option('libdir')))}
)
cmake_config = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitTerminal4Config.cmake',
    configuration: {'CONTENT': ('''include(CMakeFindDependencyMacro)
find_dependency(Qt6 6.4 COMPONENTS Core Core5Compat Gui Qml Quick Widgets)
find_dependency(KF6 6.13.0 COMPONENTS Config CoreAddons I18n Pty)
find_dependency(MauiKit4 4.0.5 CONFIG)
include("${CMAKE_CURRENT_LIST_DIR}/MauiKitTerminal4Targets.cmake")
set(MauiKitTerminal4_FOUND TRUE)
''')}
)
cmake_version = configure_file(
    input: 'meson-content.in',
    output: 'MauiKitTerminal4ConfigVersion.cmake',
    configuration: {'CONTENT': (('''set(PACKAGE_VERSION "@VERSION@")
if(PACKAGE_FIND_VERSION VERSION_LESS_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_EXACT TRUE)
endif()
''')).replace('@VERSION@', (meson.project_version()))}
)
install_data([cmake_targets, cmake_config, cmake_version], install_dir: get_option('libdir') / 'cmake' / 'MauiKitTerminal4')

translation_domain = 'mauikitterminal'
msgfmt = find_program('msgfmt')
po_files = run_command('find', 'po', '-mindepth', '2', '-maxdepth', '2', '-type', 'f', '-name', translation_domain + '.po', check: true).stdout().strip().split('\n')
foreach po_file : po_files
    if po_file != ''
        locale = po_file.split('/')[1]
        custom_target(
            'translation-' + locale,
            input: po_file,
            output: locale / (translation_domain + '.mo'),
            command: [msgfmt, '--check', '-o', '@OUTPUT@', '@INPUT@'],
            install: true,
            install_dir: get_option('localedir') / locale / 'LC_MESSAGES',
            install_tag: 'i18n')
    endif
endforeach

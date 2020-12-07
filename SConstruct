# -*- mode:python -*-

import platform
import os
import shutil
import sys

import SCons.Errors

version = "2.0.4"
abi_major = 3
abi_minor = 0

cflags = '-g -Wall'

slmsource = [
    'src/slm/ids2ngram/ids2ngram.cpp',
    'src/slm/ids2ngram/idngram_merge.cpp',
    'src/slm/mmseg/mmseg.cpp',
    'src/slm/tslminfo/tslminfo.cpp',
    'src/slm/slm.cpp',
    'src/slm/slminfo/slminfo.cpp',
    'src/slm/slmpack/arpa_slm.cpp',
    'src/slm/slmpack/slmpack.cpp',
    'src/slm/sim_sen.cpp',
    'src/slm/sim_slm.cpp',
    'src/slm/getwordfreq/getwordfreq.cpp',
    'src/slm/slmseg/slmseg.cpp',
    'src/slm/thread/slmthread.cpp',
    'src/slm/thread/test_vc.cpp',
    'src/slm/thread/ValueCompress.cpp',
    'src/slm/slmbuild/slmbuild.cpp',
    'src/slm/slmprune/slmprune.cpp',
    'src/slm/slmbuild/sim_slmbuilder.cpp',
    'src/slm/tslmendian/slm_endian.cpp',
    'src/slm/tslmendian/writer.cpp',
    'src/slm/tslmendian/slm_file.cpp',
    'src/slm/sim_dict.cpp',
    'src/portability.cpp',
    'src/lexicon/trie_writer.cpp',
    'src/lexicon/genpyt.cpp',
    'src/lexicon/pytrie_gen.cpp',
    'src/lexicon/pytrie.cpp',
    'src/pinyin/pinyin_data.cpp',
    ]

imesource = [
    'src/portability.cpp',
    'src/slm/slm.cpp',
    'src/lexicon/pytrie.cpp',
    'src/pinyin/pinyin_data.cpp',
    'src/pinyin/pinyin_seg.cpp',
    'src/pinyin/shuangpin_data.cpp',
    'src/pinyin/shuangpin_seg.cpp',
    'src/pinyin/hunpin_seg.cpp',
    'src/ime-core/imi_context.cpp',
    'src/ime-core/imi_data.cpp',
    'src/ime-core/lattice_states.cpp',
    'src/ime-core/imi_view.cpp',
    'src/ime-core/imi_uiobjects.cpp',
    'src/ime-core/imi_view_classic.cpp',
    'src/ime-core/imi_winHandler.cpp',
    'src/ime-core/ic_history.cpp',
    'src/ime-core/imi_funcobjs.cpp',
    'src/ime-core/imi_options.cpp',
    'src/ime-core/imi_option_event.cpp',
    'src/ime-core/userdict.cpp',
    ]

headers = [
    'src/slm/ids2ngram/idngram.h',
    'src/slm/ids2ngram/idngram_merge.h',
    'src/slm/slm.h',
    'src/slm/sim_dict.h',
    'src/slm/sim_sen.h',
    'src/slm/sim_slm.h',
    'src/slm/thread/ValueCompress.h',
    'src/slm/sim_fmerge.h',
    'src/slm/slmbuild/sim_slmbuilder.h',
    'src/slm/slmpack/arpa_slm.h',
    'src/slm/slmpack/common.h',
    'src/slm/tslmendian/slm_file.h',
    'src/slm/tslmendian/writer.h',
    'src/lexicon/pytrie_gen.h',
    'src/lexicon/trie_writer.h',
    'src/lexicon/pytrie.h',
    'src/ime-core/imi_view_classic.h',
    'src/ime-core/imi_uiobjects.h',
    'src/ime-core/lattice_states.h',
    'src/ime-core/ic_history.h',
    'src/ime-core/imi_funcobjs.h',
    'src/ime-core/imi_context.h',
    'src/ime-core/imi_winHandler.h',
    'src/ime-core/imi_glibHandler.h',
    'src/ime-core/userdict.h',
    'src/ime-core/imi_option_event.h',
    'src/ime-core/imi_data.h',
    'src/ime-core/utils.h',
    'src/ime-core/imi_keys.h',
    'src/ime-core/imi_option_keys.h',
    'src/ime-core/imi_options.h',
    'src/ime-core/imi_defines.h',
    'src/ime-core/imi_view.h',
    'src/portability.h',
    'src/pinyin/segmentor.h',
    'src/pinyin/shuangpin_seg.h',
    'src/pinyin/datrie.h',
    'src/pinyin/quanpin_trie.h',
    'src/pinyin/pinyin_seg.h',
    'src/pinyin/pinyin_data.h',
    'src/pinyin/syllable.h',
    'src/pinyin/shuangpin_data.h',
    'src/pinyin/hunpin_seg.h',
    'src/pinyin/datrie_impl.h',
    'src/sunpinyin.h',
    ]

# source of plugin module, it's off by default
# and this module depends on Python
imesource_plugin = [
    'src/ime-core/imi_plugin.cpp',
    ]

headers_plugin = [
    'src/ime-core/imi_plugin.h',
    ]

bins = [
    'src/mmseg',
    'src/slmseg',
    'src/ids2ngram',
    'src/idngram_merge',
    'src/slmbuild',
    'src/slmprune',
    'src/slminfo',
    'src/slmthread',
    'src/tslmendian',
    'src/tslminfo',
    'src/slmpack',
    'src/genpyt',
    'src/getwordfreq',
    'src/sunpinyin-dictgen',
    ]

man1s = [
    'man/mmseg.1',
    'man/slmseg.1',
    'man/ids2ngram.1',
    'man/idngram_merge.1',
    'man/slmbuild.1',
    'man/slmprune.1',
    'man/slminfo.1',
    'man/slmthread.1',
    'man/tslmendian.1',
    'man/tslminfo.1',
    'man/slmpack.1',
    'man/genpyt.1',
    'man/getwordfreq.1',
    ]

docs = [
    'doc/README',
    'doc/SLM-train.mk',
    'doc/SLM-inst.mk',
    ]

# options
AddOption('--prefix', dest='prefix', metavar='DIR',
          help='installation prefix')

AddOption('--libdir', dest='libdir', metavar='DIR',
          help='installation libdir')

AddOption('--datadir', dest='datadir', metavar='DIR',
          help='installation data dir')

AddOption('--rpath', dest='rpath', metavar='DIR',
          help='encode rpath in the executables')

AddOption('--enable-plugins', dest='enable_plugins', action='store_true',
          default=False, help='enable plugin mechanism at libsunpinyin layer')

AddOption('--disable-plugins', dest='enable_plugins', action='store_false',
          default=False, help='disable plugin mechanism at libsunpinyin layer')


def PathIsExecutable(key, path, env):
    if shutil.which(path) is None:
        m = "path '{path}' for option '{key}' is not found or not in $PATH"
        raise SCons.Errors.UserError(m.format(key=key, path=path))

# save the options
opts = Variables('configure.conf')
opts.Add('PREFIX', default='/usr/local')
opts.Add('LIBDIR', default='/usr/local/lib')
opts.Add('DATADIR', default='/usr/local/share')
opts.Add('ENABLE_PLUGINS', default=False)
opts.Add(PathVariable('PYTHON',
                      'python3 interpreter used to generate header files',
                      '/usr/bin/python3',
                      PathIsExecutable))

#
# ==============================environment==============================
#
#
def allinc():
    return [root for root, _, _ in os.walk('src')]


def GetOS():
    return platform.uname()[0]


def CreateEnvironment():
    make = 'make'
    wget = 'wget'
    w3m = 'wget -q -O -'
    tar = 'tar'
    if GetOS() == 'Darwin':
        wget = 'curl -O'
        w3m = 'curl -s'
    elif GetOS() == 'FreeBSD':
        make = 'gmake'
        wget = 'fetch'
        w3m = 'fetch -o -'
    elif GetOS() == 'SunOS':
        make = 'gmake'
        tar = 'gtar'
    libln_builder = Builder(action='cd ${TARGET.dir} && ln -s ${SOURCE.name} ${TARGET.name}')
    env = Environment(ENV=os.environ, CFLAGS=cflags, CXXFLAGS='-std=c++11',
                      MAKE=make, WGET=wget, W3M=w3m, TAR=tar,
                      CPPPATH=['.'] + allinc(),
                      tools=['default', 'textfile'])
    env.Append(BUILDERS={'InstallAsSymlink': libln_builder})
    env['ENDIANNESS'] = "be" if sys.byteorder == "big" else "le"
    return env


def PassVariables(envvar, env):
    for (x, y) in envvar:
        if x in os.environ:
            print('Warning: you\'ve set %s in the environmental variable!' % x)
            env[y] = os.environ[x]

env = CreateEnvironment()
opts.Update(env)

if GetOption('prefix') is not None:
    env['PREFIX'] = GetOption('prefix')
    env['LIBDIR'] = os.path.join(env['PREFIX'], 'lib')
    env['DATADIR'] = os.path.join(env['PREFIX'], 'share')

if GetOption('libdir') is not None:
    env['LIBDIR'] = GetOption('libdir')

if GetOption('datadir') is not None:
    env['DATADIR'] = GetOption('datadir')

env['ENABLE_PLUGINS'] = GetOption('enable_plugins')

opts.Save('configure.conf', env)

bindir = os.path.join(env['PREFIX'], 'bin')
mandir = os.path.join(env['PREFIX'], 'share/man')
man1dir = os.path.join(mandir, 'man1')
docdir = os.path.join(env['PREFIX'], 'share/doc/sunpinyin')
headersdir = os.path.join(env['PREFIX'], 'include/sunpinyin-2.0')
datadir = os.path.join(env['DATADIR'], 'sunpinyin')
libdir = env['LIBDIR']

# pass through environmental variables
envvar = [('CC', 'CC'),
          ('CXX', 'CXX'),
          ('CPPFLAGS', 'CPPFLAGS'),
          ('CFLAGS', 'CFLAGS'),
          ('CXXFLAGS', 'CXXFLAGS'),
          ('LDFLAGS', 'LINKFLAGS'),
          ('TAR', 'TAR'),
          ('MAKE', 'MAKE'),
          ('WGET', 'WGET')]
PassVariables(envvar, env)

# append the source and headers
if env['ENABLE_PLUGINS']:
    imesource += imesource_plugin
    headers += headers_plugin

# merge some of critical compile flags
env.MergeFlags(['-pipe -DHAVE_CONFIG_H',
                '-DSUNPINYIN_DATA_DIR=\\\'\\"%s\\"\\\'' % datadir])

if GetOption('rpath') is not None and GetOS() != 'Darwin':
    env.MergeFlags('-Wl,-R -Wl,%s' % GetOption('rpath'))


#
# ==============================configure================================
#
def CheckPKGConfig(context, version='0.12.0'):
    context.Message('Checking for pkg-config... ')
    ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
    context.Result(ret)
    return ret


def CheckPKG(context, name):
    context.Message('Checking for %s... ' % name)
    ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
    context.Result(ret)
    return ret


def CheckPython(context):
    context.Message('Checking for Python library...')
    ret = context.TryAction('python3-config --prefix')[0]
    context.Result(ret)
    if ret:
        context.env.MergeFlags(['!python3-config --includes',
                                '!python3-config --libs'])
    return ret


def AppendEndianCheck(conf):
    conf.config_h_text += r'''

#if defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
# define WORDS_BIGENDIAN 1

#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
# undef WORDS_BIGENDIAN

#elif defined(__sparc) || defined(__sparc__) \
  || defined(_POWER)   || defined(__powerpc__) \
  || defined(__ppc__)  || defined(__hpux) || defined(__hppa) \
  || defined(_MIPSEB)  || defined(_POWER) \
  || defined(__s390__) || (defined(__sh__) && defined(__BIG_ENDIAN__)) \
  || defined(__AARCH64EB__) \
  || defined(__m68k__)
# define WORDS_BIGENDIAN 1

#elif defined(__i386__) || defined(__i386) \
  || defined(__ia64)    || defined(__ia64__) \
  || defined(_M_IX86)   || defined(_M_IA64) \
  || defined(_M_ALPHA)  || defined(__amd64) \
  || defined(__amd64__) || defined(_M_AMD64) \
  || defined(__x86_64)  || defined(__x86_64__) \
  || defined(_M_X64)    || defined(__bfin__) \
  || defined(__alpha__) || defined(__ARMEL__) \
  || defined(_MIPSEL)   || (defined(__sh__) && defined(__LITTLE_ENDIAN__)) \
  || defined(__riscv) \
  || defined(__AARCH64EL__)
# undef WORDS_BIGENDIAN

#else
# error can not detect the endianness!
#endif
'''

conf = env.Configure(clean=False, help=False, config_h='config.h',
                     custom_tests={'CheckPKGConfig': CheckPKGConfig,
                                   'CheckPKG': CheckPKG,
                                   'CheckPython': CheckPython})


def DoConfigure():
    if GetOS() == 'Darwin':
        if not conf.CheckLibWithHeader('sqlite3', 'sqlite3.h', 'C'):
            Exit(1)
        if not conf.CheckLibWithHeader('iconv', 'iconv.h', 'C'):
            Exit(1)
    else:
        if not conf.CheckPKGConfig():
            Exit(1)
        if not conf.CheckPKG('sqlite3'):
            Exit(1)

    if conf.env['ENABLE_PLUGINS']:
        if not conf.CheckPython():
            Exit(1)
        conf.Define('ENABLE_PLUGINS')

    conf.Define('ENABLE_NLS', 1)
    conf.Define('GETTEXT_PACKAGE', '"sunpinyin2"')
    conf.CheckCHeader('assert.h')
    conf.CheckFunc('bind_textdomain_codeset')
    conf.CheckFunc('dcgettext')
    conf.CheckFunc('exp2')
    conf.CheckCHeader('fcntl.h')
    conf.CheckCHeader('getopt.h')
    conf.CheckFunc('getopt_long')
    conf.CheckFunc('getpagesize')
    conf.CheckFunc('get_opt')
    conf.CheckCHeader('iconv.h')
    conf.CheckCHeader('inttypes.h')
    conf.CheckCHeader('locale.h')
    conf.CheckCHeader('libintl.h')
    conf.CheckCHeader('limits.h')
    conf.CheckCHeader('locale.h')
    conf.CheckFunc('log2')
    conf.CheckCHeader('memory.h')
    conf.CheckFunc('memset')
    conf.CheckFunc('mmap')
    conf.CheckFunc('munmap')
    conf.CheckFunc('setlocale')
    conf.CheckCHeader('sys/mman.h')
    conf.CheckCHeader('sys/param.h')
    conf.CheckCHeader('sys/stat.h')
    conf.CheckCHeader('sys/types.h')
    conf.CheckCHeader('unistd.h')
    conf.CheckCHeader('wchar.h')

    # add essential package requirements
    conf.Define('PACKAGE', '"sunpinyin"')
    conf.Define('PACKAGE_NAME', '"sunpinyin"')
    conf.Define('PACKAGE_STRING', '"sunpinyin 2.0"')
    conf.Define('PACKAGE_TARNAME', '"sunpinyin"')
    conf.Define('PACKAGE_VERSION', '"2.0"')
    conf.Define('VRESION', '"2.0"')

    # append endianness checking defines
    AppendEndianCheck(conf)
    env = conf.Finish()

    if GetOS() != 'Darwin':
        env.ParseConfig('pkg-config sqlite3 --libs --cflags')

if not GetOption('clean') and not GetOption('help'):
    DoConfigure()

#
# ==============================compile==============================
#
env.Object(slmsource)
env.Command('src/pinyin/quanpin_trie.h', 'python/quanpin_trie_gen.py',
            '$PYTHON $SOURCE > $TARGET')
env.Command('src/pinyin/pinyin_info.h', 'python/pinyin_info_gen.py',
            '$PYTHON $SOURCE > $TARGET')

SConscript(['src/SConscript', 'man/SConscript', 'doc/SConscript'], exports='env')

env.Substfile('sunpinyin-2.0.pc.in', SUBST_DICT={
    '@PREFIX@': env['PREFIX'],
    '@LIBDIR@': env['LIBDIR'],
    '@VERSION@': version,
    '@CFLAGS@': ' '.join(['-I$${includedir}' + x[3:] for x in sorted(allinc())]),
})

libname_default = '%ssunpinyin%s' % (env.subst('${SHLIBPREFIX}'),
                                     env.subst('${SHLIBSUFFIX}'))
libname_link = libname_default
libname_soname = '%s.%d' % (libname_link, abi_major)
libname = '%s.%d' % (libname_soname, abi_minor)
lib = None

if GetOS() != 'Darwin':
    lib = env.SharedLibrary(libname, SHLIBSUFFIX='', source=imesource,
                            parse_flags='-Wl,-soname=%s' % libname_soname)
else:
    # TODO: add install_name on Darwin?
    lib = env.SharedLibrary('sunpinyin', source=imesource)


def DoInstall():
    lib_target = None
    if GetOS() == 'Darwin':
        lib_target = env.Install(libdir, lib)
    else:
        lib_target_bin = env.Install(libdir, lib)
        # where does it goes
        install_path = os.path.dirname(str(lib_target_bin[0]))
        lib_target = [
            lib_target_bin,
            env.InstallAsSymlink(os.path.join(install_path, libname_soname),
                                 lib_target_bin),
            env.InstallAsSymlink(os.path.join(install_path, libname_link),
                                 lib_target_bin),
            ]

    lib_pkgconfig_target = env.Install(os.path.join(libdir, 'pkgconfig'),
                                       ['sunpinyin-2.0.pc'])
    bin_target = env.Install(bindir, bins)
    man1_target = env.Install(man1dir, man1s)
    doc_target = env.Install(docdir, docs)
    header_targets = []
    for header in headers:
        header_targets.append(env.InstallAs(headersdir + header[3:], header))
    env.Alias('install-bin', bin_target)
    env.Alias('install-man1', man1_target)
    env.Alias('install-doc', doc_target)
    env.Alias('install-headers', header_targets)
    env.Alias('install-lib', lib_target + [lib_pkgconfig_target])
    Mkdir(datadir)

DoInstall()
env.Alias('install', [
    'install-bin', 'install-man1', 'install-doc', 'install-headers', 'install-lib'
])

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

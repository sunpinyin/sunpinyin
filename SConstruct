import platform
import os
import sys

version="2.0.3"
abi_major = 3
abi_minor = 0

cflags='-O2 -g -pipe '

slmsource=['src/slm/ids2ngram/ids2ngram.cpp',
           'src/slm/ids2ngram/idngram_merge.cpp',
           'src/slm/mmseg/mmseg.cpp',
           'src/slm/tslminfo/tslminfo.cpp',
           'src/slm/tslmpack/arpa_slm.cpp',
           'src/slm/tslmpack/arpa_conv.cpp',
           'src/slm/tslmpack/slmpack.cpp',
           'src/slm/slm.cpp',
           'src/slm/slminfo/slminfo.cpp',
           'src/slm/sim_sen.cpp',
           'src/slm/sim_slm.cpp',
           'src/slm/getWordFreq/getWordFreq.cpp',
           'src/slm/slmseg/slmseg.cpp',
           'src/slm/thread/slmthread.cpp',
           'src/slm/thread/test_vc.cpp',
           'src/slm/thread/ValueCompress.cpp',
           'src/slm/slmbuild/slmbuild.cpp',
           'src/slm/slmprune/slmprune.cpp',
           'src/slm/sim_slmbuilder.cpp',
           'src/slm/tslmendian/slm_endian.cpp',
           'src/slm/tslmendian/writer.cpp',
           'src/slm/tslmendian/slm_file.cpp',
           'src/slm/sim_dict.cpp',
           'src/portability.cpp',
           'src/lexicon/trie_writer.cpp',
           'src/lexicon/genPYT.cpp',
           'src/lexicon/pytrie_gen.cpp',
           'src/lexicon/pytrie.cpp',
           'src/pinyin/pinyin_data.cpp']

imesource=['src/portability.cpp',
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
           'src/ime-core/userdict.cpp']

headers=['src/slm/ids2ngram/idngram.h',
         'src/slm/ids2ngram/idngram_merge.h',
         'src/slm/slm.h',
         'src/slm/tslmpack/arpa_slm.h',
         'src/slm/tslmpack/common.h',
         'src/slm/tslmpack/arpa_conv.h',
         'src/slm/sim_dict.h',
         'src/slm/sim_sen.h',
         'src/slm/sim_slm.h',
         'src/slm/thread/ValueCompress.h',
         'src/slm/sim_fmerge.h',
         'src/slm/sim_slmbuilder.h',
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
         'src/sunpinyin.h']

# options
AddOption('--prefix', dest='prefix', metavar='DIR',
          help='installation prefix')

AddOption('--libdir', dest='libdir', metavar='DIR',
          help='installation libdir')

AddOption('--libdatadir', dest='libdatadir', metavar='DIR',
          help='installation libdata dir')

AddOption('--rpath', dest='rpath', metavar='DIR',
          help='encode rpath in the executables')

# save the options
opts = Variables('configure.conf')
opts.Add('PREFIX', default='/usr/local')
opts.Add('LIBDIR', default='/usr/local/lib')
opts.Add('LIBDATADIR', default='/usr/local/lib')

#
#==============================environment==============================
#
#
def allinc():
    inc=[]
    for root, dirs, files in os.walk('src'):
        inc.append(root)
    return inc

def GetOS():
    return platform.uname()[0]

def CreateEnvironment():
    tar = 'tar'
    make = 'make'
    wget = 'wget'
    if GetOS() == 'Darwin':
        wget = 'curl -O'
    elif GetOS() == 'FreeBSD':
        wget = 'fetch'
        make = 'gmake'
    elif GetOS() == 'SunOS':
        tar = 'gtar'
        make = 'gmake'

    return Environment(ENV=os.environ, CFLAGS=cflags, CXXFLAGS=cflags,
                       TAR=tar, MAKE=make, WGET=wget,
                       CPPPATH=['.'] + allinc())

def PassVariables(envvar, env):
    for (x, y) in envvar:
        if x in os.environ:
            print 'Warning: you\'ve set %s in the environmental variable!' % x
            env[y] = os.environ[x]
    
env = CreateEnvironment()
opts.Update(env)

if GetOption('prefix') is not None:
    env['PREFIX'] = GetOption('prefix')
    env['LIBDATADIR'] = env['PREFIX'] + '/lib'
    env['LIBDIR'] = env['PREFIX'] + '/lib'

if GetOption('libdir') is not None:
    env['LIBDIR'] = GetOption('libdir')

if GetOption('libdatadir') is not None:
    env['LIBDATADIR'] = GetOption('libdatadir')

opts.Save('configure.conf', env)

libdir = env['LIBDIR']
libdatadir = env['LIBDATADIR'] + '/sunpinyin/data'
headersdir = env['PREFIX'] + '/include/sunpinyin-2.0'

if GetOS() != 'Darwin':
    env.Append(LINKFLAGS=['-Wl,-soname=libsunpinyin.so.%d' % abi_major])

if GetOption('rpath') is not None and GetOS() != 'Darwin':
    env.Append(LINKFLAGS='-Wl,-R -Wl,%s' % GetOption('rpath'))

# pass through environmental variables
envvar = [('CC', 'CC'),
          ('CXX', 'CXX'),
          ('CFLAGS', 'CFLAGS'),
          ('CXXFLAGS', 'CXXFLAGS'),
          ('LDFLAGS', 'LINKFLAGS'),
          ('TAR', 'TAR'),
          ('MAKE', 'MAKE'),
          ('WGET', 'WGET')]
PassVariables(envvar, env)

# append critical cflags
extra_cflags=' -DHAVE_CONFIG_H -DSUNPINYIN_DATA_DIR=\'"%s"\'' % libdatadir
env.Append(CFLAGS=extra_cflags)
env.Append(CXXFLAGS=extra_cflags)

#
#==============================configure================================
#
def CheckPKGConfig(context, version='0.12.0'):
    context.Message( 'Checking for pkg-config... ' )
    ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
    context.Result(ret)
    return ret

def CheckPKG(context, name):
    context.Message( 'Checking for %s... ' % name )
    ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
    context.Result(ret)
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
  || defined(__s390__)
# define WORDS_BIGENDIAN 1

#elif defined(__i386__) || defined(__i386) \
  || defined(__ia64)    || defined(__ia64__) \
  || defined(_M_IX86)   || defined(_M_IA64) \
  || defined(_M_ALPHA)  || defined(__amd64) \
  || defined(__amd64__) || defined(_M_AMD64) \
  || defined(__x86_64)  || defined(__x86_64__) \
  || defined(_M_X64)    || defined(__bfin__) \
  || defined(__alpha__) || defined(__ARMEL__)
# undef WORDS_BIGENDIAN

#else
# error can not detect the endianness!
#endif
'''

conf = env.Configure(clean=False, help=False, config_h='config.h',
                     custom_tests={'CheckPKGConfig' : CheckPKGConfig,
                                   'CheckPKG' : CheckPKG})

def DoConfigure():
    if GetOption('clean'):
        return

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

    conf.Define('ENABLE_NLS', 1)
    conf.Define('GETTEXT_PACKAGE', '"sunpinyin2"')
    conf.CheckCHeader('assert.h')
    conf.CheckFunc('bind_textdomain_codeset')
    conf.CheckFunc('dcgettext')
    conf.CheckCHeader('dlfcn.h')
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
    conf.CheckFunc('strndup')
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
    # generate sunpinyin.pc
    f = file('sunpinyin-2.0.pc', 'w')
    content = (
        'prefix='+env['PREFIX'],
        'exec_prefix=${prefix}',
        'libdir=' + libdir,
        'includedir=${exec_prefix}/include/sunpinyin-2.0',
        '',
        'Name: libsunpinyin',
        'Description: IME library based on Statistical Language Model',
        'Version: %s' % version,
        'Requires: sqlite3',
        'Libs: -L${libdir} -lsunpinyin',
        'Cflags: ' + reduce(lambda a, b: a + ' ' + b,
                            map(lambda x: '-I${includedir}' + x[3:],
                                allinc()))
        )
    f.write(reduce(lambda a, b: a + '\n' + b, content))
    f.close()

    if GetOS() != 'Darwin':
        env.ParseConfig('pkg-config sqlite3 --libs --cflags')

DoConfigure()

#
#==============================compile==============================
#
env.Object(slmsource)

SConscript(['build/SConscript'], exports='env')

libname = 'libsunpinyin.so.%d.%d' % (abi_major, abi_minor)
libname_soname = 'libsunpinyin.so.%d' % abi_major
libname_link = 'libsunpinyin.so'

lib = env.SharedLibrary('sunpinyin-%d.%d' % (abi_major, abi_minor),
                        source=imesource)

env.Command('rawlm', 'build/tslmpack',
            '$MAKE -C raw WGET="$WGET" TAR="$TAR"')

env.Command('lm', 'rawlm',
            '$MAKE -C data WGET="$WGET" TAR="$TAR"')

if GetOption('clean'):
    os.environ['TAR'] = env['TAR']
    os.environ['MAKE'] = env['MAKE']
    os.system('$MAKE -C raw clean WGET="$WGET" TAR="$TAR"')
    os.system('$MAKE -C data clean WGET="$WGET" TAR="$TAR"')

def DoInstall():
    if not 'install' in COMMAND_LINE_TARGETS:
        return

    lib_target = None
    if GetOS() == 'Darwin':
        lib_target = env.Install(libdir, lib)
    else:
        lib_target_bin = env.InstallAs(libdir + '/' + libname, lib)
        install_path = os.path.dirname(str(lib_target_bin[0])) + '/'
        lib_target = [
            lib_target_bin,
            env.Command(install_path + libname_soname, lib_target_bin,
                        'cd %s && ln -sf %s %s' %
                        (install_path, libname, libname_soname)),
            env.Command(install_path + libname_link, lib_target_bin,
                        'cd %s && ln -sf %s %s' %
                        (install_path, libname, libname_link))
            ]

    lib_pkgconfig_target = env.Install(libdir+'/pkgconfig',
                                       ['sunpinyin-2.0.pc'])
    libdata_target = env.Install(libdatadir,
                                 ['data/lm_sc.t3g',
                                  'data/pydict_sc.bin'])
    header_targets = []
    for header in headers:
        header_targets.append(env.InstallAs(headersdir + header[3:], header))
    env.Alias('install-headers', header_targets)
    env.Alias('install-lib', lib_target + [lib_pkgconfig_target])
    env.Alias('install-libdata', libdata_target)

DoInstall()
env.Alias('install', ['install-lib', 'install-libdata', 'install-headers'])


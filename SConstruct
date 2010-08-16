import platform
import os
import sys

cflags='-O2 -pipe -DHAVE_CONFIG_H '
prefix='/usr/local'
rpath=''

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
         'src/pinyin/datrie_impl.h',
         'src/host_os.h',
         'src/sunpinyin.h']

AddOption('--prefix', dest='prefix', type='string', nargs=1,
          action='store', metavar='DIR', help='installation prefix')
AddOption('--rpath', dest='rpath', type='string', nargs=1,
          action='store', metavar='DIR', help='encode rpath in the executables')

if GetOption('prefix') is not None:
    prefix = GetOption('prefix')
if GetOption('rpath') is not None:
    rpath = GetOption('rpath')

cflags += ('-DSUNPINYIN_DATA_DIR=\'"%s/lib/sunpinyin/data"\'' % (prefix,))
libdir = prefix+'/lib'
libdatadir = libdir+'/sunpinyin/data'
headersdir = prefix+'/include/sunpinyin-2.0'

abi_major = 3
abi_minor = 0

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
    elif GetOS() == 'SunOS':
        tar = 'gtar'
        make = 'gmake'

    return Environment(ENV=os.environ, CFLAGS=cflags, CXXFLAGS=cflags,
                       TAR=tar, MAKE=make, WGET=wget,
                       CPPPATH=['.'] + allinc(), PREFIX=prefix)

env = CreateEnvironment()

if GetOS() != 'Darwin':
    env.Append(LINKFLAGS=['-Wl,-soname=libsunpinyin.so.%d' % abi_major])

if rpath != '' and GetOS() != 'Darwin':
    env.Append(LINKFLAGS='-Wl,-R -Wl,%s' % rpath)

if 'CC' in os.environ:
    print 'Warning: you\'ve set %s as C compiler' % os.environ['CC']
    env['CC']=os.environ['CC']
    
if 'CXX' in os.environ:
    print 'Warning: you\'ve set %s as C++ compiler' % os.environ['CXX']
    env['CXX']=os.environ['CXX']

if 'CFLAGS' in os.environ:
    print 'Warning: you\'ve set an external compiler flags for C.'
    env['CFLAGS'] = os.environ['CFLAGS']

if 'CXXFLAGS' in os.environ:
    print 'Warning: you\'ve set an external compiler flags for C++.'
    env['CXXFLAGS'] = os.environ['CXXFLAGS']

if 'TAR' in os.environ:
    print 'Warning: you\'ve set %s as tar' % os.environ['TAR']
    env['TAR'] = os.environ['TAR']

if 'MAKE' in os.environ:
    print 'Warning: you\'ve set %s as make' % os.environ['MAKE']
    env['MAKE'] = os.environ['MAKE']

if 'WGET' in os.environ:
    print 'Warning: you\'ve set %s as wget' % os.environ['WGET']
    env['WGET'] = os.environ['WGET']
    
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

conf = Configure(env, custom_tests={'CheckPKGConfig' : CheckPKGConfig,
                                    'CheckPKG' : CheckPKG})

config_h_content = ''

def AddConfigItem(macro_name, res):
    global config_h_content
    config_h_content += ('#define %s %s\n\n' % (macro_name, res))

def AddTestHeader(header):
    macro_name = header.replace('.', '_').replace('-', '_').replace('/', '_').upper()
    macro_name = 'HAVE_' + macro_name
    if conf.CheckCHeader(header):
        AddConfigItem(macro_name, 1)

def AddTestFunction(funcname):
    macro_name = funcname.upper()
    macro_name = 'HAVE_' + macro_name
    if conf.CheckFunc(funcname):
        AddConfigItem(macro_name, 1)

def LinkOSHeader():
    osstring = GetOS()
    header = ''
    if osstring == 'Linux':
        header = 'linux.h'
    elif osstring == 'SunOS':
        header = 'solaris.h'
    elif osstring == 'Darwin':
        header = 'mac.h'

    os.system('ln -sf ./config/%s ./src/host_os.h' % (header,));

def AppendEndianCheck():
    global config_h_content
    config_h_content += r'''
#if defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
# define WORDS_BIGENDIAN 1

#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
# undef WORDS_BIGENDIAN

#elif defined(__sparc) || defined(__sparc__) \
  || defined(_POWER) || defined(__powerpc__) \
  || defined(__ppc__) || defined(__hpux) || defined(__hppa) \
  || defined(_MIPSEB) || defined(_POWER) \
  || defined(__s390__)
# define WORDS_BIGENDIAN 1

#elif defined(__i386__) || defined(__alpha__) || defined(__i386) \
  || defined(__ia64) || defined(__ia64__) \
  || defined(_M_IX86) || defined(_M_IA64) \
  || defined(_M_ALPHA) || defined(__amd64) \
  || defined(__amd64__) || defined(_M_AMD64) \
  || defined(__x86_64) || defined(__x86_64__) \
  || defined(_M_X64) || defined(__bfin__)
# undef WORDS_BIGENDIAN

#else
# error can not detect the endianness!
#endif
'''

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

    AddConfigItem('ENABLE_NLS', 1)
    AddConfigItem('GETTEXT_PACKAGE', '"sunpinyin2"')
    AddTestHeader('assert.h')
    AddTestFunction('bind_textdomain_codeset')
    AddTestFunction('dcgettext')
    AddTestHeader('dlfcn.h')
    AddTestFunction('exp2')
    AddTestHeader('fcntl.h')
    AddTestHeader('getopt.h')
    AddTestFunction('getopt_long')
    AddTestFunction('getpagesize')
    AddTestFunction('get_opt')
    AddTestHeader('iconv.h')
    AddTestHeader('inttypes.h')
    AddTestHeader('locale.h')
    AddTestHeader('libintl.h')
    AddTestHeader('limits.h')
    AddTestHeader('locale.h')
    AddTestFunction('log2')
    AddTestHeader('memory.h')
    AddTestFunction('memset')
    AddTestFunction('mmap')
    AddTestFunction('munmap')
    AddTestFunction('setlocale')
    AddTestFunction('strndup')
    AddTestHeader('sys/mman.h')
    AddTestHeader('sys/param.h')
    AddTestHeader('sys/stat.h')
    AddTestHeader('sys/types.h')
    AddTestHeader('unistd.h')
    AddTestHeader('wchar.h')

    # add essential package requirements
    AddConfigItem('PACKAGE', '"sunpinyin"')
    AddConfigItem('PACKAGE_NAME', '"sunpinyin"')
    AddConfigItem('PACKAGE_STRING', '"sunpinyin 2.0"')
    AddConfigItem('PACKAGE_TARNAME', '"sunpinyin"')
    AddConfigItem('PACKAGE_VERSION', '"2.0"')
    AddConfigItem('VRESION', '"2.0"')

    # append endianness checking defines
    AppendEndianCheck()
    
    # generate config.h
    f = file('config.h', 'w')
    f.write(config_h_content)
    f.close()

    # generate sunpinyin.pc
    f = file('sunpinyin-2.0.pc', 'w')
    content = (
        'prefix='+prefix,
        'exec_prefix=${prefix}',
        'libdir=${exec_prefix}/lib',
        'includedir=${exec_prefix}/include/sunpinyin-2.0',
        '',
        'Name: libsunpinyin',
        'Description: IME library based on Statistical Language Model',
        'Version: 2.0.1',
        'Requires: sqlite3',
        'Libs: -L${libdir} -lsunpinyin',
        'Cflags: ' + reduce(lambda a, b: a + ' ' + b,
                            map(lambda x: '-I${includedir}' + x[3:],
                                allinc()))
        )
    f.write(reduce(lambda a, b: a + '\n' + b, content))
    f.close()
    env = conf.Finish()

    if GetOS() != 'Darwin':
        env.ParseConfig('pkg-config sqlite3 --libs --cflags')

    LinkOSHeader()

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


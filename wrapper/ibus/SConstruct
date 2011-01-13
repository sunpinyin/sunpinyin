import os
import SCons.Tool.textfile

sources = ['src/debug.cpp',
           'src/engine.cpp',
           'src/ibus_portable.cpp',
           'src/imi_ibus_win.cpp',
           'src/main.cpp',
           'src/sunpinyin_config.cpp',
           'src/sunpinyin_engine.cpp',
           'src/sunpinyin_engine_proxy.cpp',
           'src/sunpinyin_lookup_table.cpp',
           'src/sunpinyin_property.cpp']

locales = ['zh_CN']

cflags = '-O2 -g -pipe'
version = '2.0.3'

AddOption('--prefix', dest='prefix', metavar='DIR',
          help='installation prefix')
AddOption('--libdir', dest='libdir', metavar='DIR',
          help='installation libdir')
AddOption('--libexecdir', dest='libexecdir', metavar='DIR',
          help='installation libexecdir')
AddOption('--datadir', dest='datadir', metavar='DIR',
          help='installation datadir')
AddOption('--rpath', dest='rpath', metavar='DIR',
          help='encode rpath in the executables')

opts = Variables('configure.conf')
opts.Add('PREFIX', default='/usr/local')
opts.Add('LIBDIR', default='/usr/local/lib')
opts.Add('LIBEXECDIR', default='/usr/local/lib')
opts.Add('DATADIR', default='/usr/local/share')

def PassVariables(envvar, env):
    for (x, y) in envvar:
        if x in os.environ:
            print 'Warning: you\'ve set %s in the environmental variable!' % x
            env[y] = os.environ[x]

env = Environment(ENV=os.environ,
                  CFLAGS=cflags, CXXFLAGS=cflags, 
                  CPPPATH=['.'], SUBSTFILESUFFIX='.in')
opts.Update(env)

if GetOption('prefix') is not None:
    env['PREFIX'] = GetOption('prefix')
    env['LIBDIR'] = env['PREFIX'] + '/lib'
    env['LIBEXECDIR'] = env['PREFIX'] + '/lib/'
    env['DATADIR'] = env['PREFIX'] + '/share/'

if GetOption('libdir') is not None:
    env['LIBDIR'] = GetOption('libdir')

if GetOption('libexecdir') is not None:
    env['LIBEXECDIR'] = GetOption('libexecdir')

if GetOption('datadir') is not None:
    env['DATADIR'] = GetOption('datadir')

opts.Save('configure.conf', env)

if GetOption('rpath') is not None:
    env.Append(LINKFLAGS='-Wl,-R -Wl,%s' % GetOption('rpath'))

envvar = [('CC', 'CC'),
          ('CXX', 'CXX'),
          ('CFLAGS', 'CFLAGS'),
          ('CXXFLAGS', 'CXXFLAGS'),
          ('LDFLAGS', 'LINKFLAGS')]
PassVariables(envvar, env)

data_dir = env['DATADIR'] + '/ibus-sunpinyin'
icons_dir = env['DATADIR'] + '/ibus-sunpinyin/icons'
bin_dir = env['LIBEXECDIR'] + '/ibus-sunpinyin'
gettext_package = 'ibus-sunpinyin'

extra_cflags = ' -DIBUS_SUNPINYIN_LOCALEDIR=\'"%s"\'' % (env['DATADIR'] + '/locale')
extra_cflags += ' -DIBUS_SUNPINYIN_ICON_DIR=\'"%s"\'' % icons_dir
extra_cflags += ' -DLIBEXECDIR=\'"%s"\'' % bin_dir
extra_cflags += ' -DGETTEXT_PACKAGE=\'"%s"\'' % gettext_package

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

conf = Configure(env, custom_tests={'CheckPKGConfig' : CheckPKGConfig,
                                    'CheckPKG' : CheckPKG })

def DoConfigure():
    if GetOption('clean'):
        return

    if not conf.CheckPKGConfig():
        Exit(1)

    if not conf.CheckPKG('ibus-1.0'):
        Exit(1)

    if not conf.CheckPKG('sunpinyin-2.0'):
        Exit(1)
    env = conf.Finish()
    env.ParseConfig('pkg-config ibus-1.0 sunpinyin-2.0 --libs --cflags')

DoConfigure()

env.Program('ibus-engine-sunpinyin', source=sources)
for locale in locales:
    mo = 'po/%s.mo' % (locale,)
    env.Command(mo, [], 'msgfmt po/%s.po -o %s' % (locale, mo))

# bug of scons...
# it doesn't include textfile builders by default?
SCons.Tool.textfile.generate(env)
sub = {'@LIBEXEC_DIR@': bin_dir,
       '@DATA_DIR@' : data_dir,
       '@ICON_DIR@': icons_dir,
       '@VERSION@' : version,
       '@INSTALL_PREFIX@': env['PREFIX']}
env.Substfile('data/sunpinyin.xml.in', SUBST_DICT=sub)
env.Substfile('setup/config.py.in', SUBST_DICT=sub)
env.Substfile('setup/ibus-setup-sunpinyin.in', SUBST_DICT=sub)

#
#==============================install================================
#
def DoInstall():
    libexec_target = env.Install(bin_dir, ['ibus-engine-sunpinyin', 
                                           'setup/ibus-setup-sunpinyin'])
    for exec_bin in libexec_target:
        env.AddPostAction(exec_bin, Chmod(str(exec_bin), 0755))
        
    setup_target = env.Install(data_dir + '/setup',
                               ['setup/setup.glade',
                                'setup/main.py',
                                'setup/config.py'])
    icons_target = env.Install(icons_dir,
                               ['icons/cnpunc.svg',
                                'icons/eng.svg',
                                'icons/enpunc.svg',
                                'icons/fullwidth.svg',
                                'icons/halfwidth.svg',
                                'icons/han.svg',
                                'icons/setup.svg',
                                'icons/sunpinyin-logo.png'])
    component_target = env.Install(env['DATADIR'] + '/ibus/component',
                                   'data/sunpinyin.xml')
    locale_targets = []
    for locale in locales:
        path = env['DATADIR'] + '/locale/%s/LC_MESSAGES/%s.mo' % \
            (locale, gettext_package)
        locale_targets.append(env.InstallAs(path, 'po/%s.mo' % (locale,)))

    env.Alias('install-libexec', [libexec_target, setup_target])
    env.Alias('install-data', [icons_target, component_target])
    env.Alias('install-locale', locale_targets)

DoInstall()
env.Alias('install', ['install-libexec', 'install-data', 'install-locale'])

#!/usr/bin/env python
# Package SunPinyin for release

import sys, os, commands, time, plistlib

try:
    from jinja2 import Environment, FileSystemLoader
except:
    print "Install jinja2 first."
    sys.exit(1)

env = Environment(loader=FileSystemLoader('.'))
plist = plistlib.readPlist("../build/SunPinyin.app/Contents/Info.plist")

url_base = "http://sunpinyin.googlecode.com/files/"
xml_url_base = "http://release.sunpinyin.googlecode.com/hg/"
appcast_url = xml_url_base + "SunpinyinAppcast.xml"

pack_proj = "SunPinyin/SunPinyin.packproj"
pkg = "SunPinyin/build/SunPinyin.pkg"
resource_dir = "../build/SunPinyin.app/Contents/Resources"

version = plist["CFBundleVersion"]
releasenotes_url = xml_url_base + "SunpinyinReleaseNotes.xml"

zip = "SunPinyin-MacOS-%s.zip" % version
file_url = url_base + zip

priv_key = "%s/.ssh/dsa_priv.pem" % os.path.expanduser('~')
date = time.strftime("%a, %d %b %Y %H:%M:%S %z")
appcast_template = 'appcast.template.xml'
appcast = "sunpinyin_appcast.xml"

if len(sys.argv) > 1:
    priv_key = sys.argv[1]

def remove_if_exists(file):
    if os.path.isfile(file):
        os.remove(file)

print "[PACK] Remove temporary files..."

#remove_if_exists("%s/lm_sc.t3g" % resource_dir)
#remove_if_exists("%s/pydict_sc.bin" % resource_dir)

print "[PACK] Building %s..." % pkg

os.system("freeze -v %s" % pack_proj)

print "[PACK] Compressing %s..." % zip
os.chdir("SunPinyin/build")
os.system("zip -y -r ../../%s SunPinyin.pkg" % zip)
os.chdir("../..")

print "[PACK] Signing %s..." % zip
signed = commands.getoutput('openssl dgst -sha1 -binary < "%s" | openssl dgst -dss1 -sign "%s" | openssl enc -base64' % (zip, priv_key))

print "[PACK] Generating %s..." % appcast
template = env.get_template(appcast_template)

output = open(appcast, "w")

output.write(template.render(link=appcast_url,
        releaseNotesLink=releasenotes_url,
        url=file_url,
        date=date,
        version=version,
        length=os.path.getsize(zip),
        signed=signed).encode("utf-8"))

output.close()

print "Done! Please:\n  1. Publish %s to %s\n  2. Publish %s to %s\n  3. Update the release note for version %s at %s." % (appcast, appcast_url, zip, file_url, version, releasenotes_url)


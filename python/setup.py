from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

setup(
    name='sunpinyin-python',
    version='0.1',
    author = "Yong Sun",
    author_email = "Yong.Sun@Sun.COM",
    url = "http://www.opensolaris.org/os/project/input-method",
    description = "A Python binding of the SunPinyin ngram model",
    long_description = "A Python binding of the SunPinyin ngram model",
    license = "CDDL/LGPLv2.1",

    ext_modules=[
        Extension(
            "pyslm", ["pyslm.pyx", 
                      "../src/slm/slm.cpp"], 
            language="c++", 
            include_dirs=["../src/slm", ".."], 
            define_macros=[("HAVE_CONFIG_H", None)]
        ),
        Extension(
            "pytrie", ["pytrie.pyx", 
                       "../src/portability.cpp", 
                       "../src/lexicon/pytrie.cpp", 
                       "../src/lexicon/pinyin_data.cpp"], 
            language="c++", 
            include_dirs=["../src", "../src/lexicon", ".."], 
            define_macros=[("HAVE_CONFIG_H", None)]
        ),
    ],
    cmdclass = {'build_ext': build_ext}
)

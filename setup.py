from glob import glob
from setuptools import setup

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir

import sys

__version__ = "0.1"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension("draughts1",
        ["src/bb_base.cpp",
         "src/bb_comp.cpp",
         "src/bb_index.cpp",
         "src/bit.cpp",
         "src/book.cpp",
         "src/common.cpp",
         "src/dxp.cpp",
         "src/eval.cpp",
         "src/fen.cpp",
         "src/game.cpp",
         "src/gen.cpp",
         "src/hash.cpp",
         "src/hub.cpp",
         "src/libmy.cpp",
         "src/list.cpp",
         "src/move.cpp",
         "src/pos.cpp",
         "src/score.cpp",
         "src/search.cpp",
         "src/socket.cpp",
         "src/sort.cpp",
         "src/thread.cpp",
         "src/tt.cpp",
         "src/util.cpp",
         "src/var.cpp",
         "tools/python-bindings.cpp"
         ],
        # Example: passing in the version to the compiled code
        define_macros = [('VERSION_INFO', __version__)],
        extra_compile_args=['-std=c++17'],
        extra_link_args=['-ltbb'],
        include_dirs=['include'],
    ),
]

setup(
    name="draughts1",
    version=__version__,
    author="Wieger Wesselink",
    author_email="j.w.wesselink@tue.nl",
    description="Draughts library (includes Scan 3.0)",
    long_description="",
    ext_modules=ext_modules,
    # extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
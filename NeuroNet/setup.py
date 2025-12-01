from setuptools import setup, Extension
from Cython.Build import cythonize
import os

# Define the extension
extensions = [
    Extension(
        "neuronet",
        sources=["src/cython/neuronet.pyx", "src/cpp/Graph.cpp"],
        include_dirs=["src/cpp"],
        language="c++",
        extra_compile_args=["/std:c++17"] if os.name == 'nt' else ["-std=c++17"],
    )
]

setup(
    name="neuronet",
    ext_modules=cythonize(extensions, language_level="3"),
)

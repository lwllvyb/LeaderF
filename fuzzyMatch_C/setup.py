# -*- coding: utf-8 -*-

from distutils.core import setup, Extension

setup(name='fuzzyMatchC',
      version='1.0',
      ext_modules=[
          Extension('fuzzyMatchC',
                    ['fuzzyMatch.c'])])

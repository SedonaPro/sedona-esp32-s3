#! /usr/bin/env python
#
# makeunixdev.py
# 
#    This script is used to rebuild and test what a developer typically
#    needs to get going.  It rebuilds most everything from source, 
#    then runs the sedonac and unix svm test suite as a fairly quick 
#    sanity check.
#
# Author:    Matthew Giannini
# Creation:  10 Dec 08
#                 

import os
import sys
import env
import fileutil
import makesedona
import makesedonac
import makesedonacert
import makeunixvm
import compilekit

def main(argv):
  # make sedona.jar  
  makesedona.compile()
  
  # make sedonac.jar  
  makesedonac.compile()

  # make sedonacert.jar
  makesedonacert.compile()
  
  # make all kits
  compilekit.compile(env.src)

  # make unix svm
  makeunixvm.main(sys.argv[1:])
  
  # make windows test scode
  #compilekit.compile(os.path.join(env.scode, "x86-test.xml"))  
  
  # run sedonac tests
  #status = os.system(env.sedonacExe + " -test")
  #if status:
  #  raise env.BuildError("FATAL: sedonac tests failed")   

  # run svm tests
  #  status = os.system(env.svmExe + " " + os.path.join(env.scode, "x86-test.scode") + " -test")
  #  if status:
  #    raise env.BuildError("FATAL: sedonac tests failed")

# Main
if __name__ == '__main__':
  main(sys.argv)

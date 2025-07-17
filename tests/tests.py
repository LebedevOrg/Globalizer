#!/usr/bin/env python
# -*- coding: utf-8 -*-

import unittest
import subprocess
import platform
import sys
import re
import argparse
import os.path

class TestCaseExamin(unittest.TestCase):
    def setUp(self):
        self.examinSuccessLogMessage = 'Global optimum FOUND!'
        self.examinBinPath = './'
        parser = argparse.ArgumentParser()
        parser.add_argument('--binPath', help = 'A path to the examin binary folder')
        args = parser.parse_args()
        args = vars(args)
        if args['binPath'] is not None:
            self.examinBinPath = args['binPath']
        self.examinExecPath = os.path.abspath(self.examinBinPath) + '/' + getPlatformExecutableName('examin')
        if not os.path.exists(self.examinExecPath):
            raise Exception('Examin exacutable not found: {}'.format(self.examinExecPath))

    def _run_examin(self, args, mpiProcs = 1):
        args += ' -spm 1000000'
        if mpiProcs == 1:
            cmd = self.examinExecPath + ' ' + args
        else:
            cmd = getPlatformMPIexecName() + ' -n ' + str(mpiProcs) + \
                ' ' + self.examinExecPath + ' ' + args
        PIPE = subprocess.PIPE
        p = subprocess.Popen(cmd, shell=True, stdin=PIPE, stdout=PIPE,
            stderr=subprocess.STDOUT)
        p.wait()
        return p.stdout.read().decode("utf-8")

    def _getLibPath(self, libname):
        return os.path.abspath(self.examinBinPath) + '/' + getPlatformLibName(libname)

    def _getLibConfigPath(self, libConf):
        return os.path.abspath(self.examinBinPath) + '/' + libConf

class TestExaminSequentalSolve(TestCaseExamin):
    def test_gkls_default_conf(self):
        log = self._run_examin('-lib ' + self._getLibPath('gkls') +
            ' -libConf ' + self._getLibConfigPath('gkls_conf.xml') + '-stopCond 1 -r 4 -dpp')
        self.assertIn(self.examinSuccessLogMessage, log)

        numberOfTrials = int(re.findall('NumberOfTrials = (\d+)', log)[0])
        self.assertLess(numberOfTrials, 500)

class TestExaminMPISolve(TestCaseExamin):
    def test_gkls_4d_2levels_4procs(self):
        log = self._run_examin('-lib ' + self._getLibPath('gkls') +
            ' -libConf ' + self._getLibConfigPath('gkls_conf.xml') +
            ' -stopCond 1 -r 4.5 -N 4 -nl 2 -dl 2_2 -cl 3_1 -dpp', 4)
        self.assertIn(self.examinSuccessLogMessage, log)

        numberOfIters = int(re.findall('Iteration = (\d+)', log)[0])
        self.assertLess(numberOfIters, 150)

def getPlatformMPIexecName():
    if 'Linux' in platform.system():
        return 'mpirun '
    elif 'Windows' in platform.system():
        return 'mpiexec '
    return None

def getPlatformLibName(lib):
    if 'Linux' in platform.system():
        return 'lib' + lib + '.so'
    elif 'Windows' in platform.system():
        return lib + '.dll'
    return None

def getPlatformExecutableName(name):
    if 'Linux' in platform.system():
        return name
    elif 'Windows' in platform.system():
        return name + '.exe'
    return None

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromModule(sys.modules[__name__])
    unittest.TextTestRunner(verbosity=2).run(suite)

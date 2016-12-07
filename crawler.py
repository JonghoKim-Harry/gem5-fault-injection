import subprocess
import re
import sys
import os.path

class Crawler:
    @staticmethod
    def parse_simout(filename):
        simout = open(filename, 'r')
        for line in simout:
            if 'command line:' in line:
                tmpcmd = line.replace('command line:', '')
                tmpcmd = tmpcmd.replace('gem5.debug', 'gem5.opt')
                cmd = re.sub(r"(--debug-flags=)(\S*)", r"\1FI", tmpcmd)
                subprocess.call(cmd, shell=True)

    @staticmethod
    def parse_debug(filename):
        pass


if __name__ == '__main__':
    for idx in range(int(sys.argv[2]), int(sys.argv[3]) + 1):
        filename = sys.argv[1] + '/PipeReg_f2ToD/simout_' + str(idx)
        if os.path.isfile(filename):
            Crawler.parse_simout(filename)

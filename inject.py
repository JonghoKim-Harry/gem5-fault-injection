import os
import sys
import random
import re
 
#  Absolute Path to *THIS* Script
WHERE_AM_I = os.path.dirname(os.path.realpath(__file__))

GOLDEN_RUNTIME = {
    'hello': 25617000,
    'matmul': 101952000,
    'stringsearch': 162824000       # on HP-Z640 w/ this script
}

BENCH_BINARY = {
    'hello': os.path.abspath(WHERE_AM_I + '/tests/test-progs/hello_arm'),
    'stringsearch': os.path.abspath(WHERE_AM_I + '/tests/test-progs/search_small_arm')
}

BENCH_OPTION = {
    'hello': None,
    'stringsearch': None
}

class ExpManager:
    ##
    #  gem5 commands are like:
    #    <gem5 bin> <gem5 options> <gem5 script> <gem5 script options>
    GEM5_BINARY = os.path.abspath(WHERE_AM_I + '/build/ARM/gem5.debug')
    GEM5_SCRIPT = os.path.abspath(WHERE_AM_I + '/configs/example/se.py')

    ##
    #  gem5 MinorCPU bits
    BIT_LENGTH = {
        'f1ToF2': 512,       # 64 Byte = 512 bit
        'f2ToD': 64,
        'dToE': 64,
        'eToF1': 32,
        'f2ToF1': 32
    }

    @staticmethod
    def inject_single(inj_time, inj_bit, inj_comp1, inj_comp2, idx=0, bench_name='stringsearch', do_injection=True):
        ##
        #  One fault injected per each experiment
        #
        
        # Component Info
        comp_info = '_'.join([inj_comp1, inj_comp2])

        #  gem5 option
        redirection = '-re'
        outdir = '--outdir=' + bench_name + '/' + comp_info
        stdout_file = '--stdout-file=' + 'simout_' + str(idx)
        stderr_file = '--stderr-file=' + 'simerr_' + str(idx)
        stats_file = '--stats-file=' + 'stats_' + str(idx)
        debug_file = '--debug-file=' + 'debug_' + str(idx)
        debug_flags = '--debug-flags=' + 'FI'
        gem5_option = ' '.join([redirection, outdir, stdout_file, stderr_file, stats_file, debug_file, debug_flags])

        #  gem5 script option
        env = '--cpu-type=MinorCPU --caches -n 1'
        bench_binary = '--cmd=' + BENCH_BINARY[bench_name]
        bench_option = BENCH_OPTION[bench_name]
        if bench_option:
            bench_option = '--options=' + bench_option
        else:
            bench_option = ''
        output = '--output=' + '/'.join([bench_name, comp_info, 'result_' + str(idx)])
        injectTime = '--injectTime=' + str(inj_time)
        injectLoc = '--injectLoc=' + str(inj_bit)
        injectArch = '--injectArch=' + inj_comp1
        injectComp = '--injectComp=' + inj_comp2
        runtime_limit = ' '.join(['-m', str(2 * GOLDEN_RUNTIME[bench_name])])
        if do_injection:
            inj_info = ' '.join([injectTime, injectLoc, injectArch, injectComp, runtime_limit])
        else:
            inj_info = ''
        gem5_script_option = ' '.join([env, bench_binary, bench_option, output, inj_info])

        #  gem5 command
        gem5_command = ' '.join([ExpManager.GEM5_BINARY, gem5_option, ExpManager.GEM5_SCRIPT, gem5_script_option])
        os.system(gem5_command)

    @staticmethod
    def inject_random(inj_comp1, inj_comp2, start_idx=1, end_idx=1000, bench_name='stringsearch', do_injection=True):
        runtime = GOLDEN_RUNTIME[bench_name]

        #  Digest - All stat & log files are too large to store
        os.system('mkdir ' + bench_name)
        comp_info = '_'.join([inj_comp1, inj_comp2])
        exp_info = '_'.join([bench_name, comp_info, str(start_idx), str(end_idx)])
        digest = open(bench_name + '/' + 'digest_' + exp_info + '.txt', 'a')

        #  Iterating several experiments
        for idx in range(int(start_idx), int(end_idx)+1):
            #  Pick random numbers
            rand_time = str(random.randrange(1, GOLDEN_RUNTIME[bench_name]))
            rand_bit = str(random.randrange(0, ExpManager.BIT_LENGTH[inj_comp2]))

            #  Do single experiment
            ExpManager.inject_single(rand_time, rand_bit, inj_comp1, inj_comp2, idx, bench_name, do_injection)

            digest.write('\t'.join([str(idx), rand_time, rand_bit]))

            ##
            #  Read stat file. If "sim_tick" stat not exists,
            #  we consider it as "failure"
            #
            failure = True
            outdir = bench_name + '/' + comp_info
            with open(outdir + '/' + 'stats_' + str(idx)) as stat_read:
                for line in stat_read:
                    pattern = re.compile(r'\s+')
                    line2 = re.sub(pattern, '', line)
                    if "sim_ticks" in line2:
                        failure = False
                        sim_ticks = int(re.findall('\d+', line2)[0])
                        runtime_100 = float(sim_ticks)/runtime*100
                        if runtime_100 >= 200:
                            failure = True
                        digest.write('\t' + str(runtime_100) + '%')

            if failure:
                digest.write('\tSys-halt')
            else:
                ##
                #  Read result file. If the result is different from golden output,
                #  we define it as SDC (Silent Data Corruption), which is failure
                #
                result = outdir + '/' + 'result_' + str(idx)
                if os.path.isfile(result):
                    golden = os.path.abspath(WHERE_AM_I + '/' + 'golden_output_' + bench_name)
                    if os.system(' '.join(['cmp', '-s', result, golden])) != 0:
                        digest.write('(SDC)')
                        failure = True
                else:
                    failure = True

            ##
            #  Log failure as "FF", non-failure as "NF"
            #
            if failure:
                digest.write('\tFF')
            else:
                digest.write('\tNF')

            digest.write('\n')
        digest.close()


if __name__ == '__main__':
    if len(sys.argv) >= 7:
        ExpManager.inject_random(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6])
    elif len(sys.argv) == 6:
        ExpManager.inject_random(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], True)
    else:
        print('More argument needed: <comp1> <comp2> <start index> <end index> <bench name> (do_injection)')
        print('comp1 = Reg | PipeReg')
        print('comp2 = f1ToF2 | f2ToD | dToE | eToF1 | f2ToF1')
        print('do_injection = True | False\t(Default: True)')

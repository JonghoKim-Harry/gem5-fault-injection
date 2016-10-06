GEM5_BINARY="./build/ARM/gem5.debug"
TEST_PROG="tests/test-progs/hello/bin/arm/linux/hello"
INJECT_TIME=1000000
INJECT_LOC=5
INJECT_ARCH="FU"

GEM5_OPTIONS=""

DEBUG_SCOREBOARD=true #set false to disable debug flag

if [ $DEBUG_SCOREBOARD = true ]; then
    GEM5_OPTIONS="--debug-flags=MinorScoreboard -r"
fi

$GEM5_BINARY $GEM5_OPTIONS configs/example/se.py -c $TEST_PROG --cpu-type "minor" --caches --injectTime $INJECT_TIME --injectLoc $INJECT_LOC --injectArch $INJECT_ARCH

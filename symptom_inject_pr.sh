bench_home="/home/yohan/miBench"

case "$2" in
hello )
  bench="helloWorld/hello_$1"
  options=""
  ;;
matmul )
  bench="matrixmul/matmul_$1"
  options=""
  ;;
qsort )
  bench="automotive/qsort/qsort_small_$1"
  options="$bench_home/automotive/qsort/input_small.dat"
  ;;
stringsearch )
  bench="office/stringsearch/search_small_$1"
  options=""
  ;;
gsm )
  bench="telecomm/gsm/bin/toast_$1"
  options="-fps -c $bench_home/telecomm/gsm/data/small.au"
  ;;
bitcount )
  bench="automotive/bitcount/bitcnts_$1"
  options=7500
  ;;
jpeg )
  bench="consumer/jpeg/jpeg-6a/cjpeg_$1"
  options="-dct int -progressive -opt -outfile output_small_encode.jpeg $bench_home/consumer/jpeg/input_small.ppm"
  ;;
fft )
  bench="telecomm/FFT/fft_$1"
  options="4 4096"
  ;;
dijkstra )
  bench="network/dijkstra/dijkstra_small_$1"
  options="$bench_home/network/dijkstra/input.dat"
  ;;
basicmath ) 
  bench="automotive/basicmath/basicmath_small_$1"
  options=""
  ;;
typese )
  bench="/consumer/typeset/lout-3.24/lout_$1"
  options=" -I $bench_home/consumer/typeset/lout-3.24/include -D $bench_home/consumer/typeset/lout-3.24/data -F $bench_home/consumer/typeset/lout-3.24/font -C $bench_home/consumer/typeset/lout-3.24/maps -H $bench_home/consumer/typeset/lout-3.24/hyph $bench_home/consumer/typeset/small.lout"
  ;;
crc )
  bench="telecomm/CRC32/crc_$1"
  options="$bench_home/telecomm/adpcm/data/large.pcm"
  ;;
patricia )
  bench="network/patricia/patricia_$1"
  options="$bench_home/network/patricia/small.udp"
  ;;
sha )
  bench="security/sha/sha_$1"
  options="$bench_home/security/sha/input_small.asc"
  ;;
ispell )
  bench="office/ispell/ispell_$1"
  options="$bench_home/office/ispell/tests/americanmed+ < $bench_home/office/ispell/tests/small.txt"
  ;;
esac

protection=no_protection								# Protection scheme to be used
vul_analysis=no								# Enable/Disable vulnerability analysis
cpu_type=MinorCPU								# CPU Type
num_procs=1									# Number of processors
gemv_exec_path=./build/$1/gem5.opt		# Path to gemv executable
config_path=./configs/example/se.py		# Path to config file

#$gemv_exec_path -d $2/$6 -re --stdout-file=simout_$5 --stderr-file=simerr_$5 --debug-file=FI_$5 --stats-file=stats_$5 --debug-flags=FI,Exec,Minor,IntRegs,SyscallAll $config_path --cpu-type=$cpu_type --caches -n $num_procs -c "$bench_home/$bench" -o "$options" --output=$2/$6/result_$5 --injectArch=$6 --injectTime=$3 --injectLoc=$4 -m $7

$gemv_exec_path -d $2/$6 -re --stdout-file=simout_$5 --stderr-file=simerr_$5 --debug-file=FI_$5 --stats-file=stats_$5 --debug-flags=FI,SymptomFI,Symptom $config_path --cpu-type=$cpu_type --caches -n $num_procs -c "$bench_home/$bench" -o "$options" --output=$2/$6/result_$5 --injectArch=$6 --injectTime=$3 --injectLoc=$4 --injectComp=$8 -m $7 #--traceMask=YES --correctStore --correctLoad

if cmp -s ./$2/$6/result_$5 golden/golden_output_$2
then
	if cmp -s ./$2/$6/stats_$5 bin.txt
	then
		echo "F	$3	$4"
	else
		echo "NF	$3	$4"
	fi
else
	echo "F	$3	$4"
fi

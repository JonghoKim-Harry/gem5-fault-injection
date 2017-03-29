bench_home="/home/yohan/miBench"

case "$2" in
susan )
  bench="automotive/susan/susan_$1"
  options="$bench_home/automotive/susan/input_small.pgm $2/$6/result_$8 -e"
  ;;
jpeg )
  bench="consumer/jpeg/jpeg-6a/cjpeg_$1"
  options="-dct int -progressive -opt -outfile $2/$6/result_$8 $bench_home/consumer/jpeg/input_small.ppm"
esac

protection=no_protection								# Protection scheme to be used
vul_analysis=no								# Enable/Disable vulnerability analysis
cpu_type=MinorCPU								# CPU Type
num_procs=1									# Number of processors
gemv_exec_path=./build/$1/gem5.opt		# Path to gemv executable
config_path=./configs/example/se.py		# Path to config file

$gemv_exec_path -d $2/$6 -re --stdout-file=simout_$5 --stderr-file=simerr_$5 --debug-file=FI_$5 --stats-file=stats_$5 --debug-flags=FI,SymptomFI,Symptom $config_path --cpu-type=$cpu_type --caches -n $num_procs -c "$bench_home/$bench" -o "$options" --injectArch=$6 --injectTime=$3 --injectLoc=$4 --injectComp=$9 -m $7 --traceMask=YES


if cmp -s ./$2/$6/result_$8 golden/golden_output_$2
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

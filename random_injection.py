import os
import sys
import random
import re

arch = sys.argv[1]
bench = sys.argv[2]
injectArch = sys.argv[3]
start = sys.argv[4]
end = sys.argv[5]

if(bench == 'hello'):
	runtime = 29995000
elif(bench == 'matmul'):
	runtime = 101952000
elif(bench == 'stringsearch'):
	runtime = 162836000
elif(bench == 'susan'):
	runtime = 1540836000
elif(bench == 'jpeg'):
	runtime = 9902426000
elif(bench == 'gsm'):
	runtime = 9985126500
elif(bench == 'bitcount'):
	runtime = 12314086500
elif(bench == 'qsort'):
	runtime = 13467377000
elif(bench == 'dijkstra'):
	runtime = 25391777500
elif(bench == 'basicmath'):
	runtime = 25391777500

os.system("mkdir " + str(bench))
#f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'w') 

os.system("rm -rf " + str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt")

for i in range(int(start), int(end)):
	if (injectArch == "NO"):
		injectLoc = 0
	if (injectArch == "Reg"):
		injectLoc = random.randrange(0,1376) #32: Data (43 integer RFs)
	
	injectTime = random.randrange(0,runtime)
	#injectTime = 2129574
	#injectLoc = 255
	f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'a') 
	
	f.write(str(injectTime) + "\t" + str(injectLoc) + "\t")
	
	if (injectArch == "Reg"):
		if(bench == 'susan') or (bench == 'jpeg'):
			os.system("./inject_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
		else:
			os.system("./inject.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
		
		fi_read = file(bench+"/FI_" + str(injectArch)+ "_" + str(i))
		for line in fi_read:
			if "NF" in line:
				f.write("NF\t")
			else:
				f.write("F\t")
				
		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		for line in fi_read:
			line2 = line.strip().split(' ')
			if "read" in line:
				f.write("read\t" + line2[4] + "\t" + line2[8] + "\t")
			elif "overwritten" in line:
				f.write("overwritten\t" + line2[4] + "\t" + line2[8] + "\t")
			elif "unused" in line:
				f.write("unused\t" + line2[4] + "\t\t")
		
		failure = True
		stat_read = file(bench+"/"+injectArch+"/stats_" + str(i))
		for line in stat_read:
			pattern = re.compile(r'\s+')
			line = re.sub(pattern, '', line)
			line2 = line.strip().split(' ')
			if "sim_ticks" in line:
				sim_ticks = int(re.findall('\d+', line)[0])
				f.write(str(float(sim_ticks)/runtime*100) + "\n")
				failure = False
				
		if(failure):
			f.write("failure\n")
		
	f.close()
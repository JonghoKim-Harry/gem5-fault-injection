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
	runtime = 1203432
elif(bench == 'matmul'):
	runtime = 101952000
elif(bench == 'stringsearch'):
	runtime = 162835000 #valid for DC LAB server
elif(bench == 'susan'):
	runtime = 2368994000
elif(bench == 'gsm'):
	runtime = 15977939000
elif(bench == 'jpeg'):
	runtime = 17640839000
elif(bench == 'bitcount'):
	runtime = 23239204000
elif(bench == 'qsort'):
	runtime = 17144029500
elif(bench == 'dijkstra'):
	runtime = 27263894000
elif(bench == 'basicmath'):
	runtime = 250114465000

os.system("mkdir " + str(bench))
#f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'w') 

os.system("rm -rf " + str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt")

for i in range(int(start), int(end)):
	if (injectArch == "NO"):
		injectLoc = 0
	elif (injectArch == "Reg"):
		injectLoc = random.randrange(0,480) #32: Data (15 user integer registers)
	elif (injectArch == "FU"):
		injectLoc = random.randrange(1,15)
	
	injectTime = random.randrange(0,runtime)
	injectTime = 145745500
	injectLoc = 164
	f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'a') 
	
	f.write(str(injectTime) + "\t" + str(injectLoc) + "\t")
	
	if (injectArch == "Reg"):
		if(bench == 'susan') or (bench == 'jpeg'):
			os.system("./inject_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5)  + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
		else:
			os.system("./inject.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
		
		fi_read = file(bench+"/FI_" + str(injectArch)+ "_" + str(i))
		for line in fi_read:
			if "NF" in line:
				f.write("NF\t")
			else:
				f.write("F\t")
				
		read = False
		overwritten = False
				
		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		for line in fi_read:
			line2 = line.strip().split(' ')
			if "read" in line and read is False:
				read = True
				f.write("read\t" + line2[4] + "\t" + line2[8] + "\t")
				if("syscall" not in line):
					pcState = line2[9]
			elif "read" in line and read is True:
				if("syscall" in line):
					f.write(line2[8] + "\t")
				elif(pcState != line2[9]):
					f.write(line2[8] + "\t")
					pcState = line2[9]
			elif "overwritten" in line and read is False:
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
			
	if (injectArch == "FU"):
		if(bench == 'susan') or (bench == 'jpeg'):
			os.system("./inject_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5)  + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
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
			if "Flipping" in line:
				f.write(line2[6] + "\t" + line2[9] + "\t")
		
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

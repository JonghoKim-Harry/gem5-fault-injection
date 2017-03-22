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
	runtime = 102119000 #valid for DC LAB server
elif(bench == 'stringsearch'):
	runtime = 162835000 #valid for DC LAB server
elif(bench == 'susan'):
	runtime = 2369362000 #valid for DC LAB server
elif(bench == 'gsm'):
	runtime = 15973624000 #valid for DC LAB server
elif(bench == 'jpeg'):
	runtime = 17637096000 #valid for DC LAB server
elif(bench == 'bitcount'):
	#runtime = 23239140000 #valid for DC LAB server
	runtime = 2409772000 #valid for DC LAB server (input 7500)    
elif(bench == 'qsort'):
	runtime = 16505757500 #valid for DC LAB server
elif(bench == 'dijkstra'):
	runtime = 27277285000 #valid for DC LAB server
elif(bench == 'basicmath'):
	runtime = 251651656000 #valid for DC LAB server
elif(bench == 'crc'):
	runtime = 1091403357500 #valid for DC LAB server
elif(bench == 'fft'):
	runtime = 28748840000
elif(bench == 'typese'):
	runtime = 83872940000
elif(bench == 'patricia'):
	runtime = 99999999999999
elif(bench == 'sha'):
	runtime = 4890363000
elif(bench == 'ispell'):
	runtime = 99999999999999

os.system("mkdir " + str(bench))
#f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'w') 

os.system("rm -rf " + str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt")
os.system("rm -rf " + str(bench) + "/sec_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt")

for i in range(int(start), int(end)):
	if (injectArch == "NO"):
		injectLoc = 0
	elif (injectArch == "Reg" or injectArch == "RegHard"):
		injectLoc = random.randrange(0,480) #32: Data (15 user integer registers)
	elif (injectArch == "FU"):
		injectLoc = random.randrange(1,15)
        elif (injectArch == "LSQ"):
                injectIndex = random.randrange(0,8)
                injectLoc = random.randrange(0,96)+injectIndex*128

        injectTime = random.randrange(0,runtime)
        #injectTime = 9841886
        #injectLoc = 65
        f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'a')

        f.write(str(injectTime) + "\t" + str(injectLoc) + "\t")

	if (injectArch == "Reg" or injectArch == "RegHard"):
		if(bench == 'susan') or (bench == 'jpeg'):
			os.system("./symptom_inject_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5)  + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
		else:
			os.system("./symptom_inject.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
			
		non_failure = False
		fi_read = file(bench+"/FI_" + str(injectArch)+ "_" + str(i))
		for line in fi_read:
			if "NF" in line:
				non_failure = True

		previous = "NF"
		
		read = False
		contRead = False
		overwritten = False
				
		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		for line in fi_read:
			line2 = line.strip().split(' ')
			if "Corrupted" in line:
				if "read" in line and read is False:
					read = True
					if("syscall" not in line):
						pcState = line2[9]
				elif "read" in line and read is True:
					contRead = True
		
		failure = True
		stat_read = file(bench+"/"+injectArch+"/stats_" + str(i))
		for line in stat_read:
			pattern = re.compile(r'\s+')
			line = re.sub(pattern, '', line)
			line2 = line.strip().split(' ')
			if "sim_ticks" in line:
				sim_ticks = int(re.findall('\d+', line)[0])
				failure = False
				if(float(sim_ticks)/runtime*100 <= 100 and non_failure is True):
					contRead = False
				elif(float(sim_ticks)/runtime*100 >= 200):
					previous = "infinite"
				elif(float(sim_ticks)/runtime*100 > 100 and non_failure is True):
					previous = "timing"
				
		if(failure):
			previous = "halt"
		elif(failure is False and non_failure is False):
			previous = "sdc"

		second_complete = False
		read = False
		correctTime = []

		fi_read = file(bench+"/FI_" + str(injectArch)+ "_" + str(i))
		for line in fi_read:
			if "NF" in line:
				f.write("NF\t")
				non_failure = True
			else:
				f.write("F\t")

		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		for line in fi_read:
			line2 = line.strip().split(' ')
			if "Corrupted" in line:
				if "read" in line and contRead is False:
					read = True
					f.write("read\t" + line2[4] + "\t")
					break
				elif "read" in line and contRead is True:
					contRead = True
					f.write("read\t" + line2[4] + "\t")
					break
				elif "overwritten" in line and read is False:
					f.write("overwritten\t" + line2[4] + "\t")
				elif "unused" in line:
					f.write("unused\t" + line2[4] + "\t")
				elif "corrected" in line:
					f.write("corrected\t" + line2[4] + "\t")
		
		failure = True
		stat_read = file(bench+"/"+injectArch+"/stats_" + str(i))
		for line in stat_read:
			pattern = re.compile(r'\s+')
			line = re.sub(pattern, '', line)
			line2 = line.strip().split(' ')
			if "sim_ticks" in line:
				sim_ticks = int(re.findall('\d+', line)[0])
				f.write(str(float(sim_ticks)/runtime*100) + "\t")
				failure = False
				if(float(sim_ticks)/runtime*100 <= 100 and non_failure is True):
					contRead = False
				
		if(failure):
			f.write("failure\t")
			
		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		found=False;
		for line in fi_read:
			line2 = line.strip().split('=')
			if "Misprediction Early length" in line:
				f.write(line2[1] + "\t")
				found=True;
		if not found:
			f.write("Empty" + "\t")
		
		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		found=False;
		for line in fi_read:
			line2 = line.strip().split('=')
			if "Misprediction Read length" in line:
				f.write(line2[1] + "\t")
				found=True;
		if not found:
			f.write("Empty" + "\t")
		
		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		found=False;
		for line in fi_read:
			line2 = line.strip().split('=')
			if "Exception Early length" in line:
				f.write(line2[1] + "\t")
				found=True;
		if not found:
			f.write("Empty" + "\t")

		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		found=False;
		for line in fi_read:
			line2 = line.strip().split('=')		
			if "Exception Read length" in line:
				f.write(line2[1] + "\t")
				found=True;
		if not found:
			f.write("Empty" + "\t")

		fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
		found=False;
		for line in fi_read:
			line2 = line.strip().split('=')
			if "Exception Name" in line:
				f.write(line2[1] + "\t")
				found=True;
		if not found:
			f.write("Empty" + "\t")
		
		
		f.write("\n")
					
		#os.system("rm -rf " + bench + "/" + injectArch + "/FI_" + str(i) + "_*")
		#os.system("rm -rf " + bench + "/" + injectArch + "/FI_" + str(i))
		#os.system("rm -rf " + bench + "/" + injectArch + "/result_" + str(i) + "_*")
		#os.system("rm -rf " + bench + "/" + injectArch + "/simout_" + str(i) + "_*")
		#os.system("rm -rf " + bench + "/" + injectArch + "/simerr_" + str(i) + "_*")
		#os.system("rm -rf " + bench + "/" + "/FI_" + injectArch + "_" + str(i) + "_*")
			
			
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

        if (injectArch == "LSQ"):
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

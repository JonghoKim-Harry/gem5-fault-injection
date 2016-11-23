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
	runtime = 23239140000 #valid for DC LAB server
elif(bench == 'qsort'):
	runtime = 16505757500 #valid for DC LAB server
elif(bench == 'dijkstra'):
	runtime = 27277285000 #valid for DC LAB server
elif(bench == 'basicmath'):
	runtime = 250114465000

os.system("mkdir " + str(bench))
#f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'w') 

os.system("rm -rf " + str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt")
os.system("rm -rf " + str(bench) + "/sec_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt")

for i in range(int(start), int(end)):
	if (injectArch == "NO"):
		injectLoc = 0
	elif (injectArch == "Reg"):
		injectLoc = random.randrange(0,480) #32: Data (15 user integer registers)
	elif (injectArch == "FU"):
		injectLoc = random.randrange(1,15)
	
	injectTime = random.randrange(0,runtime)
	#injectTime = 85381464
	#injectLoc = 361
	f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'a') 
	s = open(str(bench) + "/sec_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'a') 
	
	f.write(str(injectTime) + "\t" + str(injectLoc) + "\t")
	
	if (injectArch == "Reg"):
		if(bench == 'susan') or (bench == 'jpeg'):
			os.system("./inject_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5)  + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
		else:
			os.system("./inject.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
			
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

		contReadIdx = 0
		second_complete = False
		read = False
		correctTime = []
	
		if(contRead):
			s.write(str(i)+"\n")
			fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
			linecount = 0
			previous_op = "ND"
			for line in fi_read:
				line2 = line.strip().split(':')
				line3 = line.strip().split(' ')
				if "read" in line:
					if previous_op != line3[8]:
						correctTime.append(line2[0])
						previous_op = line3[8]
						linecount += 1
					else:
						correctTime[linecount-1] = line2[0]
			#print correctTime
			
			if linecount > 100:
				root = "100ND"
				
			elif linecount == 1:
				root = previous_op
			
			else:
				#fi_read = file(bench+"/"+injectArch+"/FI_"+str(i))
				#for line in fi_read:
				print "Second-level analysis"
				for contReadIdx in range(0, linecount):	
					#line2 = line.strip().split(':')
					#line3 = line.strip().split(' ')
					#if "read" in line:
						#correctTime = line2[0]
					if(bench == 'susan') or (bench == 'jpeg'):
						os.system("./second_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5) + " " + str(contReadIdx) +  " " + str(correctTime[contReadIdx]) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i) + "_" + str(contReadIdx))
					else:
						os.system("./second.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) +  " " + str(contReadIdx) +  " " + str(correctTime[contReadIdx]) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i) + "_" + str(contReadIdx))
					
					fi_read_second = file(bench+"/FI_" + str(injectArch)+ "_" + str(i) + "_" + str(contReadIdx))
					for line_second in fi_read_second:
						if "NF" in line_second:
							non_failure = True
							s.write("NF\t")
						else:
							second_complete = True
							non_failure = False
							s.write("F\t")
					
					root_candidate = "ND"
							
					fi_read_second = file(bench+"/"+injectArch+"/FI_"+str(i)+"_"+str(contReadIdx))
					for line_second in fi_read_second:
						line2_second = line_second.strip().split(' ')
						if "read" in line_second and read is False:
							read = True
							s.write(line2_second[8] + "\t")
							root_candidate = line2_second[8]
						elif "read" in line_second and read is True:
							s.write(line2_second[8] + "\t")
							root_candidate = line2_second[8]
						elif "overwritten" in line_second and read is False:
							s.write("overwritten\t" + line2_second[4] + "\t" + line2_second[8] + "\t")
						elif "unused" in line_second:
							s.write("unused\t" + line2_second[4] + "\t\t")
							
					failure = True
					stat_read = file(bench+"/"+injectArch+"/stats_" + str(i)+"_"+str(contReadIdx))
					root = "ND"
					for line in stat_read:
						pattern = re.compile(r'\s+')
						line = re.sub(pattern, '', line)
						line2 = line.strip().split(' ')
						if "sim_ticks" in line:
							sim_ticks = int(re.findall('\d+', line)[0])
							s.write(str(float(sim_ticks)/runtime*100) + "\n")
							failure = False
							if(float(sim_ticks)/runtime*100 >= 200):
								if(previous == "infinite"):
									root = root_candidate
							elif(float(sim_ticks)/runtime*100 > 100 and non_failure is True):
								if(previous == "timing"):
									root = root_candidate
							
					if(failure):
						s.write("failure\n")
						if(previous == "halt"):
							root = root_candidate
					elif(failure is False and non_failure is False):
						if(previous == "sdc"):
							root = root_candidate
						
					#contReadIdx += 1
					
					if (root != "ND"):
						s.write("\n")
						break
	
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
			if "read" in line and contRead is False:
				read = True
				f.write("read\t" + line2[4] + "\t" + line2[8] + "\t")
				break
			elif "read" in line and contRead is True:
				contRead = True
				f.write("read\t" + line2[4] + "\t" + root + "\t")
				break
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
				if(float(sim_ticks)/runtime*100 <= 100 and non_failure is True):
					contRead = False
				
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
	s.close()
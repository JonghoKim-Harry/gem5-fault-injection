import os
import sys
import random
import re

arch = sys.argv[1]
bench = sys.argv[2]
injectArch = sys.argv[3]
start = sys.argv[4]
end = sys.argv[5]
profiledFile = sys.argv[6]

selectiveInput = open(profiledFile, 'r')

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
    elif (injectArch == "Reg"):
        injectLoc = random.randrange(0,480) #32: Data (15 user integer registers)
    elif (injectArch == "FU"):
        injectLoc = random.randrange(1,15)
    elif (injectArch == "LSQ"):
        #injectIndex = random.randrange(0,8)
        #injectLoc = random.randrange(0,96)+injectIndex*128
        line = selectiveInput.readline()
        cols = line.rstrip('\t').split('\t')
        injectTime = int(cols[0])
        LSQQueue = cols[1] #requests, transfers, storeBuffer
        injectLoc = int(cols[2])
        if LSQQueue == "requests":
            injectLoc = injectLoc # do nothing
        elif LSQQueue == "transfers":
            injectLoc = injectLoc + 128
        elif LSQQueue == "storeBuffer":
            injectLoc = injectLoc + 384
        else:
            print "Wrong LSQ Queue"
            sys.exit(0)
    elif (injectArch == "PipeReg"):
        line = selectiveInput.readline()
        cols = line.rstrip('\t').split('\t')
        injectTime = int(cols[2])
        injectLoc = int(cols[1])
        injectComp = cols[0]
        

    #injectTime = random.randrange(0,runtime)
    #injectTime = 9841886
    #injectLoc = 65
    f = open(str(bench) + "/val_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'a')
    f.write(str(injectTime) + "\t" + str(injectLoc) + "\t")
    
    symptomf = open(str(bench) + "/symptom_" + str(injectArch)+"_"+str(start)+"_"+str(end)+".txt", 'a')
    
    valFailure = "Initial"
    valRuntime = "Initial"    
    if (injectArch == "Reg"):
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
                valFailure = "NF"
                non_failure = True
            else:
                f.write("F\t")
                valFailure = "F"

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
                valRuntime=str(float(sim_ticks)/runtime*100)
                failure = False
                if(float(sim_ticks)/runtime*100 <= 100 and non_failure is True):
                    contRead = False
                
        if(failure):
            f.write("failure\t")
            valRuntime="failure"
            
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
            os.system("./symptom_inject_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5)  + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
        else:
            os.system("./symptom_inject.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
        
        fi_read = file(bench+"/FI_" + str(injectArch)+ "_" + str(i))
        for line in fi_read:
            if "NF" in line:
                f.write("NF\t")
                valFailure = "NF"
            else:
                f.write("F\t")
                valFailure = "F"
                
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
                valRuntime=str(float(sim_ticks)/runtime*100)
                failure = False
                
        if(failure):
            f.write("failure\n")        
            valRuntime="failure"

    if (injectArch == "LSQ"):
        if(bench == 'susan') or (bench == 'jpeg'):
                os.system("./symptom_inject_output.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5)  + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
        else:
                os.system("./symptom_inject.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))

        fi_read = file(bench+"/FI_" + str(injectArch)+ "_" + str(i))
        for line in fi_read:
                if "NF" in line:
                        f.write("NF\t")
                        valFailure = "NF"
                else:
                        f.write("F\t")
                        valFailure = "F"

        failure = True
        stat_read = file(bench+"/"+injectArch+"/stats_" + str(i))
        for line in stat_read:
                pattern = re.compile(r'\s+')
                line = re.sub(pattern, '', line)
                line2 = line.strip().split(' ')
                if "sim_ticks" in line:
                        sim_ticks = int(re.findall('\d+', line)[0])
                        f.write(str(float(sim_ticks)/runtime*100) + "\n")
                        valRuntime=str(float(sim_ticks)/runtime*100)
                        failure = False

        if(failure):
            f.write("failure\n")
            valRuntime="failure"
    
    
    if (injectArch == "PipeReg"):
        if(bench == 'susan') or (bench == 'jpeg'):
                os.system("./symptom_inject_output_pr.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(i).zfill(5)  + " " + str(injectComp) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))
        else:
                os.system("./symptom_inject_pr.sh " + str(arch) + " " + str(bench) + " " + str(injectTime) + " " + str(injectLoc) + " " + str(i) + " " + str(injectArch) + " " + str(2*runtime) + " " + str(injectComp) + " > " + str(bench) + "/FI_" + str(injectArch) + "_" + str(i))

        fi_read = file(bench+"/FI_" + str(injectArch)+ "_" + str(i))
        for line in fi_read:
                if "NF" in line:
                        f.write("NF\t")
                        valFailure = "NF"
                else:
                        f.write("F\t")
                        valFailure = "F"

        failure = True
        stat_read = file(bench+"/"+injectArch+"/stats_" + str(i))
        for line in stat_read:
                pattern = re.compile(r'\s+')
                line = re.sub(pattern, '', line)
                line2 = line.strip().split(' ')
                if "sim_ticks" in line:
                        sim_ticks = int(re.findall('\d+', line)[0])
                        f.write(str(float(sim_ticks)/runtime*100) + "\n")
                        valRuntime=str(float(sim_ticks)/runtime*100)
                        failure = False

        if(failure):
            f.write("failure\n")
            valRuntime="failure"
            
        
        bubbleCheckFile = open(bench+"/"+injectArch+"/"+"FI_"+str(i))
        bubbleCheckLines=bubbleCheckFile.read()
        if "BUBBLE" in bubbleCheckLines:
            tempLines=[]
            bubbleCheckFile.close()
            symptomf.write(bench + "\t")
            symptomf.write(str(i) + "\t")
            symptomf.write("None" + "\t")
            symptomf.write(injectArch + "\t")
            symptomf.write(str(injectTime) + "\t")
            symptomf.write(str(injectLoc) + "\t")
            symptomf.write("BUBBLE" + "\t")
            symptomf.write(injectComp + "\n")
            os.system("rm " + bench+"/"+injectArch+"/"+"FI_"+str(i))
            continue
        elif "FAULT" in bubbleCheckLines:
            tempLines=[]
            bubbleCheckFile.close()
            symptomf.write(bench + "\t")
            symptomf.write(str(i) + "\t")
            symptomf.write("None" + "\t")
            symptomf.write(injectArch + "\t")
            symptomf.write(str(injectTime) + "\t")
            symptomf.write(str(injectLoc) + "\t")
            symptomf.write("BUBBLE" + "\t")
            symptomf.write(injectComp + "\n")
            os.system("rm " + bench+"/"+injectArch+"/"+"FI_"+str(i))    
            continue
        tempLines=[]
        bubbleCheckFile.close()
            
            
            
            
            
    f.close()

        
    
    #parse phase
    srcTxt = bench+"/"+injectArch+"/"+"FI_ori"
    cmpTxt = bench+"/"+injectArch+"/"+"FI_"+str(i)
    
    #branch misprediction parse phase
    f1 = open(srcTxt, 'r')
    f2 = open(cmpTxt, 'r')
    
    bm_type = "None"
    bm_length = "None"
    bm_predicted = "None"
    bm_actual = "None"
    
    bm_isFirst = False
    bm_firstLength = "None"
    uselessFlag = False
    
    injectEarlySN = "None"
    bm_uselessLength = "None"
    bm_inst = "None"
    bm_uselessInst = "None"
   
    while True:
        line1 = f1.readline()
        line2 = f2.readline()
        while(not "Taken" in line2 and line2):
            if ("injectEarlySN" in line2):
                line2_SN = line2.strip().split(' ')
                injectSN = int(line2_SN[4])
                injectEarlySN = str(injectSN)
                bm_isFirst=True
            line2 = f2.readline()
        if not line2:break
        
        while(not "Taken" in line1 and line1):
            line1 = f1.readline()
        line1_split = line1.strip().split('\t')
        pc1 = line1_split[0].strip().split(":")
        line2_split = line2.strip().split('\t')
        pc2 = line2_split[0].strip().split(":")
        
        #find first
        if bm_isFirst and "Incorrect" in line2:
            bm_first = False
            bm_firstLength = str(int(line2_split[5]) - injectSN)
        
        if uselessFlag == False:
            if len(pc1) == 3: #pc1 and pc2 
                if(pc1[2] == pc2[2]):
                    if(line1_split[4] != line2_split[4]):
                        if("Correct" in line1):
                            bm_type = "Good"
                            bm_length = str(int(line2_split[5]) - injectSN)
                            bm_predicted = line2_split[2]
                            bm_actual = line2_split[3]
                            bm_inst = line2_split[1]
                            break
                        elif("Incorrect" in line1):
                            bm_type = "Useless"
                            bm_uselessLength = str(int(line2_split[5]) - injectSN)
                            bm_uselessInst = line2_split[1]
                            uselessFlag = True
                elif("Incorrect" in line2): #if different pc and correct, then we parse next pair
                    bm_type = "DifferentPC"
                    bm_length = str(int(line1_split[5]) - injectSN)
                    bm_predicted = line2_split[2]
                    bm_actual = line2_split[3]
                    bm_inst = line2_split[1]
                    break
            else: #if only original ended. we only consider additional incorrect.
                if("Incorrect" in line2):
                    bm_type = "Good"
                    bm_length = str(int(line2_split[5]) - injectSN)
                    bm_predicted = line2_split[2]
                    bm_actual = line2_split[3]
                    bm_inst = line2_split[1]
                    break
        elif "Incorrect" in line2: #After useless. we only consider incorrect
            bm_type = "Real after usless"
            bm_length = str(int(line2_split[5]) - injectSN)
            bm_predicted = line2_split[2]
            bm_actual = line2_split[3]
            bm_inst = line2_split[1]
            break
            
    f1.close()
    f2.close()
    
    #exception parse phase
    f1 = open(srcTxt, 'r')
    f2 = open(cmpTxt, 'r')
    
    ex_length = "None"
    ex_type = "None"
    
    ex_isFirst = False
    ex_firstLength = "None"
    
    ex_inst = "None"
    
    while True:
        line1 = f1.readline()
        while(not "Exec:" in line1 and line1):
            line1=f1.readline()
            
        line2 = f2.readline()
        while(not "Exec:" in line2 and line2):
            if("injectEarlySN" in line2):
                line2_SN = line2.strip().split(' ')
                injectSN = int(line2_SN[4])
                ex_isFirst=True
            line2=f2.readline()
        if not line2:break
        
        #No symptom in original but it occurs in corrupted run
        if not line1 and line2:
            line2_split = line2.strip().split(':')
            ex_type = line2_split[11]
            ex_inst = line2_split[13]
            ex_length = str(int(line2_split[9])-injectSN)
            if ex_isFirst:
                ex_isFirst = False
                ex_firstLength = str(int(line2_split[9])-injectSN)
            break
      
        line1_split = line1.strip().split(':')
        line2_split = line2.strip().split(':')
        #[7]=PC, [9]=SeqNum [11]=Type

        if ex_isFirst:
            ex_isFirst = False
            ex_firstLength = str(int(line2_split[9])-injectSN)
        
        if line1_split[7] == line2_split[7]:
            continue
        
        ex_type = line2_split[11]
        ex_inst = line2_split[13]
        ex_length = str(int(line2_split[9])-injectSN)
        break

    f1.close()
    f2.close()
    
    
    
    #cache miss parse phase
    f1 = open(srcTxt, 'r')
    f2 = open(cmpTxt, 'r')
    
    cm_length = "None"
    
    cm_isFirst = False
    cm_firstLength = "None"
    
    cm_inst = "None"
    
    while True:
        line1 = f1.readline()
        while(not "CacheMiss:" in line1 and line1):
            line1=f1.readline()
            
        line2 = f2.readline()
        while(not "CacheMiss:" in line2 and line2):
            if("injectEarlySN" in line2):
                line2_SN = line2.strip().split(' ')
                injectSN = int(line2_SN[4])
                cm_isFirst = True
            line2=f2.readline()
        if not line2:break
        
        #No symptom in original but it occurs in corrupted run
        if not line1 and line2:
            line2_split = line2.strip().split(':')
            cm_length = str(int(line2_split[6])-injectSN)
            cm_inst = line2_split[8]
            if cm_isFirst:
                cm_isFirst = False
                cm_firstLength = str(int(line2_split[6])-injectSN)
            break
      
        line1_split = line1.strip().split(':')
        line2_split = line2.strip().split(':')
        #[4]=PC, [6]=SeqNum

        if cm_isFirst:
            cm_isFirst = False
            cm_firstLength = str(int(line2_split[6])-injectSN)
        
        if line1_split[4] == line2_split[4]:
            continue
        
        cm_length = str(int(line2_split[6])-injectSN)
        cm_inst = line2_split[8]
        break

    f1.close()
    f2.close()
    
    
    
    
    
    #print str(i) + "\t" + str(injectTime) + "\t" + str(injectLoc) + "\t" + ("F" if failure else "NF") + "\t" + valRuntime + "\t" + bm_type + "\t" + bm_length + "\t" + bm_predicted + "\t" + bm_actual + "\t" + ex_type + "\t" + ex_length + "\t" + cm_length
    #symptomf.write(str(i) + "\t" + str(injectTime) + "\t" + str(injectLoc) + "\t" + ("F" if failure else "NF") + "\t" + valRuntime + "\t" + injectEarlySN + "\t" + bm_type + "\t" + bm_length + "\t" + bm_predicted + "\t" + bm_actual + "\t" + ex_type + "\t" + ex_length + "\t" + cm_length + "\t" + bm_firstLength + "\t" + ex_firstLength + "\t" + cm_firstLength + "\n")
    
        
    symptomf.write(bench + "\t")
    symptomf.write(str(i) + "\t")
    symptomf.write(injectEarlySN + "\t")
    symptomf.write(injectArch + "\t")
    symptomf.write(str(injectTime) + "\t")
    symptomf.write(str(injectLoc) + "\t")
    symptomf.write(valFailure + "\t")
    symptomf.write(valRuntime + "\t")
    symptomf.write(bm_length + "\t")
    symptomf.write(ex_length + "\t")
    symptomf.write(cm_length + "\t")
    symptomf.write(bm_firstLength + "\t")
    symptomf.write(ex_firstLength + "\t")
    symptomf.write(cm_firstLength + "\t")
    symptomf.write(bm_predicted + "\t")
    symptomf.write(bm_actual + "\t")
    symptomf.write(bm_type + "\t")
    symptomf.write(bm_uselessLength + "\t")
    symptomf.write(bm_inst + "\t")
    symptomf.write(ex_type + "\t")
    symptomf.write(ex_inst + "\t")
    symptomf.write(cm_inst + "\n")
    
    
    
    
    
    
    os.system("rm " + cmpTxt)    
    

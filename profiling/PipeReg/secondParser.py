import os
import sys
import random
import re
import subprocess

input = sys.argv[1]
output = sys.argv[2]
start = int(sys.argv[3])
end = int(sys.argv[4])

inputFile1 = open(input+"_f1ToF2", 'r')
inputFile2 = open(input+"_f2ToD", 'r')
inputFile3 = open(input+"_dToE", 'r')
inputFile4 = open(input+"_eToF1", 'r')
inputFile5 = open(input+"_f2ToF1", 'r')
inputCountFile = open(input+"_count", 'r')

outputFile1 = open(output+"_"+str(start)+"_"+str(end)+".txt", 'w')
outputFile2 = open(output+"_"+str(start+10000)+"_"+str(end+10000)+".txt", 'w')
outputFile3 = open(output+"_"+str(start+20000)+"_"+str(end+20000)+".txt", 'w')
outputFile4 = open(output+"_"+str(start+30000)+"_"+str(end+30000)+".txt", 'w')
outputFile5 = open(output+"_"+str(start+40000)+"_"+str(end+40000)+".txt", 'w')






count_f1ToF2=int(inputCountFile.readline().split()[1])
count_f2ToD=int(inputCountFile.readline().split()[1])
count_dToE=int(inputCountFile.readline().split()[1])
count_eToF1=int(inputCountFile.readline().split()[1])
count_f2ToF1=int(inputCountFile.readline().split()[1])

random_f1ToF2=random.sample(xrange(0,count_f1ToF2), end-start)
random_f2ToD=random.sample(xrange(0,count_f2ToD), end-start)
random_dToE=random.sample(xrange(0,count_dToE), end-start)
random_eToF1=random.sample(xrange(0,count_eToF1), end-start)
random_f2ToF1=random.sample(xrange(0,count_f2ToF1), end-start)

#f1ToF2
lines = inputFile1.readlines()
for i in range(0,end-start):
    injectComp = 'f1ToF2'
    injectLoc = random.randrange(0,512)
    outputFile1.write(injectComp+"\t"+str(injectLoc)+"\t"+lines[random_f1ToF2[i]])
    
#f2ToD
lines = inputFile2.readlines()
for i in range(0,end-start):
    injectComp = 'f2ToD'
    injectLoc = random.randrange(0,64)
    outputFile2.write(injectComp+"\t"+str(injectLoc)+"\t"+lines[random_f2ToD[i]])
    
#dToE
lines = inputFile3.readlines()
for i in range(0,end-start):
    injectComp = 'dToE'
    injectLoc = random.randrange(0, 8 * 2 * (34+8) * 2)
    outputFile3.write(injectComp+"\t"+str(injectLoc)+"\t"+lines[random_dToE[i]])
    
#eToF1
lines = inputFile4.readlines()
for i in range(0,end-start):
    injectComp = 'eToF1'
    injectLoc = random.randrange(0,32)
    outputFile4.write(injectComp+"\t"+str(injectLoc)+"\t"+lines[random_eToF1[i]])
    
#f2ToF1
lines = inputFile5.readlines()
for i in range(0,end-start):
    injectComp = 'f2ToF1' 
    injectLoc = random.randrange(0,32)    
    outputFile5.write(injectComp+"\t"+str(injectLoc)+"\t"+lines[random_f2ToF1[i]])
    


    
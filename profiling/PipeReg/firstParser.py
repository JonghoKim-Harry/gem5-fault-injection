import os
import sys
import random
import re
import subprocess

input = sys.argv[1]
output = sys.argv[2]

inputFile = open(input,'r')
outputFile1 = open(output+"_f1ToF2", 'w')
outputFile2 = open(output+"_f2ToD", 'w')
outputFile3 = open(output+"_dToE", 'w')
outputFile4 = open(output+"_eToF1", 'w')
outputFile5 = open(output+"_f2ToF1", 'w')
outputCount = open(output+"_count", 'w')

inputLine = inputFile.readline()
inputLine = inputFile.readline()

linecount = [0,0,0,0,0]

while True:
    inputLine = inputFile.readline()
    if not inputLine:break
    line_split = inputLine.split()
    if len(line_split)<6: break
    time = line_split[0]
    
    if "line" in line_split[1]: #f1ToF2
        outputFile1.write(time+"\n")
        linecount[0]+=1
        
    if "inst" in line_split[2]: #f2ToD
        outputFile2.write(time+"\n")
        linecount[1]+=1
        
    if "op" in line_split[3]: #DtoE
        outputFile3.write(time+"\n")
        linecount[2]+=1
        
    if "addr" in line_split[4]: #eToF1
        outputFile4.write(time+"\n")
        linecount[3]+=1
        
    if "addr" in line_split[5]: #F2ToF1
        outputFile5.write(time+"\n")
        linecount[4]+=1
    
outputCount.write("f1ToF2    "+str(linecount[0])+"\n")
outputCount.write("f2ToD    "+str(linecount[1])+"\n")
outputCount.write("dToE    "+str(linecount[2])+"\n")
outputCount.write("eToF1    "+str(linecount[3])+"\n")
outputCount.write("f2ToF1    "+str(linecount[4])+"\n")

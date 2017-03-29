#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
using namespace std;

#define ARRAY_MAX 40000

int main(int argc, char* argv[])
{
	if(argc!=5)
	{
		cout<<"Please insert the name of benchmark and data/paddr and number of lines and number of injection\n";
		cout<<"Parser will open \"BENCHMARKNAME\"_(data/paddr)_input.txt\n";
		return 0;
	}
	
	string inputFileName(argv[1]);
	string outputFileName(argv[1]);
	int injectionType=-1;
	
	if(string(argv[2]).compare("data")==0)
	{
		inputFileName.append("_data_input.txt");
		outputFileName.append("_data.txt");
		injectionType=0;
	}
	else if(string(argv[2]).compare("paddr")==0)
	{
		inputFileName.append("_paddr_input.txt");
		outputFileName.append("_paddr.txt");
		injectionType=1;
	}
	else
	{
		//cout<<"Please insert to second argument as data/paddr\n";
		return 0;
	}

	
	 cout<<"Open File"<<endl;
    //ifstream input("stringsearch_data_input.txt");
    ifstream input(inputFileName);
    ofstream output(outputFileName);
	
    string line;
	
	

    //number of rows
    unsigned long long lowerBorder=0;
    unsigned long long upperBorder=std::stoull(string(argv[3]),NULL,10); 
    unsigned int num_of_injection = std::stoull(string(argv[4]),NULL,10); 
	upperBorder--; // 0 to n-1
	
    unsigned long long buffer[ARRAY_MAX];
    cout<<"Injection Type: "<<injectionType<<", upper_border="<<upperBorder<<endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis(lowerBorder, upperBorder);

    for(int i=0; i<num_of_injection; i++)
    {
		unsigned long long index=dis(gen);
		bool notRepe=true;
		for(int j=0; j<i; j++)
		{
			if(index==buffer[j])
			{
			notRepe=false;
			break;
			}
		}
		if(notRepe)
			buffer[i]=index;
		else
			i--;
	}

	sort(buffer, buffer+num_of_injection);

	ofstream debugger("debug.txt");
	for(int i=0; i<num_of_injection; i++)
	{
		debugger<<buffer[i]<<endl;
	}

	int j=0;
	for(int i=0; i<num_of_injection; i++)
	{
		unsigned long long target=buffer[i];
	for(;j<target;j++)
		getline(input, line);

	if(j!=target)
	{
		cout<<"Chech repetition\n";
		return 0;
	}

	getline(input, line);	
	j++;
	stringstream linestream(line);
		unsigned long long time;
	string buffer, dummy;
	int index,size,hasData,frag0,frag4,intDummy;
	linestream>>time;
	linestream>>buffer;
	linestream>>index;
	linestream>>dummy;
	linestream>>intDummy;
	linestream>>hasData;
	linestream>>frag0;
	linestream>>frag4;
	linestream>>intDummy;
	linestream>>size;

	int loc;
	if(injectionType==0)
	{
		if(hasData==1)
		{
		std::uniform_int_distribution<int> sizeDis(0, (size*8)-1);
		loc=sizeDis(gen);
		}
		else
		{
		if(frag4==1)
		{
			std::uniform_int_distribution<int> sizeDis(0, (size*8)-1);
			loc=sizeDis(gen);			
		}
		else
		{
			std::uniform_int_distribution<int> sizeDis(0, (size*4)-1);
			loc=sizeDis(gen);			
		}
		}
		loc+=index*128;
	}
	else if(injectionType==1)
	{
		std::uniform_int_distribution<int> sizeDis(0, 29);
		loc=sizeDis(gen)+64+index*128;
	}
	output<<time<<"\t"<<buffer<<"\t"<<loc<<endl;


	}

	return 0;
}

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
using namespace std;


int main(int argc, char* argv[])
{
	if(argc==1)
	{
		cout<<"Please insert the name of benchmark\n";
		cout<<"Parser will open \"BENCHMARKNAME\"_raw.txt\n";
		return 0;
	}
	
	string inputFileName(argv[1]);
	inputFileName.append("_raw.txt");
	string dataFileName(argv[1]);
	dataFileName.append("_data_input.txt");
	string paddrFileName(argv[1]);
	paddrFileName.append("_paddr_input.txt");
	
    ifstream input(inputFileName.c_str());
    ofstream data_output(dataFileName.c_str());
	ofstream paddr_output(paddrFileName.c_str());

    string line;
    
	int paddr_lineCount=0;
	int data_lineCount=0;
	while(!input.eof())
	{
		getline(input, line);
		stringstream linestream(line);
		
		string buffer;
		string outputString("");
		
		bool hasData=false;
		getline(linestream, buffer, '\t');
		for(int i=0; i<10; i++)
		{
			getline(linestream, buffer, '\t');
			if(i==5 || i==6) // size, frag[0]
			{
				if(buffer.compare("1")==0)
					hasData=true;
			}
			outputString.append(buffer);
			if(i<9)
				outputString.append("\t");
		}
                if(buffer.size()==0) break;

		paddr_output<<outputString<<endl;
		paddr_lineCount++;
		if(hasData)
		{
			data_output<<outputString<<endl;
			data_lineCount++;
		}
	
	}

	cout<<"paddr line count : "<<paddr_lineCount<<endl;
	cout<<"data line count : "<<data_lineCount<<endl;
	
    return 0;
}

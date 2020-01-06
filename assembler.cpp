#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
using namespace std;
int strNum2Int(string num);

int main(int argc, char *argv[]){
	//variables for parsing line
	string line;
	string ogLine;
	string opCode;
	string label;
	string arg;
	int labelEndIndex = 0;
	int commentStartIndex = 0;
	int opCodeVal = 0;
	int address = 0;
	//map to correspond string opCode ABC to int representation
	map<string, int> opCodes;
	//map to correspond labels to address
	map<string, int> labels;
	//used to set address of next instruction
	int orgVal = 0;
	//that value is extracted from this string
	string orgArg = "";
	//fill in opCode to int map
	//these values have arguments and are thus only 4 bits
	opCodes["DAT"] = 0;
	opCodes["HLT"] = 0;
	opCodes["EXT"] = 0;
	opCodes["LDA"] = 1;
	opCodes["LDI"] = 2;
	opCodes["STA"] = 3;
	opCodes["STI"] = 4;
	opCodes["ADD"] = 5;
	opCodes["SUB"] = 6;
	opCodes["JMP"] = 7;
	opCodes["JMZ"] = 8;
	opCodes["AND"] = 9;
	opCodes["IOR"] = 10;
	opCodes["XOR"] = 11;
	opCodes["ADL"] = 12;
	opCodes["ADC"] = 13;
	opCodes["SBB"] = 14;
	//the following values have no argument and thus are the full 8 bits
	opCodes["NEG"] = 240;
	opCodes["COM"] = 241;
	opCodes["CLR"] = 242;
	opCodes["SET"] = 243;
	opCodes["RTL"] = 244;
	opCodes["RTR"] = 245;
	opCodes["LSL"] = 246;
	opCodes["LSR"] = 247;
	opCodes["ASR"] = 248;
	opCodes["TST"] = 249;
	opCodes["CLC"] = 250;
	opCodes["SEC"] = 251;
	opCodes["TCA"] = 252;
	opCodes["TVA"] = 253;
	opCodes["JAL"] = 254;
	opCodes["NOP"] = 255;

	//We will parse data from input lines into this struct
	struct lineData {
		int opCode;
		string strOpCode;
		int numArg;
		string arg;
		string label;
		int address;
	};

	//We will push relevant lineData structs onto this vector to be written to out file later
	vector<lineData> instructions;	
	if(argc > 3){
		cerr<<"Error: Wrong number of arguments"<<endl;
		return 1;
	}
	istream *input;
	ifstream inf;
	if(argc > 1){
		//we have an input file argument
		inf.open(argv[1]);
		if(!inf.is_open()) {
			cerr<<"Error: Input file could not be opened"<<endl;
			return 2;
		}
		input = &inf;
	}else{
		//input comes from stdin
		input = &cin;

	}

	//parse data line by line
	while(getline(*input, line)){
		//reset previous loops values
		opCode ="";
		arg="";
		label = "";
		
		//if we are past 255 instructions and the next
		//line isnt an ORG
		//then the program is too big
		if(address>255 && line.find("ORG") == -1){
			cerr<<"Error: Program is too big"<<endl;
			return 4;
		}
		//COMMENT PARSE
		commentStartIndex = line.find(";");
		if(commentStartIndex != string::npos){
			//if there is a comment present
			//its useless so strip it off
			if(commentStartIndex > 0){
				line = line.substr(0,commentStartIndex-1);					
			}else{
				//need to be careful when stripping off a comment-only line
				line = "";
			}
		}

		//LABEL PARSE
		if(line.compare("") != 0 && !isspace(line.substr(0,1)[0])){
			//there is a label
			labelEndIndex = line.find(":");
			//it cant be the empty string
			if(labelEndIndex == 0){
					cerr<<"Error: an invalid label was encountered"<<endl;
					return 7;
			}
			label = line.substr(0,labelEndIndex);
			//we need to check all of the label's characters
			//to see if this is a valid label
			for(int i =0; i<label.length(); i++){
				char character = label.substr(i,1)[0];
				if(isalpha(character) == 0 && character != '_' && isdigit(character) == 0 || (i==0 && isalpha(character) == 0)){
					//invalid label
					cerr<<"Error: an invalid label was encountered"<<endl;
					return 7;
				}
			}
			//check to see if we had already seen this label
			if(labels.count(label) != 0){
				//we have seen this label before
				cerr<<"Error: an invalid label was encountered"<<endl;
				return 7;
					
			}
			//valid label
			//save the label in the map
			labels[label] = address;
			//strip it from the line 
			//to make the rest of parsing easier
			line = line.substr(labelEndIndex + 1,line.length());
		}

		//OP CODE PARSE
		if(line.length() > 3){
			//if there was an op code present
			//save it
			//OP code starts at first index of character
			int opCodeStartIndex = 0;
			for(int i =0; i<line.length(); i++){
				if(isalpha(line.substr(i,1)[0])){
					opCodeStartIndex = i;
					i = line.length();
				}
			}
			//now we know where opCode starts
			opCode = line.substr(opCodeStartIndex,3);
			//strip it from line so that only arg is left
			//(possibly)
			line = line.substr(opCodeStartIndex+3,line.length()-(opCodeStartIndex+3));
		}

		//ARG PARSE	
		if(line.length() > 0){				
			//if arg is present
			//save it
			//we must strip opcode-argument space seperator
			arg = line.substr(1,line.length());	
		}

		//at this point we have label, argument, op code saved into 
		//local varibales
		//lets validate them and put them into our struct
		if(opCode.compare("ORG") == 0){
			//ORG instruction get speical handeling
			//check to make sure no label at the front of this org instruction
			if(label.compare("") != 0){
				cerr<<"Error: an invalid label was encountered"<<endl;
				return 7;
			}

			//lets check to make sure org arg is constant
			if(arg.length() >0 && isalpha(arg.substr(0,1)[0])){
				cerr<<"Error: an invalid argument was encountered"<<endl;
				return 6;
			}
			orgVal = strNum2Int(arg);
			if(orgVal == -257){	
				cerr<<"Error: an invalid argument was encountered"<<endl;
				return 6;
			}
			//address of next instruction starts at org val
			address=orgVal;
		}else if(opCode.compare("") != 0){
			//This is a normal opCode
			//create new struct to push into vector
			lineData dat = lineData();
			//check valid opCode
			if(opCodes.count(opCode) == 0){
				//opCode not recognized
				cerr<<"Error: invalid op-code"<<endl;
				return 5;
			}
			//set opcode of struct
			dat.opCode = opCodes[opCode];
			//set string opcode of struct
			dat.strOpCode = opCode;
			//check address in bounds
			if(address > 255){
				cerr<<"Error: the program is too big"<<endl;
				return 4;
			}
			//set address
			dat.address = address;
			//now we have to deal with the (possible) argument
			if(arg.compare("") != 0){
				//if there is an argument
				//make sure this instruction should have an OP
				//CODE
				if(dat.strOpCode.compare("HLT") == 0 || dat.opCode >= 240){
					cerr<<"Error: Invalid Argument with this OP Code"<<endl;
					return 6;
				}
				
				//Is this argument a label or a number?
				if(isdigit(arg.substr(0,1)[0]) || arg.substr(0,1).compare("-") == 0 || arg.substr(0,1).compare("+") == 0){
					//its a number so convert it

					if(arg.substr(0,1).compare("+") == 0){
						//no need to keep + in front
						arg = arg.substr(1,arg.length() -1);
					}

					//convert str arg to num
					dat.numArg = strNum2Int(arg);
					//check to make sure arg in bounds
					//for its opcode
					if((dat.strOpCode.compare("DAT") == 0 &&
						//DAT
						(dat.numArg > 255 || dat.numArg < -128)) 
							|| 
					   (dat.strOpCode.compare("DAT") != 0 && 
					    	//NOT DAT
					    	(dat.numArg > 15 || dat.numArg < -8))){
							//DAT args are 0 to 255 (unsigned)
							//or -128 to 127 (signed)
							//All other op codes are
							// 0 to 15 (unsigned) or
							// -8 to 7 (signed)
							cerr<<"Error: a invalid argument was found"<<endl;
							return 6;
					}
					//this instruction does not have str arg
					dat.arg = "";
				}else{
					//this is a label refrence
					//so lets save it for later until 
					//we collected all labels
					dat.arg = arg;
				}

			}
			//push struct into vector for later
			instructions.push_back(dat);
			//increment address for next time
			address++;
			
		}else{
			//meaningless line
			//such as comment or blank line

		}
	}
	
	//now we have to output the data as a binary file
	//lets store data into this array to output later
	unsigned char outData[256] = {};
	//for all the instructions we gathered
	for(int i =0; i<instructions.size(); i++){
		//helpful local variable 'data' for this loop
		lineData data = instructions[i];
		//Do we need to resolve label argument?
		if(data.arg.compare("") != 0){
			//lets check if we have seen this label argument before
			if(labels.count(data.arg) == 0){
				cerr<<"Error: Unresolved label refrence"<<endl;
				return 8;
			}
			//this used a label refrence as the argument
			//lets get its value
			if(data.strOpCode.compare("EXT") == 0){
				//if its an EXT we take upper 4 bits
				data.numArg = (uint8_t)((labels[data.arg])/16);
			}else{
				//all other instructions take lower 4 bits
				//which will be extracted when we % 16 in
				//a few lines
				data.numArg = labels[data.arg];
			}
		}

		//if its an op code with an argumet
		if(data.opCode<240){
			//make the 4 bit opCode into a full 8 bits
			data.opCode = data.opCode * 16;
		}

		//if the opCode is not DAT(0)
		if(data.strOpCode.compare("DAT") != 0){
			//the argument is limited to 4 bits
			data.numArg = data.numArg % 16;

			//takes care of negative argument
			if(data.numArg < 0){
				data.numArg = data.numArg + 16;
			}
		}
		//EXT 0 check
		if(data.strOpCode.compare("EXT") == 0 && data.numArg == 0){
			cerr<<"Error: invalid op-code"<<endl;
			return 5;
		}
		//save in array for output later
		outData[data.address] = ((uint8_t)(data.opCode + data.numArg));
	}
	//Where should we output this data
	if(argc == 3){
		//output to outfile from argument
		ofstream outfile;
		outfile.open(argv[2],std::ofstream::out | std::ofstream::trunc);
		//check sucessful open
		if(!outfile.is_open()){
			cerr<<"Error: output file cannot be opened"<<endl;
			return 3;
		}
		//output
		for(int i = 0; i<256; i++){	
			outfile<<outData[i];
		}

		outfile.close();	
	}else{
		//we output to cout
		for(int i = 0; i<256; i++){	
			cout<<outData[i];
		}
	}
}
//Helper function to convert string numbers to ints
int strNum2Int(string num){
	//this returns -257 for bad num values
	//code in main will use this fact and and throw errors accordingly
	int toReturn;
	if(num.length() == 0){
		return 0;

	}

	//check for funny bussiness such as decimals and what not
	if(num.find(".") != -1 || num.find("\t") != -1 || num.find(" ") != -1 
			|| num.find("\n")!= -1){
		//this is not an integer
		return -257;

	}

	//NOTE: i did not know doing strtol(num.c_str(),nullptr,0)
	//coverted it automataically
	//would have been helpful but alas I worked around it
	if(num.substr(0,1).compare("0") == 0 && num.length() > 1){
		//just decimal or octal
		if(num.substr(0,2).compare("0x")== 0){
			//hex
			try{
				toReturn = strtol(num.c_str(),nullptr,16);
			}catch(int e){
				return -257;
			}

		}else{
			//octal
			try{
				toReturn = strtol(num.c_str(),nullptr,8);
			}catch(int e){
				return -257;
			}

		}		
	}else{
		//decimal
		try{
			toReturn = strtol(num.c_str(),nullptr,10);
		}catch(int e){
			return -257;
		}
	}
	if(toReturn > 255){
		return -257;
	}else{
		return toReturn;
	}

}


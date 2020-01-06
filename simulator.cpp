#include <iostream>
#include <fstream>
#include <string>
using namespace std;
string dec2hex(int d);
bool validateAddress(int add, int numRead);
int andBinarys(int num1, int num2);
int orBinarys(int num1, int num2);
int xorBinarys(int num1, int num2);

int main(int argc, char *argv[]){
	size_t numBytes= 256;
	unsigned char data[numBytes];
	int numdata[numBytes];
	size_t numRead;

	if(argc > 3){
		cerr<<"The wrong number of arguments were given."<<endl;
		return 1;
	}

	if(argc > 1){
		//arguments present
		//so we get input from file
		string infilename = argv[1];
		ifstream inf(infilename.c_str());
		if(inf.fail()){
			cerr<<"open failed"<<endl;
			return 2;
		}
		inf.read((char*) &data[0], numBytes);
		numRead = inf.gcount();

	}else{
		//get input from stdin
		cin.read((char *) &data[0], 256);
		numRead = cin.gcount();
	}

	//converts unsigned char to int
	for(int i = 0 ; i<numRead; i++){
		numdata[i] = data[i];
	}

	int byteVal;
	int lowerNibble;
	int upperNibble;
	uint8_t accumulatorVal = 0;
	int extVal = 0;
	string opCode ="";
	string argument="";
	int argumentVal = 0;
	int carry = 0;
	string memAdd = "";
	string hexArg = "";
	string accumulator="ACC=0x";
	//this is the simulation
	//loop over instructions
	for(int i = 0; i< numRead; i++){
		byteVal = numdata[i];
		lowerNibble = byteVal % 16;
		upperNibble = byteVal/16;
		memAdd = "0x" + dec2hex(i) + "  ";
		opCode ="   ";
		//add extension to argument
		argumentVal = lowerNibble+extVal;
		argument = "0x" + dec2hex(argumentVal) + "    ";
		//reset EXT val since it only carries over once
		extVal = 0;
		if(byteVal == 0){
			//HLT
			//output out file
			argument = "        ";
			cout<<memAdd<<"HLT"<<" "<<argument<<accumulator<<dec2hex(accumulatorVal)<<endl;
			//convert back from ints to unsigned chars
			for(int j = 0; j<numRead; j++){
				data[j] = numdata[j];
			}
			//we have a second argument (the out file)
			if(argc > 2){
				ofstream outfile;
				outfile.open(argv[2]);
				if(!outfile.is_open()){
					cerr<<"the output file cannot be opened or it cannot be written"<<endl;
					return 3;
				}
				//print our data
				for(int j = 0; j<numRead; j++){
					outfile<<data[j];
				}
				outfile.close();	
			}
			//successful exit
			return 0;
		}else if(byteVal > 0 && byteVal <= 15){
			//EXT
			opCode = "EXT";
			extVal = byteVal * 16;
		}else if(upperNibble == 1){
			//LDA
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			opCode = "LDA";
			accumulatorVal = numdata[argumentVal];
		}else if(upperNibble == 2){
			//LDI
			//check if first read is in bounds
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			//check if second read is in bounds
			if(!validateAddress(numdata[argumentVal], numRead)){	
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			opCode = "LDI";
			accumulatorVal = numdata[numdata[argumentVal]];
		}else if(upperNibble == 3){
			//STA
			if(!validateAddress(argumentVal, numRead)){
				//attempt to write past EOF
				cerr<<"Error: Attempt to write past EOF"<<endl;
				return 6;
			}
			opCode = "STA";
			numdata[argumentVal] = accumulatorVal;
		}else if(upperNibble == 4){
			//STI
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			if(!validateAddress(numdata[argumentVal], numRead)){
				//attempt to write past EOF
				cerr<<"Error: Attempt to write past EOF"<<endl;
				return 6;
			}
			opCode = "STI";
			numdata[numdata[argumentVal]] = accumulatorVal;
		}else if(upperNibble == 5){
			//ADD
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			if(accumulatorVal + numdata[argumentVal] > 255){
				carry = 1;
			}else{
				carry = 0;
			}
			accumulatorVal = accumulatorVal + numdata[argumentVal];
			opCode = "ADD";
		}else if(upperNibble == 6){
			//SUB
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			uint8_t complement = ~numdata[argumentVal];
			//cout<<"acc :"<<int(accumulatorVal)<<endl;
			//cout<<"complement :"<<int(complement)<<endl;
			//cout<<"sum :"<<int(complement + accumulatorVal + 1)<<endl;
			if(accumulatorVal + complement + 1 > 255){
				carry = 1;
			}else{
				carry = 0;
			}
			accumulatorVal = accumulatorVal + complement + 1;
			opCode = "SUB";
		}else if(upperNibble == 7){
			//JMP
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to execute code past EOF"<<endl;
				return 4;
			}
			i = argumentVal - 1;
			opCode = "JMP";
		}else if(upperNibble == 8){
			//JMZ
			if(accumulatorVal == 0){
				if(!validateAddress(argumentVal, numRead)){
					//attempt to read past EOF
					cerr<<"Error: Attempt to execute code past EOF"<<endl;
					return 4;
				}
				i = argumentVal - 1;
			}
			opCode = "JMZ";
		}else if(upperNibble == 9){
			//AND
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			accumulatorVal = andBinarys(accumulatorVal, numdata[argumentVal]);
			opCode = "AND";
		}else if(upperNibble == 10){
			//IOR
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			accumulatorVal = orBinarys(numdata[argumentVal], accumulatorVal);
			opCode = "IOR";
		}else if(upperNibble == 11){
			//XOR
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			accumulatorVal = xorBinarys(numdata[argumentVal], accumulatorVal);
			
			opCode = "XOR";
		}else if(upperNibble == 12){
			//ADL
			if(argumentVal < 16){
				//then this ADL was not precceded by EXT
				//so need to sign extend
				if(argumentVal > 8){
					//then MSB in lower nibble is 1
					//so we extend with 1s
					argumentVal = argumentVal + 240;
				}
			}

			argument = "0x" + dec2hex(argumentVal) + "    ";
			if(accumulatorVal + argumentVal > 255){
				carry = 1;
			}else{
				carry = 0;
			}
			accumulatorVal = accumulatorVal + argumentVal;
			opCode = "ADL";
		}else if(upperNibble == 13){
			//ADC
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			int oldCarry = carry;
			if(accumulatorVal + numdata[argumentVal] > 255){
				carry = 1;
			}else{
				carry = 0;
			}
			
			accumulatorVal = accumulatorVal + numdata[argumentVal] + oldCarry;
			opCode = "ADC";
		}else if(upperNibble == 14){
			//SBB
			
			if(!validateAddress(argumentVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to read past EOF"<<endl;
				return 5;
			}
			
			int oldCarry = carry;
			uint8_t complement = ~numdata[argumentVal];
			if(accumulatorVal + complement + oldCarry > 255){
				carry = 1;
			}else{
				carry = 0;
			}
			accumulatorVal = accumulatorVal + complement + oldCarry;
			opCode = "SBB";
		}else if(byteVal == 240){
			//NEG
			accumulatorVal = (~accumulatorVal) + 1;
			opCode = "NEG";
			argument = "        ";
		}else if(byteVal == 241){
			//COM
			accumulatorVal = ~accumulatorVal;
			opCode = "COM";
			argument = "        ";
		}else if(byteVal == 242){
			//CLR
			opCode = "CLR";
			accumulatorVal = 0;
			argument = "        ";
		}else if(byteVal == 243){
			//SET
			accumulatorVal = 255;
			opCode = "SET";
			argument = "        ";
		}else if(byteVal == 244){
			//RTL
			uint8_t numShiftLeft1 = accumulatorVal * 2;
			uint8_t numShiftRight7 = accumulatorVal/128;
			accumulatorVal = orBinarys(numShiftLeft1,numShiftRight7);
			opCode = "RTL";
			argument = "        ";


		}else if(byteVal == 245){
			//RTR
			uint8_t numShiftRight1 = accumulatorVal/2;
			uint8_t numShiftLeft7 = accumulatorVal * 128;
			accumulatorVal = orBinarys(numShiftRight1,numShiftLeft7);
			argument = "        ";
			opCode = "RTR";
		}else if(byteVal == 246){
			//LSL
			accumulatorVal = accumulatorVal*2;
			argument = "        ";
			opCode = "LSL";
		}else if(byteVal == 247){
			//LSR
			accumulatorVal = accumulatorVal/2;
			argument = "        ";
			opCode = "LSR";
		}else if(byteVal == 248){
			//ASR
			if(accumulatorVal > 127){
				accumulatorVal = accumulatorVal/2 + 128;
			}else{
				accumulatorVal = accumulatorVal/2;
			}
			argument = "        ";
			opCode = "ASR";
		}else if(byteVal == 249){
			//TST
			if(accumulatorVal != 0){
				accumulatorVal = 1;
			}
			argument = "        ";
			opCode ="TST";
		}else if(byteVal == 250){
			//CLC
			carry = 0;
			opCode = "CLC";
			argument = "        ";
		}else if(byteVal == 251){
			//SEC
			carry = 1;
			opCode = "SEC";
			argument = "        ";
		}else if(byteVal == 252){
			//TCA
			accumulatorVal = carry;
			argument = "        ";
			opCode = "TCA";
		}else if(byteVal == 253){
			//TVA
			accumulatorVal = carry ^ (accumulatorVal/128);
			argument = "        ";
		}else if(byteVal == 254){
			//JAL
			if(!validateAddress(accumulatorVal, numRead)){
				//attempt to read past EOF
				cerr<<"Error: Attempt to execute code past EOF"<<endl;
				return 4;
			}
			int oldPc = i;
			i = accumulatorVal - 1;
			argument = "        ";
			accumulatorVal = oldPc + 1;
			opCode ="JAL";
		}else{
			//NOP
			argument = "        ";
			opCode = "NOP";
			//cout<<"NOP? "<<byteVal<<endl;
		}
		cout<<memAdd<<opCode<<" "<<argument<<accumulator<<dec2hex(accumulatorVal)<<endl;
		if(i + 1 == numRead){
			i = -1;
		}
	}
	return 0;
}

string dec2hex(int d){
	string hex2asc = "0123456789ABCDEF";
	if(d>255){
		return "ZZ";
	}
	string firstDigit = hex2asc.substr(d/16,1);
	string secondDigit = hex2asc.substr(d-((d/16)*16),1);
	return (firstDigit + secondDigit);
}

bool validateAddress(int add, int numRead){
	if(add>=numRead || add<0){
		return false;
	}
	return true;
}

int andBinarys(int num1, int num2){
	int number1 = num1;
	int number2 = num2;
	int digit1;
	int digit2;
	int result = 0;
	for(int i = 128; i>=1; i = i/2){
		digit1 = number1/i;
		digit2 = number2/i;
		if(digit1 == digit2 && digit1 == 1){
			result = result + i;
		}
		number1 = number1%i;
		number2 = number2%i;
	}
	return result;
}

int orBinarys(int num1, int num2){
	int number1 = num1;
	int number2 = num2;
	int digit1;
	int digit2;
	int result = 0;
	for(int i = 128; i>=1; i = i/2){
		digit1 = number1/i;
		digit2 = number2/i;
		if(digit1 == 1 ||digit2 == 1){
			result = result + i;
		}
		number1 = number1%i;
		number2 = number2%i;
	}
	return result;
}

int xorBinarys(int num1, int num2){
	int number1 = num1;
	int number2 = num2;
	int digit1;
	int digit2;
	int result = 0;
	for(int i = 128; i>=1; i = i/2){
		digit1 = number1/i;
		digit2 = number2/i;
		if((digit1 == 1 || digit2 == 1) && !(digit1 == 1 && digit2 == 1)){
			result = result + i;
		}
		number1 = number1%i;
		number2 = number2%i;
	}
	return result;
}


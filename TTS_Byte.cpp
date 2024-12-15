#include <fstream>
#include <iostream>
#include <vector>
#include <experimental/filesystem>
#include <algorithm>
#include <bitset>
#include <math.h>
#include <algorithm>

using namespace std;

vector<wstring> filenames;
char hexS[32768];
char hexT[32768];
char hexU[32768];
bool one = false;
int fir = 1;
int sec = 0;

char convs[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void getFilenames(){
    filenames.resize(0);
    int s = 0;
    for (auto& entry : std::experimental::filesystem::directory_iterator("TTS_AI/")){ // For every file in directory
        filenames.resize(filenames.size()+1); // Resize to 1 above before size
        filenames[s] = entry.path().wstring(); // Store path in variable
        s ++;
    }

}

void getHex(wstring fileName, char * hexx){
    char bytes[16384];
    ifstream fileIn(fileName.c_str());

    fileIn.seekg(0, std::ios::end); // Go to the end of the file
    size_t length = fileIn.tellg(); // Store the length of the file
    fileIn.seekg(0, std::ios::beg); // Go to beginning of file
    fileIn.read(bytes, length); // Read bytes of file into byte array
    fileIn.close();

    int s = 0;
    int b = 0;
    bool first = true;
    for (int i = 0; i < 16384; i ++){ // Bytes
        for (int j = 0; j < 2; j ++){
            for (int k = 0; k < 4; k ++){
                if (bitset<8>(bytes[i])[7-j*4-k] == 1){
                    s += pow(2, 3-k);
                }
                if (k == 3){
                    //fileO << s << endl;
                    if (first) { // If first part of byte
                        hexx[b*2] = convs[s];
                        first = false;
                        if (i > length) hexx[b*2] = convs[0];
                    }
                    else{
                        hexx[b*2+1] = convs[s];
                        first = true;
                        if (i > length) hexx[b*2+1] = convs[0];
                        b ++;
                    }
                    s = 0;
                }
            }
        }
    }
}

void makeFile(char * hexx){
    ofstream file(L"TTS_Hex.txt");
    bool first = true;
    for (int i = 0; i < 32768; i ++){
        file << hexx[i];
        if (first) first = false;
        else {
            if ((i+1)%64 == 0) file << endl;
            else file << " ";
            //cout << (int)i+1 % 2 << " " << i + 1 << endl;
            first = true;
        }
    }
    file << endl;
    file.close();
}

void getBytes(wstring fileName){
    char bytes[16384];
    ifstream file(fileName.c_str());
    file.seekg(0, std::ios::end); // Go to the end of the file
    size_t length = file.tellg(); // Store the length of the file
    file.seekg(0, std::ios::beg); // Go to beginning of file
    file.read(bytes, length); // Read bytes of file into byte array
    file.close();

    ofstream fileO("TTS_Hex_Byte.txt");
    for (int i = 0; i < length; i ++){
        fileO << (int)bytes[i] << " ";
        if ((i+1) % 32 == 0) fileO << endl;
    }
    fileO.close();
}

void compareSave(char * hex1, char * hex2){
    ofstream file(L"TTS_Hex_Comp.txt");
    bool first = true;
    for (int i = 0; i < 32768; i ++){
        if (hex1[i] == hex2[i]) hexU[i] = 'O';
        else hexU[i] = 'X';
        file << hexU[i];
        if (first) first = false;
        else {
            if ((i+1)%64 == 0) file << endl;
            else file << " ";
            first = true;
        }
    }
    file << endl;
    file.close();
}

string wstrToStr(wstring wstr){
    string str = "";
    for (int i = 0; i < wstr.size(); i ++){
        str += wstr[i];
    }
    return str;
}

void compare(char * hex1, char * hex2){
    for (int i = 0; i < 32768; i ++){
        if (hex1[i] != hex2[i]) hexU[i] = 'X'; // Sets not matching hex to not same
        if ((hex1[i] != hex2[i]) && i == 0) cout << wstrToStr(filenames[fir]) << " " << wstrToStr(filenames[sec]) << endl;
    }
}

int main(){
    /* for (int i = 0; i < 32768; i ++){ // Sets every hex to same
        hexU[i] = 'O';
    }
    getFilenames();
    getHex(filenames[sec], hexT);
    for (int i = 0; i < filenames.size(); i ++){ // Compares every file for best accuracy
        if (one == false) getHex(filenames[fir], hexS);
        if (one) getHex(filenames[sec], hexT);
        compare(hexS, hexT);
        if (one){
            one = false;
            fir += 2;
        }
        else{
            one = true;
            sec += 2;
        }
        if (fir >= filenames.size()) fir = 1;
        if (sec >= filenames.size()) sec = 0;
    }
    ofstream file("TTS_Except.txt");
    for (int i = 0; i < 32768; i ++){ // For every hex
        if (hexU[i] == 'O'){ // If hex never differed
            file << to_string(i) << ",";
        }
    }
    file << endl;
    file.close(); */
    vector<int> exceptions;
    string lines[1];
    ifstream file("TTS_Except.txt");
    file >> lines[0];
    file.close();
    string plc = "";
    for (int i = 0; i < lines[0].length(); i ++){
        if (lines[0][i] == ','){
            exceptions.resize(exceptions.size()+1);
            exceptions[exceptions.size()-1] = stoi(plc);
            plc = "";
        }
        else plc += lines[0][i];
    }
    cout << exceptions.size();
    //getBytes(filenames[0]);
    return 0;
}
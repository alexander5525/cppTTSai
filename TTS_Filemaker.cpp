#include <fstream>
#include <iostream>
#include <vector>
#include <experimental/filesystem>

using namespace std;

vector<wstring> filenames;
vector<int> exceptions;
char bytes[16384];

void getExceptions(){ // Bytes which never change in an ogg file (constants)
    string lines[1];
    ifstream file("TTS_Except.txt");
    file >> lines[0];
    string plc = "";
    for (int i = 0; i < lines[0].length(); i ++){
        if (lines[0][i] == ','){
            exceptions.resize(exceptions.size()+1);
            exceptions[exceptions.size()-1] = stoi(plc);
            plc = "";
        }
        else plc += lines[0][i];
    }
    file.close();
}

void getFilenames(){
    filenames.resize(0);
    int s = 0;
    int b = 0;
    wstring plc = L"";
    wofstream file(L"TTS_Sheet.txt");
    for (auto& entry : std::experimental::filesystem::directory_iterator("TTS_AI_text/")){ // For every file in directory
        filenames.resize(filenames.size()+1); // Resize to 1 above before size
        filenames[s] = entry.path().wstring(); // Store path in variable
        for (int i = 14; i < filenames[s].length()-4; i ++){ // Remove the .txt ending
            plc += filenames[s][i];
        }
        filenames[s] = plc;
        plc = L"";
        wifstream fileIn(entry.path().wstring().c_str());
        wstring letters[1];
        wstring check = L"-)%&§]([";
        int found = 0;
        fileIn >> letters[0];
        fileIn.close();
        file << filenames[s];
        file << "\t" << letters[0] << "\t";
        /* for (int j = 0; j < letters[0].length(); j ++){
            check = L"-)%&§]([";
            found = check.find(letters[0][j]);
            if (found != std::string::npos){
                file << "0" << to_wstring(found) << ",";
            }
            check = L"nm{}kgxhs z2bpl5";
            found = check.find(letters[0][j]);
            if (found != std::string::npos){
                file << "1" << to_wstring(found) << ",";
            }
            check = L"rwv7c$y !   dtf";
            found = check.find(letters[0][j]);
            if (found != std::string::npos){
                file << "2" << to_wstring(found) << ",";
            }
            check = L"a ei3o06 <u# ";
            found = check.find(letters[0][j]);
            if (found != std::string::npos){
                file << "3" << to_wstring(found) << ",";
            }
            if (letters[0][j] == 164){ // ä
                file << "31,";
            }
            if (letters[0][j] == 188){ // ö
                file << "38,";
            }
            if (letters[0][j] == 182){ // ü
                file << "312,";
            }
            if (letters[0][j] == 159){ // ß
                file << "19,";
            }
        } */
        wstring name = L"TTS_AI/";
        name += filenames[s];
        name += L".ogg";
        ifstream fil(name.c_str());
        if (fil.is_open() != true) cerr << name.c_str() << " does not exist.";
        fil.seekg(0, std::ios::end); // Go to the end of the file
        size_t length = fil.tellg(); // Store the length of the file
        fil.seekg(0, std::ios::beg); // Go to beginning of file
        fil.read(bytes, length); // Read bytes of file into byte array
        fil.close();
        b = 0;
        bool yet = false; // When the file is finished set size scalar and never thereafter
        bool no = false;
        for (int i = 0; i < 16384; i ++){ // Bytes
            for (int j = 0; j < 2; j ++){
                for (int e = 0; e < exceptions.size(); e ++) if (i*2+j == exceptions[e]){
                    no = true;
                    break;
                }
                if (i > length) {
                    if (yet != true) {
                        file << (float)b / (float)23979;
                        yet = true;
                        break;
                    }
                }
                if(!no) b ++;
                else no = false;
            }
            if (yet) break;
        }
        file << endl;
        s ++;
    }
    file.close();

}

int main(){
    getExceptions();
    getFilenames();
    /* ofstream file("TTS_Sheet.txt");
    for (int i = 0; i < filenames.size(); i ++){
        ifstream fileIn("TTS_AI_text/" + filenames[i] + ".txt");
        string letters[1];
        fileIn >> letters[0];
        file << filenames[i];
        file << "\t" << letters[0] << endl;
        fileIn.close();
    }
    file.close(); */
}

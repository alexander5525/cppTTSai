#include <vector>
#include <iostream>
#include <bitset>
#include <math.h>
#include <fstream>

using namespace std;

vector<wstring> filenames;
vector<int> exceptions;

char bytes[16384]; // What will be part of the real file
float bits[32768]; // Generated and gathered fake bits
float inps[43];
char convs[4];
float bitConvs[16];
float siz;

float betterE(float num){
    if (num > 88.722839) return pow(2, 127);
    else if (num < -87.3365447) return pow(2, -125);
    else return exp(num);
}

float toFloat(char * bytes){
    float fract = 1;
    int expon = 0;
    float num;
    int bits[32];
    int s;
    for (int i = 0; i < 4; i ++){
        s = (int)bytes[i];
        if (s < 0){ // weird thing where the first bit counts as -128
            bits[i*8] = 1;
            s += 128;
        }
        else bits[i*8] = 0;
        for (int b = 0; b < 8; b ++){
            if (b != 0){
                if (s - pow(2, 7-b) < 0) bits[i*8+b] = 0;
                else{
                    bits[i*8+b] = 1;
                    s -= pow(2, 7-b);
                }
            }
        }
    }
    for (int i = 0; i < 32; i ++){ // Checking for zero where all bits are 0
        if (bits[i] == 1) break;
        if (i == 31) return 0.0;
    }
    for (int i = 1; i < 8; i ++){ // checking for +-inf where all exponent bits are 1 and fraction bits are 0
        if (bits[i] != 1) break;
        if (i == 7){
            for (int j = 9; j < 32; j ++){
                if (bits[j] == 1) break; // when fraction bits are not all 0 then return nan
                if (j == 31){
                    if (bits[0] == 1) return -INFINITY; //negative
                    else return INFINITY; // positive
                }
            }
            return NAN;
        }
    }
    for (int i = 0; i < 8; i ++){
        if (bits[1+i] == 1) expon += (int)pow(2, 7-i);
    }
    for (int i = 0; i < 23; i ++){
        if (bits[9+i] == 1) fract += pow(2, -i-1);
    }
    num = pow(-1, bits[0])*pow(2, expon-127)*fract;
    return num;
}

int toInt(char * bytes){
    int out = 0;
    int bits[32];
    int s;
    for (int i = 0; i < 4; i ++){
        s = (int)bytes[i];
        if (s < 0){ // weird thing where the first bit counts as -128
            bits[i*8] = 1;
            s += 128;
        }
        for (int b = 0; b < 8; b ++){
            if (b != 0){
                if (s - pow(2, 7-b) < 0) bits[i*8+b] = 0;
                else{
                    bits[i*8+b] = 1;
                    s -= pow(2, 7-b);
                }
            }
        }
    }
    for (int i = 1; i < 32; i ++){
        if (bits[i] == 1) out += pow(2, 31-i);
    }
    out *= pow(-1, bits[0]);
    return out;
}

class Neuron {
    public:
        float act = 0.5; // Activation of the Neuron between 0 and 1
        int layer; // Layer where the neuron is
        vector<float> weight; // Weights of each connection
        float bias; // Bias of the neuron
        int pCount; // Number of neurons in previous layer
        vector<float> desWei; // The desired weight change
        float desBias; // Desired bias change
        float desAct; // Desired change in activation (act wont be affected only the weights and biases)
        //vector<int> connections; // Indices of neurons, to which it's connected                               // Everything to do with connections has been commented out

        void init(int pCou){
            pCount = pCou;
            weight.resize(pCou); // Size of weight array is equal to size of neuron array in previous layer
            desWei.resize(pCou);
            bias = 0;
            desBias = 0;
            desAct = 0;
            for (int we = 0; we < pCou; we ++){
                weight[we] = 0;
                desWei[we] = 0;
            }
        }

        void calc(float mid){ // Calculates the activation by putting the sum of all weights and bias into the sigmoid
            act = (float)1/(float)(1+betterE(-mid)); // Sigmoid function
            if(isnan(act)) cerr << "Error: Activation results in nan" << endl;
        }

};

class Layer{
    public:
        vector<Neuron> neurons; // Array, which contains the neurons of the layer
        int layer; // Layer in the network
        int nCount; // Neurons in Layer

        void initialize(int neur_num, int lay, vector<Layer> layers){ // Will initialize the neurons
            layer = lay;
            nCount = neur_num;
            neurons.resize(neur_num);
            for (int i = 0; i < neur_num; i ++){
                neurons[i].layer = lay;
                if (lay != 0){
                    neurons[i].init(layers[lay-1].nCount); // It the layer is not the first one, then the neurons will get bias and weight values assigned
                }
            }
        }

        void calcLay(Layer lay /*lay is the previous layer*/){
            float mid = 0;
            for (int n = 0; n < nCount; n ++){
                for (int i = 0; i < lay.nCount; i ++){
                    mid += lay.neurons[i].act*neurons[n].weight[i];
                }
                mid += neurons[n].bias;
                neurons[n].calc(mid);
                mid = 0;
            }
        }

};

class NeuralNetwork{
    public:
        vector<Layer> layers; // The layers inside the neural network
        int lCount; // Number of layers
        float totCost = 0; // Cost of the network
        int batchSize = 1;

        void initAll(int size, int laySize[]){ // Initializes the network by
            lCount = size;
            layers.resize(size);
            for (int i = 0; i < size; i ++){
                layers[i].initialize(laySize[i], i, layers);
            }
        }

        void calculate(){ // Will calculate the activation of the neurons from the second layer onward
            for (int i = 1; i < lCount; i++){
                layers[i].calcLay(layers[i-1]);
            }
        }

        void input(float inp[], int scalar = 1){ // Will set the activation of the neurons in the input layer to the array given
            // The scalar is useful for converting a number line from 0 to n into 0 to 1 if n is given as the scalar
            for (int i = 0; i < layers[0].nCount; i++){
                layers[0].neurons[i].act = inp[i] / scalar;
            }
        }

        void loadNetByte(){
            ifstream file("TTS_AI_V1.txt");
            file.seekg(0, std::ios::end);
            size_t length = file.tellg();
            file.seekg(0, std::ios::beg);
            char* buffer = new char[(int)length];
            file.read(buffer, length);
            file.close();
            char bytes[4];
            int b = 0; // current byte
            int stage = 0; // stage of reading
            int p = 0;
            int ntSiz = 0;
            int l = 1;
            int w = 0;
            int total = 1;
            int current = 0;
            vector<int> sizes;
            for (int i = 0; i < length; i ++){
                if (b == 3){
                    bytes[b] = buffer[i];
                    if (stage == 0){ // layer count
                        ntSiz = toInt(bytes);
                        total += ntSiz;
                        sizes.resize(ntSiz);
                        stage ++;
                        current ++;
                    }
                    else if (stage == 1){ // layer sizes
                        sizes[p] = toInt(bytes);
                        p ++;
                        if (p == ntSiz){
                            int temp[ntSiz]; // What will be passed into the network
                            for (int i = 0; i < ntSiz; i ++){
                                temp[i] = sizes[i]; // Converts the vector into an array
                                total += sizes[i];
                            }
                            initAll(ntSiz, temp);
                            p = 0;
                            stage ++;
                        }
                        current ++;
                    }
                    else if (stage == 2){ // weights & biases
                        // l is current layer
                        // p is current neuron
                        // w is current weight
                        if (w == layers[l-1].nCount){
                            layers[l].neurons[p].bias = toFloat(bytes);
                            current ++;
                            /* if (current % 100 == 0){
                                system("cls");
                                cout << "Loading..." << endl;
                                cout << (int)100*((float)current/(float)total) << "% complete ";
                                for (int i = 0; i < 10; i ++){
                                    if (i < (int)(10*((float)current/(float)total))) cout << "[#]";
                                    else cout << "[]";
                                }
                                cout << endl;
                                //cout << "Neuron " << n << "..." << endl;
                            } */
                            w = 0;
                            p ++;
                            if (p == layers[l].nCount){
                                p = 0;
                                l ++;
                            }
                        }
                        else{
                            layers[l].neurons[p].weight[w] = toFloat(bytes);
                            w ++;
                        }
                    }
                    b = 0;
                }
                else{
                    bytes[b] = buffer[i];
                    b ++;
                }
            }
            //system("cls");
        }

        void makeOutput(){
            int n = 0;
            size_t length;
            ifstream file("TTS_AI/1000_0.ogg"); // Open a random file
            if (file.is_open() != true) cerr << "File doesn't exist.";
            file.seekg(0, std::ios::end); // Go to the end of the file
            length = file.tellg(); // Store the length of the file
            file.seekg(0, std::ios::beg); // Go to beginning of file
            file.read(bytes, length); // Read bytes of file into byte array
            file.close();
            int s = 0;
            float plc;
            int bibs[8]; // Actual bits
            bool exc = false;
            for (int i = 0; i < 16384; i ++){
                for (int j = 0; j < 2; j ++){
                    exc = true;
                    for (int e = 0; e < exceptions.size(); e ++) {
                        if (i*2+j == exceptions[e]) break;
                        else if (e == exceptions.size()-1) exc = false;
                    }
                    if (exc){
                        for (int k = 0; k < 4; k ++) if (bitset<8>(bytes[i])[7-j*4-k] == 1) s += pow(2, 3-k); // Exception
                        bits[i*2+j] = bitConvs[s];
                        s = 0;
                    }
                    else{
                        for (int l = 0; l < 16; l ++){ // Not exception
                            if (layers[lCount-1].neurons[n].act < (float)(1+2*l)/(float)32) {
                                plc = l;
                                break;
                            }
                            if (l == 15) plc = 15;
                        }
                        bits[i*2+j] = plc/(float)16;
                        n ++;
                    }
                }
            }
            s = 0;
            for (int i = 0; i < 32768; i ++){
                s = bits[i]*16;
                if (i % 2 == 0){
                    for (int b = 0; b < 4; b ++){
                        if (s - pow(2, 3-b) < 0) bibs[b] = 0;
                        else {
                            s -= pow(2, 3-b);
                            bibs[b] = 1;
                        }
                    }
                }
                else{
                    for (int b = 0; b < 4; b ++){
                        if (s - pow(2, 3-b) < 0) bibs[4+b] = 0;
                        else {
                            s -= pow(2, 3-b);
                            bibs[4+b] = 1;
                        }
                    }
                    for (int b = 0; b < 8; b ++){
                        if (bibs[b] == 1) s += pow(2, 7-b);
                    }
                    bytes[i/2] = (char)s;
                }
            }
            ofstream fileO("TTS_Gen.ogg", ios::binary); // , ios::binary if needed as raw binary data
            for (int i = 0; i < 16384; i ++){
                fileO << bytes[i];
            }
            fileO.close();
        }
};

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

float* getInputs(wstring letters){
    wstring check = L"-)%&§]([";
    int found = 0;
    string plc = "";
    for (int j = 0; j < letters.length(); j ++){
            if (letters[j] == L','){
                for (int b = 0; b < 42; b ++) inps[b] /= 3;
                inps[42] = siz;
                break;
            }
            check = L"-)%&§]([";
            found = check.find(letters[j]);
            if (found != std::string::npos){
                inps[j*3] = 0;
                for (int i = 0; i < 3; i ++){
                    if (found - (i+1)*4 < 0){
                        inps[j*3+1] = i;
                        found -= i*4;
                        inps [j*3+2] = found;
                        break;
                    }
                }
            }
            check = L"nm{}kgxhs z2bpl5";
            found = check.find(letters[j]);
            if (found != std::string::npos){
                inps[j*3] = 1;
                for (int i = 0; i < 3; i ++){
                    if (found - (i+1)*4 < 0){
                        inps[j*3+1] = i;
                        found -= i*4;
                        inps [j*3+2] = found;
                        break;
                    }
                }
            }
            check = L"rwv7c$y !   dtf";
            found = check.find(letters[j]);
            if (found != std::string::npos){
                inps[j*3] = 2;
                for (int i = 0; i < 3; i ++){
                    if (found - (i+1)*4 < 0){
                        inps[j*3+1] = i;
                        found -= i*4;
                        inps [j*3+2] = found;
                        break;
                    }
                }
            }
            check = L"a ei3o06 <u# ";
            found = check.find(letters[j]);
            if (found != std::string::npos){
                inps[j*3] = 3;
                for (int i = 0; i < 3; i ++){
                    if (found - (i+1)*4 < 0){
                        inps[j*3+1] = i;
                        found -= i*4;
                        inps [j*3+2] = found;
                        break;
                    }
                }
            }
            if (letters[j] == 164){ // ä
                inps[j*3] = 3;
                inps[j*3+1] = 0;
                inps[j*3+2] = 1;
            }
            if (letters[j] == 188){ // ö
                inps[j*3] = 3;
                inps[j*3+1] = 2;
                inps[j*3+2] = 0;
            }
            if (letters[j] == 182){ // ü
                inps[j*3] = 3;
                inps[j*3+1] = 3;
                inps[j*3+2] = 0;
            }
            if (letters[j] == 159){ // ß
                inps[j*3] = 1;
                inps[j*3+1] = 2;
                inps[j*3+2] = 1;
            }
        }
    return inps;
}

NeuralNetwork net;

int main(){
    for (int i = 0; i < 16; i ++){ // AI code
        bitConvs[i] = ((float)1/(float)16)*i;
    }
    getExceptions();
    cout << "Loading" << endl;
    net.loadNetByte();
    cout << "Inputs" << endl;
    siz = 0.4524;
    net.input(getInputs(L"w3li,"));
    cout << "Calculating" << endl;
    net.calculate();
    cout << "Making output" << endl;
    net.makeOutput();
}
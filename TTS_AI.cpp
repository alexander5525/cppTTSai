#include <bitset>
#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <math.h>
#include <direct.h>
#include <algorithm>
#include <experimental/filesystem>
#include <windows.h>

using namespace std;

vector<wstring> filenames;
char bytes[16384];
float bits[32768]; // Not actual bits just what will be passed into the network
float bats[32768]; // Bits generated (not actual bits)
float inps[43]; // 13 vocals made up of 3 bits with 4 states + 1 extra for the percentage of the file size
int randint; // Index for file;
char convs[4]; // Placeholder for bit conversion

vector<int> exceptions; // Bytes of an ogg file, which are not read

float bitConvs[16]; // Bytes for the sake of space will be converted into 2 and then 1 of 16 nums in range [-1;1]

size_t length;

float betterE(float num){
    if (num > 88.722839) return pow(2, 127);
    else if (num < -87.3365447) return pow(2, -125);
    else return exp(num);
}

char * toByte(float num, char type){
    int bits[32]; // bits of the data types
    int s = 0;
    if (type == 'i'){ // if an integer is handled
        num = (int)num;
        bits[0] = 0;
        if (num < 0){
            bits[0] = 1;
            num *= -1;
        }
        for (int i = 0; i < 31; i ++){
            if (num - pow(2, 30-i) < 0) bits[1+i] = 0;
            else {
                bits[1+i] = 1;
                num -= pow(2, 30-i);
            }
        }
    }
    else if (type == 'f'){ // if a float is handled
        int expon;
        vector<int> bibs;
        if (num == 0){
            for (int i = 0; i < 32; i ++) bits[i] = 0;
            s = 0;
            for (int i = 0; i < 4; i ++){ // converts the bits into bytes
                for (int b = 0; b < 8; b ++){
                    if (bits[i*8+b] == 1) s += pow(2, 7-b);
                }
                convs[i] = (char)s;
                s = 0;
            }
            return convs;
        }
        if (num == INFINITY){
            bits[0] = 0;
            for (int i = 0; i < 8; i ++) bits[1+i] = 1;
            for (int i = 0; i < 23; i ++) bits[9+i] = 0;
            for (int i = 0; i < 4; i ++){ // converts the bits into bytes
                for (int b = 0; b < 8; b ++){
                    if (bits[i*8+b] == 1) s += pow(2, 7-b);
                }
                convs[i] = (char)s;
                s = 0;
            }
            return convs;
        }
        if (num == -INFINITY){
            bits[0] = 1;
            for (int i = 0; i < 8; i ++) bits[1+i] = 1;
            for (int i = 0; i < 23; i ++) bits[9+i] = 0;
            for (int i = 0; i < 4; i ++){ // converts the bits into bytes
                for (int b = 0; b < 8; b ++){
                    if (bits[i*8+b] == 1) s += pow(2, 7-b);
                }
                convs[i] = (char)s;
                s = 0;
            }
            return convs;
        }
        bits[0] = 0;
        if (num < 0){
            bits[0] = 1;
            num *= -1;
        }
        s = 0;
        while (true){ // find out the nearest power of 2
            if (num / pow(2, s) >= 1 && num / pow(2, s) < 2) break;
            else if (num / pow(2, s) < 1) s --;
            else s ++;
        }

        expon = s + 127; // bias
        for (int i = 7; i > -1; i --){ // convert biased exponent to raw bits
            if (expon -pow(2, i) < 0) bits[8-i] = 0;
            else {
                bits[8-i] = 1;
                expon -= pow(2, i);
            }
        }
        num /= pow(2, s);
        num -= 1;
        while (true){ // converting fraction to bits
            if (num == 0) break;
            num *= 2;
            bibs.resize(bibs.size()+1);
            bibs[bibs.size()-1] = (int)num;
            num -= (int)num;
            if (bibs.size() > 23) break;
        }
        for (int i = 0; i < 23; i ++){ // saving fraction in bits
            if (bibs.size() < i+1) bits[9+i] = 0;
            else{
                if (i == 22 && bibs.size() > 23){
                    if (bibs[23] == 1) bits[31] = 1; // rounding up
                    else bits[31] = bibs[22];
                }
                else bits[9+i] = bibs[i];
            }
        }
    }
    s = 0;
    for (int i = 0; i < 4; i ++){ // converts the bits into bytes
        for (int b = 0; b < 8; b ++){
            if (bits[i*8+b] == 1) s += pow(2, 7-b);
        }
        convs[i] = (char)s;
        s = 0;
    }
    return convs;
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

        void ranVal(vector<Layer> layers){
            random_device rd; // non-deterministic generator
            mt19937 gen(rd()); // to seed mersenne twister.
            uniform_real_distribution<> dist(-1, 1); // distribute results between -5 and 5 inclusive.
            for (int n = 0; n < nCount; n ++){
                neurons[n].bias = dist(gen); // Sets bias to a random number between -5 and 5
                for (int j = 0; j < layers[layer].neurons[n].weight.size(); j ++){
                    neurons[n].weight[j] = dist(gen); // Sets weight to a random number between -5 and 5
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

        /* void calcLayNew(Layer lay){ // lay is the previous layer
            float mid = 0;
            for (int n = 0; n < nCount; n ++){
                for (int i = 0; i < neurons[n].weight.size(); i ++){
                    mid += lay.neurons[neurons[n].connections[i]].act*neurons[n].weight[i];
                }
                mid += neurons[n].bias;
                neurons[n].calc(mid);
                mid = 0;
            }
        } */

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

        /* void newInit(int * laySiz, int lCou){
        lCount = lCou;
        layers.resize(lCou);
        for (int j = 0; j < lCou; j ++){
            layers[j].layer = j;
            layers[j].nCount = laySiz[j];
            layers[j].neurons.resize(laySiz[j]);
            for (int i = 0; i < laySiz[j]; i ++){
                layers[j].neurons[i].layer = j;
                if (j != 0){
                    layers[j].neurons[i].pCount = laySiz[j-1];
                    if (j < (lCou-1)/2+1){
                        layers[j].neurons[i].weight.resize(laySiz[j-1]/laySiz[j]); // Size of weight array is equal to size of neuron array in previous layer divided by size of current layer
                        layers[j].neurons[i].desWei.resize(laySiz[j-1]/laySiz[j]);
                        for (int we = 0; we < laySiz[j-1]/laySiz[j]; we ++){
                            layers[j].neurons[i].weight[we] = 0;
                            layers[j].neurons[i].desWei[we] = 0;
                        }
                        layers[j].neurons[i].connections.resize(laySiz[j-1]/laySiz[j]);
                        for (int c = 0; c < laySiz[j-1]/laySiz[j]; c ++){
                            layers[j].neurons[i].connections[c] = i*(laySiz[j-1]/laySiz[j]) + c;
                        }
                    }
                    else{
                        layers[j].neurons[i].weight.resize(1); // Size of weight array is equal to 1 because 1 neuron gives information to many
                        layers[j].neurons[i].desWei.resize(1);
                        layers[j].neurons[i].weight[0] = 0;
                        layers[j].neurons[i].desWei[0] = 0;
                        layers[j].neurons[i].connections.resize(1);
                        layers[j].neurons[i].connections[0] = floor(i/(laySiz[j]/laySiz[j-1]));
                    }
                    layers[j].neurons[i].bias = 0;
                    layers[j].neurons[i].desBias = 0;
                    layers[j].neurons[i].desAct = 0;
                }
            }
        }
    } */

        void ran(){ // Will generate random weights and biases for the neurons from the second layer onward
            for (int i = 1; i < lCount; i++){
                layers[i].ranVal(layers);
            }
        }

        void calculate(){ // Will calculate the activation of the neurons from the second layer onward
            for (int i = 1; i < lCount; i++){
                layers[i].calcLay(layers[i-1]);
            }
        }

        /* void calculateNew(){ // Will calculate the activation of the neurons from the second layer onward
            for (int i = 1; i < lCount; i++){
                layers[i].calcLayNew(layers[i-1]);
            }
        } */

        void display(){ // Displays the information about the network in the terminal
            for (int i = 0; i < lCount; i++){
                cout << i+1 << ". Layer";
                if (i == 0) cout << " (Input)";
                else if (i == lCount-1) cout << " (Output)";
                else cout << " (Hidden)";
                cout << endl;
                for (int j = 0; j < layers[i].nCount; j ++){
                    cout << endl << j+1 << ". Neuron\n";
                    cout << "Activation: " << layers[i].neurons[j].act << endl;
                    cout << "Neuron Layer: " << layers[i].neurons[j].layer+1 << endl;
                    if (i != 0){
                        cout << "Bias: " << layers[i].neurons[j].bias << endl;
                        for (int k = 0; k < layers[i].neurons[j].weight.size(); k ++){
                            //cout << "Connection " << k+1 << ": " << layers[i].neurons[j].connections[k] << endl;
                            cout << "Weight " << k+1 << ": " << layers[i].neurons[j].weight[k] << endl;
                        }
                    }
                }
                cout << endl;
            }
            cout << "Total Cost: " << totCost << endl << endl << endl << endl;
        }

        void input(float inp[], int scalar = 1){ // Will set the activation of the neurons in the input layer to the array given
            // The scalar is useful for converting a number line from 0 to n into 0 to 1 if n is given as the scalar
            for (int i = 0; i < layers[0].nCount; i++){
                layers[0].neurons[i].act = inp[i] / scalar;
            }
        }

        void adjustBatch(int size){ // Sets the batch size with relative ease
            batchSize = size;
        }

        void backprop(float desire[]){ // Does the backpropagation
            float cost = 0;
            for (int i = 0; i < layers[lCount-1].nCount; i ++){
                cost += pow((layers[lCount-1].neurons[i].act - desire[i]), 2); // Calculate Cost | C0 = (a-y)^2
            }
            totCost += cost;
            // NOTE: Indices are not written down in the comments, due to it not supporting super and subscript
            float x = 0;
            for (int l = lCount-1; l > 0; l --){
                if (l == lCount-1){ // In the last layer
                    for (int n = 0; n < layers[l].nCount; n ++){
                        x = layers[l].neurons[n].bias;
                        for (int i = 0; i < layers[l-1].nCount; i++){
                            x += layers[l].neurons[n].weight[i]*layers[l-1].neurons[i].act; // x = w*a+w*a...+b
                        }// Backpropagate bias
                        if (isnan(x)) cerr << "Error: X is nan " << endl << "Activation was " << layers[l].neurons[n].act << " in Layer " << l << " Neuron " << n << endl;
                        layers[l].neurons[n].desBias += pow(1+betterE(-x), -2)*betterE(-x)*2*(layers[l].neurons[n].act-desire[n]); // C0'(b) = (1+e^-x)^-2*e^-x*2(a-y)
                        if (isnan(layers[l].neurons[n].desBias)) cerr << "Error: DesBias is nan"<< endl << "X: " << x << " Y: " << desire[n] << endl;
                        for (int w = 0; w < layers[l-1].nCount; w ++){ // Backpropagate weights and activations
                            layers[l-1].neurons[w].desAct += layers[l].neurons[n].weight[w]* // w*
                            pow(1+betterE(-x), -2)*betterE(-x)* // (1+e^-x)^-2*e^-x*
                            2*(layers[l].neurons[n].act - desire[n]); // 2(a-y)
                            if (isnan(layers[l-1].neurons[w].desAct)) {
                                cerr << "Error: DesAct is nan" << endl << "X: " << x << " Y: " << desire[n] << endl;
                                cout << layers[l].neurons[n].weight[w] << " " << pow(1+betterE(-x), -2) << " " << betterE(-x) << " " << (float)2*(layers[l].neurons[n].act - desire[n]) << endl;
                            }
                            // Whole function: C0'(a) = SUM(w*(1+e^-x)^-2*e^-x*2(a-y))
                            layers[l].neurons[n].desWei[w] += layers[l-1].neurons[w].act* // C0'(w) = a*
                            pow(1+betterE(-x), -2)*betterE(-x)* // (1+e^-x)^-2*e^-x*
                            2*(layers[l].neurons[n].act - desire[n]); // 2(a-y)
                            //if (isnan(layers[l].neurons[n].desWei[w])) cerr << "Error: DesWei is nan"<< endl << "X: " << x << " Y: " << desire[n] << endl;
                        } // Whole function: C0'(w) = a*-(1+e^-x)^-2*-(e^-x)*2(a-y)
                    }
                }
                else{ // In any other layer (except the first)
                    for (int n = 0; n < layers[l].nCount; n ++){
                        x = layers[l].neurons[n].bias;
                        for (int i = 0; i < layers[l-1].nCount; i++){
                            x += layers[l].neurons[n].weight[i]*layers[l-1].neurons[i].act; // x = w*a+w*a...+b
                        }
                        if (isnan(x)) cerr << "Error: X is nan " << endl << "Activation was " << layers[l].neurons[n].act << " in Layer " << l << " Neuron " << n << endl;
                        layers[l].neurons[n].desBias += pow(1+betterE(-x), -2)*betterE(-x)*layers[l].neurons[n].desAct; // C0'(b) = (1+e^-x)^-2*e^-x*C0'(a)
                        for (int w = 0; w < layers[l-1].nCount; w ++){
                            layers[l].neurons[n].desWei[w] += pow(1+betterE(-x), -2)*betterE(-x)*layers[l-1].neurons[w].act*layers[l].neurons[n].desAct; // C0'(w) = (1+e^-x)^-2*e^-x*a*C0'(a)
                            if (l != 1){
                                layers[l-1].neurons[w].desAct += pow(1+betterE(-x), -2)*betterE(-x)* // C0'(a) = (1+e^-x)^-2*e^-x*w
                                layers[l].neurons[n].weight[w]*layers[l].neurons[n].desAct; // *C0'(a)
                            }
                        }
                    }
                }
            }
            for (int l = 1; l < lCount; l ++){
                for (int n = 0; n < layers[l].nCount; n ++){
                    layers[l].neurons[n].desAct = 0; // Resets desired activation for future runs
                }
            }
        }

        /* void backpropNew(float desire[]){ // Does the backpropagation
            float cost = 0;
            for (int i = 0; i < layers[lCount-1].nCount; i ++){
                cost += pow((layers[lCount-1].neurons[i].act - desire[i]), 2); // Calculate Cost | C0 = (a-y)^2
            }
            totCost += cost;
            // NOTE: Indices are not written down in the comments, due to it not supporting super and subscript
            float x = 0;
            for (int l = lCount-1; l > 0; l --){
                if (l == lCount-1){ // In the last layer
                    for (int n = 0; n < layers[l].nCount; n ++){
                        x = layers[l].neurons[n].bias;
                        for (int i = 0; i < layers[l].neurons[n].weight.size(); i++){
                            x += layers[l].neurons[n].weight[i]*layers[l-1].neurons[layers[l].neurons[n].connections[i]].act; // x = w*a+w*a...+b
                        }// Backpropagate bias
                        if (isnan(x)) cerr << "Error: X is nan " << endl << "Activation was " << layers[l].neurons[n].act << " in Layer " << l << " Neuron " << n << endl;
                        layers[l].neurons[n].desBias += pow(1+betterE(-x), -2)*betterE(-x)*2*(layers[l].neurons[n].act-desire[n]); // C0'(b) = (1+e^-x)^-2*e^-x*2(a-y)
                        if (isnan(layers[l].neurons[n].desBias)) cerr << "Error: DesBias is nan"<< endl << "X: " << x << " Y: " << desire[n] << endl;
                        for (int w = 0; w < layers[l].neurons[n].weight.size(); w ++){ // Backpropagate weights and activations
                            layers[l-1].neurons[layers[l].neurons[n].connections[w]].desAct += layers[l].neurons[n].weight[w]* // w*
                            pow(1+betterE(-x), -2)*betterE(-x)* // (1+e^-x)^-2*e^-x*
                            2*(layers[l].neurons[n].act - desire[n]); // 2(a-y)
                            // Whole function: C0'(a) = SUM(w*(1+e^-x)^-2*e^-x*2(a-y))
                            layers[l].neurons[n].desWei[w] += layers[l-1].neurons[layers[l].neurons[n].connections[w]].act* // C0'(w) = a*
                            pow(1+betterE(-x), -2)*betterE(-x)* // (1+e^-x)^-2*e^-x*
                            2*(layers[l].neurons[n].act - desire[n]); // 2(a-y)
                            //if (isnan(layers[l].neurons[n].desWei[w])) cerr << "Error: DesWei is nan"<< endl << "X: " << x << " Y: " << desire[n] << endl;
                        } // Whole function: C0'(w) = a*-(1+e^-x)^-2*-(e^-x)*2(a-y)
                    }
                }
                else{ // In any other layer (except the first)
                    for (int n = 0; n < layers[l].nCount; n ++){
                        x = layers[l].neurons[n].bias;
                        for (int i = 0; i < layers[l].neurons[n].weight.size(); i++){
                            x += layers[l].neurons[n].weight[i]*layers[l-1].neurons[layers[l].neurons[n].connections[i]].act; // x = w*a+w*a...+b
                        }
                        if (isnan(x)) cerr << "Error: X is nan " << endl << "Activation was " << layers[l].neurons[n].act << " in Layer " << l << " Neuron " << n << endl;
                        layers[l].neurons[n].desBias += pow(1+betterE(-x), -2)*betterE(-x)*layers[l].neurons[n].desAct; // C0'(b) = (1+e^-x)^-2*e^-x*C0'(a)
                        for (int w = 0; w < layers[l].neurons[n].weight.size(); w ++){
                            layers[l].neurons[n].desWei[w] += pow(1+betterE(-x), -2)*betterE(-x)*layers[l-1].neurons[layers[l].neurons[n].connections[w]].act*
                            layers[l].neurons[n].desAct; // C0'(w) = (1+e^-x)^-2*e^-x*a*C0'(a)
                            if (l != 1){
                                layers[l-1].neurons[layers[l].neurons[n].connections[w]].desAct += pow(1+betterE(-x), -2)*betterE(-x)* // C0'(a) = (1+e^-x)^-2*e^-x*w
                                layers[l].neurons[n].weight[w]*layers[l].neurons[n].desAct; // *C0'(a)
                            }
                        }
                    }
                }
            }
            for (int l = 1; l < lCount; l ++){
                for (int n = 0; n < layers[l].nCount; n ++){
                    layers[l].neurons[n].desAct = 0; // Resets desired activation for future runs
                }
            }
        } */

        void gradDes(){ // Gradient descend
            for (int l = 1; l < lCount; l ++){
                for (int n = 0; n < layers[l].nCount; n ++){
                    layers[l].neurons[n].bias -= layers[l].neurons[n].desBias/batchSize; // Divide by batch size for average
                    layers[l].neurons[n].desBias = 0; // Reset the values
                    for (int i = 0; i < layers[l-1].nCount; i ++){
                        layers[l].neurons[n].weight[i] -= layers[l].neurons[n].desWei[i]/batchSize;
                        layers[l].neurons[n].desWei[i] = 0;
                    }
                }
            }
            totCost /= batchSize; // Calculates total cost from the batch
        }

        /* void gradDesNew(){ // Gradient descend
            for (int l = 1; l < lCount; l ++){
                for (int n = 0; n < layers[l].nCount; n ++){
                    layers[l].neurons[n].bias -= layers[l].neurons[n].desBias/batchSize; // Divide by batch size for average
                    layers[l].neurons[n].desBias = 0; // Reset the values
                    for (int i = 0; i < layers[l].neurons[n].weight.size(); i ++){
                        layers[l].neurons[n].weight[i] -= layers[l].neurons[n].desWei[i]/batchSize;
                        layers[l].neurons[n].desWei[i] = 0;
                    }
                }
            }
            totCost /= batchSize; // Calculates total cost from the batch
        } */

        void saveNet(){ // Saves crucial information about a Network (Size, Weights and biases)
            ofstream file("TTS_AI_V1.txt");
            file << to_string(lCount) + "\n";
            for (int i = 0; i < lCount; i ++){
                file << to_string(layers[i].nCount) + ",";
            }
            file << "\n";
            for (int i = 1; i < lCount; i ++){
                for (int j = 0; j < layers[i].nCount; j ++){
                    //file << to_string(layers[i].neurons[j].act) + ",";
                    for (int k = 0; k < layers[i-1].nCount; k ++){
                        file << to_string(layers[i].neurons[j].weight[k]) + ",";
                    }
                    file << to_string(layers[i].neurons[j].bias) + ",";
                }
                file << "\n";
            }
            file.close();
        }

        void saveNetByte(){ // Saves crucial information about a Network (Size, Weights and biases) in raw byte data
            ofstream file("TTS_AI_V1.txt", ios::binary);
            int total = 1 + lCount;
            int current = 0;
            for (int i = 1; i < lCount; i ++){
                total += layers[i].nCount;
            }
            toByte(lCount, 'i');
            for (int c = 0; c < 4; c ++) file << convs[c];
            current ++;
            for (int i = 0; i < lCount; i ++){
                toByte(layers[i].nCount, 'i');
                for (int c = 0; c < 4; c ++) file << convs[c];
                current ++;
            }
            for (int i = 1; i < lCount; i ++){
                for (int n = 0; n < layers[i].nCount; n ++){
                    if (current % 100 == 0){
                        system("cls");
                        cout << "Saving in Progress..." << endl;
                        cout << (int)100*((float)current/(float)total) << "% complete ";
                        for (int i = 0; i < 10; i ++){
                            if (i < (int)(10*((float)current/(float)total))) cout << "[#]";
                            else cout << "[]";
                        }
                        cout << endl;
                        //cout << "Neuron " << n << "..." << endl;
                    }
                    for (int w = 0; w < layers[i].neurons[n].weight.size(); w ++){
                        toByte(layers[i].neurons[n].weight[w], 'f');
                        for (int c = 0; c < 4; c ++) file << convs[c];
                    }
                    toByte(layers[i].neurons[n].bias, 'f');
                    for (int c = 0; c < 4; c ++) file << convs[c];
                    current ++;
                }
                //cout << "Layer " << i << "..." << endl << endl;
            }
            file.close();
            system("cls");
        }

        /* void saveNetNew(){ // Saves crucial information about a Network (Size, Weights, Biases and Connections)
            ofstream file("TTS_AI_V3.txt");
            file << to_string(lCount) + "\n";
            for (int i = 0; i < lCount; i ++){
                file << to_string(layers[i].nCount) + ",";
            }
            file << "\n";
            for (int l = 1; l < lCount; l ++){
                for (int n = 0; n < layers[l].nCount; n ++){
                    file << layers[l].neurons[n].connections.size() << ",";
                }
            }
            for (int i = 1; i < lCount; i ++){
                for (int j = 0; j < layers[i].nCount; j ++){
                    //file << to_string(layers[i].neurons[j].act) + ",";
                    file << endl;
                    for (int c = 0; c < layers[i].neurons[j].connections.size(); c ++){
                        file << to_string(layers[i].neurons[j].connections[c]) << ",";
                    }
                    file << endl;
                    for (int k = 0; k < layers[i].neurons[j].weight.size(); k ++){
                        file << to_string(layers[i].neurons[j].weight[k]) + ",";
                    }
                    file << to_string(layers[i].neurons[j].bias) + ",";
                }
            }
            file << endl;
            file.close();
        } */

        void loadNet(string name){ //Loads the Network from a name specified
            ifstream file(name);
            if (file.is_open()){
                int num = count(istreambuf_iterator<char>(file),  istreambuf_iterator<char>(), '\n'); // Counts the number of lines in the file
                string lines[num];
                file.close(); // Lines have already been called and now need to be reloaded
                ifstream file2(name);
                for (int i = 0; i < num; i ++){ // Loads lines into memory
                    file2 >> lines[i];
                }
                int size; // Amount of layers in network
                vector<int> sizes; // Size of said layers
                string plc = "";
                int pl = 0;
                int neg = 1; // Multiplier for positive/ negative numbers
                int pls[] = {1, 0, 0, 0};
                for (int line = 0; line < num; line ++){ // Saves the lines in the network
                    if (line == 0){
                        for (int j = 0; j < lines[line].length(); j ++){ // Reads the size from the file
                            plc += lines[line][j];
                        }
                        size = stoi(plc); // Stores the size into the size variable
                        plc = "";
                        sizes.resize(size);
                    }
                    else if (line == 1){
                        for (int i = 0; i < lines[line].length(); i ++){
                            if (lines[line][i] == ','){
                                sizes[pl] = stoi(plc); // Stores the read number in the vector
                                plc = "";
                                pl ++;
                            }
                            else plc += lines[line][i];
                        }
                        int temp[size]; // What will be passed into the network
                        for (int i = 0; i < size; i ++){
                            temp[i] = sizes[i]; // Converts the vector into an array
                        }
                        initAll(size, temp); // Passes size and array on
                        pl = 0; plc = "";
                    }
                    else{
                        // pl will be used for keeping track of the weight and bias
                        // pls[0] will be used to keep track of the layer currently on
                        // pls[1] will be used to keep track of the neuron
                        // pls[2] will keep track on the power of 10
                        // pls[3] keeps track of whether powers of 10 will be applied
                        for (int i = 0; i < lines[line].length(); i ++){
                            if (lines[line][i] == ','){ // When the number has been fully read
                                if (pl < sizes[pls[0]-1]) {
                                    layers[pls[0]].neurons[pls[1]].weight[pl] = neg*stoi(plc)*pow(10, pls[2]); // Add weight to the neurons
                                    pl ++;
                                }
                                else {
                                    layers[pls[0]].neurons[pls[1]].bias = neg*stoi(plc)*pow(10, pls[2]); // Add bias to the neurons
                                    pl = 0;
                                    pls[1] ++;
                                }
                                plc = "";
                                neg = 1;
                                pls[3] = 0;
                                pls[2] = 0;
                            }
                            else if (lines[line][i] == '-'){
                                neg = -1; // If the number is negative
                            }
                            else if (lines[line][i] == '.'){
                                pls[3] = 1; // Read powers of 10 true
                                pls[2] = 0; // Start power: 0
                            }
                            else {
                                plc += lines[line][i]; // Add the char to plc
                                if (pls[3] == 1) pls[2] --; // Substract power of 10 when read is true
                            }
                        }
                        pls[0] ++;
                        pls[1] = 0;
                    }
                }
                file2.close();
            }
            else cout << "File doesn't exist.";
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
                            if (current % 100 == 0){
                                system("cls");
                                cout << "Loading..." << endl;
                                cout << (int)100*((float)current/(float)total) << "% complete ";
                                for (int i = 0; i < 10; i ++){
                                    if (i < (int)(10*((float)current/(float)total))) cout << "[#]";
                                    else cout << "[]";
                                }
                                cout << endl;
                                //cout << "Neuron " << n << "..." << endl;
                            }
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
            system("cls");
        }

        /* void loadNetNew(string name){ //Loads the Network from a name specified
            ifstream file(name);
            if (file.is_open()){
                int num;
                num = count(istreambuf_iterator<char>(file),  istreambuf_iterator<char>(), '\n'); // Counts the number of lines in the file
                vector<string> lines;
                lines.resize(num);
                file.close(); // Lines have already been called and now need to be reloaded
                ifstream file2(name);
                for (int i = 0; i < num; i ++){ // Loads lines into memory
                    file2 >> lines[i];
                }
                file2.close();
                int size; // Amount of layers in network
                vector<int> sizes; // Size of said layers
                string plc = "";
                int pl = 0;
                int neg = 1; // Multiplier for positive/ negative numbers
                int pls[] = {1, 0, 0, 0, 0};
                for (int line = 0; line < num; line ++){ // Saves the lines in the network
                    if (line == 0){
                        for (int j = 0; j < lines[line].length(); j ++){ // Reads the size from the file
                            plc += lines[line][j];
                        }
                        size = stoi(plc); // Stores the size into the size variable
                        plc = "";
                        sizes.resize(size);
                    }
                    else if (line == 1){
                        for (int i = 0; i < lines[line].length(); i ++){
                            if (lines[line][i] == ','){
                                sizes[pl] = stoi(plc); // Stores the read number in the vector
                                plc = "";
                                pl ++;
                            }
                            else plc += lines[line][i];
                        }
                        int temp[size]; // What will be passed into the network
                        for (int i = 0; i < size; i ++){
                            temp[i] = sizes[i]; // Convets the vector into an array
                        }
                        newInit(temp, size); // Passes array on
                        pl = 0; plc = "";
                    }
                    else if (line == 2){
                        for (int i = 0; i < lines[line].length(); i ++){
                            if (lines[line][i] == ','){
                                layers[pls[0]].neurons[pls[1]].connections.resize(stoi(plc)); // Resize connections vector
                                plc = "";
                                pls[1] ++;
                                if (layers[pls[0]].nCount == pls[1]){ // If the neuron index is out of range
                                    pls[1] = 0;
                                    pls[0] ++;
                                }
                            }
                            else plc += lines[line][i];
                        }
                        pls[1] = 0;
                        pls[0] = 1;
                    }
                    else{
                        // pl will be used for keeping track of the weight, bias and connection
                        // pls[0] will be used to keep track of the layer currently on
                        // pls[1] will be used to keep track of the neuron
                        // pls[2] will keep track on the power of 10
                        // pls[3] keeps track of whether powers of 10 will be applied
                        // pls[4] will keep track on whether connections or wei & bias are being stored
                        if (pls[4] == 0){ // Connections
                            for (int i = 0; i < lines[line].length(); i ++){
                                if (lines[line][i] == ','){
                                    layers[pls[0]].neurons[pls[1]].connections[pl] = stoi(plc);
                                    pl ++;
                                    plc = "";
                                }
                                else plc += lines[line][i];
                            }
                            pls[4] = 1;
                            pl = 0;
                        }
                        else if (pls[4] == 1){ // Weights and Biases
                            for (int i = 0; i < lines[line].length(); i ++){
                                if (lines[line][i] == ','){ // When the number has been fully read
                                    if (pl < layers[pls[0]].neurons[pls[1]].weight.size()) {
                                        layers[pls[0]].neurons[pls[1]].weight[pl] = neg*stoi(plc)*pow(10, pls[2]); // Add weight to the neurons
                                        pl ++;
                                    }
                                    else {
                                        layers[pls[0]].neurons[pls[1]].bias = neg*stoi(plc)*pow(10, pls[2]); // Add bias to the neurons
                                        pl = 0;
                                        pls[1] ++;
                                    }
                                    plc = "";
                                    neg = 1;
                                    pls[3] = 0;
                                    pls[2] = 0;
                                }
                                else if (lines[line][i] == '-'){
                                    neg = -1; // If the number is negative
                                }
                                else if (lines[line][i] == '.'){
                                    pls[3] = 1; // Read powers of 10 true
                                    pls[2] = 0; // Start power: 0
                                }
                                else {
                                    plc += lines[line][i]; // Add the char to plc
                                    if (pls[3] == 1) pls[2] --; // Substract power of 10 when read is true
                                }
                            }
                            if (layers[pls[0]].nCount == pls[1]){
                                pls[0] ++;
                                pls[1] = 0;
                            }
                            pls[4] = 0;
                        }
                    }
                }
            }
            else cout << "File doesn't exist.";
        } */
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

void getFilenames(){
    filenames.resize(0);
    int s = 0;
    wstring plc = L"";
    string dir = "TTS_smol_big/";
    for (auto& entry : std::experimental::filesystem::directory_iterator(dir)){ // For every file in directory
        filenames.resize(filenames.size()+1); // Resize to 1 above before size
        filenames[s] = entry.path().wstring().c_str(); // Store path in variable
        for (int i = dir.length()-1; i < filenames[s].length()-4; i ++){ // Remove the .txt ending and the TTS_AI_text/
            plc += filenames[s][i];
        }
        filenames[s] = plc;
        plc = L"";
        s ++;
    }
}

float * getBits(){
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, filenames.size()-1);
    randint = dist(gen);
    wstring filename = L"TTS_AI/";
    filename += filenames[randint];
    filename += L".ogg";
    ifstream file(filename.c_str()); // Open a file
    if (file.is_open() != true) cerr << filename.c_str() << " does not exist.";
    file.seekg(0, std::ios::end); // Go to the end of the file
    length = file.tellg(); // Store the length of the file
    file.seekg(0, std::ios::beg); // Go to beginning of file
    file.read(bytes, length); // Read bytes of file into byte array
    file.close();
    int s = 0;
    int b = 0;
    bool yet = false; // When the file is finished set size scalar and never thereafter
    bool no = false; // whether the hex bit is an exception or not
    //ofstream fileO("wo.txt");
    for (int i = 0; i < 16384; i ++){ // Bytes
        for (int j = 0; j < 2; j ++){
            for (int k = 0; k < 4; k ++){
                if (bitset<8>(bytes[i])[7-j*4-k] == 1){
                    s += pow(2, 3-k);
                }
                if (k == 3){
                    for (int e = 0; e < exceptions.size(); e ++) if (i*2+j == exceptions[e]){
                        s = 0;
                        no = true;
                        break;
                    }
                    bits[b] = bitConvs[s];
                    //fileO << s << endl;
                    s = 0;
                    if (i > length) {
                        if (yet != true) {
                            inps[42] = (float)b / (float)23979;
                            yet = true;
                        }
                        bits[b] = bitConvs[s]; // If the file has already ended, fill the rest with zeroes
                    }
                    if(!no) b ++;
                    else no = false;
                }
            }
        }
    }
    //fileO.close();
    return bits;
}

float* getInputs(){
    wstring filename = L"TTS_AI_text/";
    filename += filenames[randint];
    filename += L".txt";
    wifstream fileIn(filename.c_str());
    wstring letters[1];
    fileIn >> letters[0];
    wstring check = L"-)%&§]([";
    int found = 0;
    for (int j = 0; j < letters[0].length(); j ++){
            if (letters[0][j] == L','){
                for (int b = 0; b < 42; b ++) inps[b] /= 3;
                break;
            }
            check = L"-)%&§]([";
            found = check.find(letters[0][j]);
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
            found = check.find(letters[0][j]);
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
            found = check.find(letters[0][j]);
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
            found = check.find(letters[0][j]);
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
            if (letters[0][j] == 164){ // ä
                inps[j*3] = 3;
                inps[j*3+1] = 0;
                inps[j*3+2] = 1;
            }
            if (letters[0][j] == 188){ // ö
                inps[j*3] = 3;
                inps[j*3+1] = 2;
                inps[j*3+2] = 0;
            }
            if (letters[0][j] == 182){ // ü
                inps[j*3] = 3;
                inps[j*3+1] = 3;
                inps[j*3+2] = 0;
            }
            if (letters[0][j] == 159){ // ß
                inps[j*3] = 1;
                inps[j*3+1] = 2;
                inps[j*3+2] = 1;
            }
        }
    return inps;
}

/* void bitsToFile(){ // WIP
    int s = 0;
    ofstream fileO("w.txt", ios::binary);
    for (int i = 0; i < 16384; i ++){
        for (int j = 0; j < 8; j ++){
            if (j == 0 && bits[8*i+j] == 1) s = -128;
            else {
                if (bits[8*i+j] == 1) s += (int)pow(2, 7-j);
            }
        }
        fileO << (char)s;
        s = 0;
    }
    fileO.close();
} */

void bitsToFile(NeuralNetwork nt){
    int beets[131072]; // Actual bits
    //for (int b = 0; b < 131072; b ++) beets[b] = 0;
    int plc;
    for (int i = 0; i < nt.layers[nt.lCount-1].nCount; i ++){
        for (int j = 0; j < 16; j ++){
            if (nt.layers[nt.lCount-1].neurons[i].act < (float)(1+2*j)/(float)32) {
                plc = j;
                break;
            }
        }
        bats[i] = plc/(float)16;
        /* if (plc - 8 >= 0) {
            beets[i*4] = 1;
            plc -= 8;
        }
        if (plc - 4 >= 0){
            beets[i*4+1] = 1;
            plc -= 4;
        }
        if (plc - 2 >= 0){
            beets[i*4+1] = 1;
            plc -= 2;
        }
        if (plc - 1 >= 0){
            beets[i*4+1] = 1;
        } */
    }
}

int laySize[] = {43, (int)pow(2, 7), (int)pow(2, 10), (int)pow(2, 12), 23979};

/* float inps[8];

float * genInps(){
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 1);
    for (int i = 0; i < 8; i ++){
        inps[i] = dist(gen);
    }
    return inps;
} */

HMENU hMenu;

HWND name;

bool loop = false;

void AddMenus(HWND wnd){
    hMenu = CreateMenu();

    HMENU hFile = CreateMenu();
    HMENU hEdit = CreateMenu();
    HMENU hHelp = CreateMenu();

    AppendMenuW(hFile, MF_STRING, 6, L"Initialize");
    AppendMenuW(hFile, MF_STRING, 7, L"Load from file");
    AppendMenuW(hFile, MF_SEPARATOR, 0LL, NULL);
    AppendMenuW(hFile, MF_STRING, 1, L"Close");

    AppendMenuW(hEdit, MF_STRING, 5, L"Error Count (slow)");

    AppendMenuW(hHelp, MF_STRING, 4, L"FAQ");

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFile, L"File");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEdit, L"Edit");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hHelp, L"Help");

    SetMenu(wnd, hMenu);

    CreateWindowW(L"Button", L"Start", WS_VISIBLE | WS_CHILD, 100, 40, 100, 22, wnd, (HMENU)2, NULL, NULL);
    CreateWindowW(L"Button", L"Save", WS_VISIBLE | WS_CHILD, 100, 65, 100, 22, wnd, (HMENU)3, NULL, NULL);
}

NeuralNetwork net;
bool error = false;
bool unsaved = false;
bool initialized = false;

int batch = 25;

int errCount = 0;

LRESULT CALLBACK windowProc(HWND wnd, UINT msg, WPARAM wpara, LPARAM lpara){
    LRESULT res = 0;
    switch(msg){
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CREATE:
            AddMenus(wnd);
            break;
        case WM_CLOSE:
            if (unsaved) {
                if (MessageBoxW(wnd, L"Are you sure you want to quit?\nAny unsaved changes will be lost.", L"Quit?", MB_ICONEXCLAMATION | MB_YESNO) == IDYES){
                    DestroyWindow(wnd);
                }
            }
            else DestroyWindow(wnd);
            break;
        case WM_COMMAND:
            switch(wpara){
                case 1:
                    if (unsaved) {
                        if (MessageBoxW(wnd, L"Are you sure you want to quit?\nAny unsaved changes will be lost.", L"Quit?", MB_ICONEXCLAMATION | MB_YESNO) == IDYES){
                            DestroyWindow(wnd);
                        }
                    }
                    else DestroyWindow(wnd);
                    break;
                case 2:
                    if (initialized){
                        loop = true;
                        unsaved = true;
                    }
                    else MessageBoxW(wnd, L"The neural network has not yet been initialized.", L"Error", MB_ICONERROR | MB_OK);
                    while (loop){
                        for (int b = 0; b < batch; b ++){ // Runs in batch
                            cout << "Run " << b << endl; //<< " Input ";
                            getBits(); // IMPORTANT: Is using a smaller directory to teach the AI about the size factor
                            net.input(getInputs()); // Take the bits as input and output
                            //cout << "Calculate ";
                            net.calculate();
                            /* if (error){
                                bitsToFile(net);
                                for (int i = 0; i < 32768; i ++){
                                    if (bits[i] != bats[i]) errCount ++;
                                }
                            } */
                            //cout << "Backprop" << endl;
                            net.backprop(bits);
                            }
                        cout << "Batch finished" << endl; // << "Gradient descend" << endl;
                        net.gradDes();
                        cout << "Cost: " << net.totCost << endl;
                        /* if (error){
                            cout << "Error Count: " << errCount/batch << endl;
                            errCount = 0;
                        } */
                        cout << endl;
                        net.totCost = 0;
                        if(name == GetForegroundWindow()){
                            loop = false;
                            cout << "Stopped" << endl;
                        }
                    }
                    break;
                case 3:
                    if (initialized){
                        net.saveNetByte();
                        cout << "Finished Saving!";
                        unsaved = false;
                    }
                    else MessageBoxW(wnd, L"The neural network has not yet been initialized.", L"Error", MB_ICONERROR | MB_OK);
                    break;
                case 4:
                    MessageBoxW(wnd, L"How to Initialize the network?\n-Either load the network from a file or\n-Initialize the network under the file dropdown menu.",
                    L"Help", MB_ICONINFORMATION | MB_OK);
                    MessageBoxW(wnd, L"How to Stop the training?\n-Click into VSCode/CMD after the batch has finished.", L"Help", MB_ICONINFORMATION | MB_OK);
                    MessageBoxW(wnd, L"How Save the network?\n-Press the save button after the training has finished.", L"Help", MB_ICONINFORMATION | MB_OK);
                    break;
                case 5:
                {
                    HMENU submen = GetSubMenu(hMenu, 1);  // Currently does nothing
                    if (error){
                        CheckMenuItem(submen, 0, MF_BYPOSITION | MF_UNCHECKED);
                        error = false;
                    }
                    else{
                        CheckMenuItem(submen, 0, MF_BYPOSITION | MF_CHECKED);
                        error = true;
                    }
                    break;
                }
                case 6:
                    if (!initialized){
                        cout << "Initializing" << endl;
                        net.initAll(5, laySize);
                        cout << "Randomizing" << endl;
                        net.ran();
                        cout << "Finished initializing" << endl;
                        initialized = true;
                    }
                    break;
                case 7:
                    if (!initialized){
                        net.loadNetByte();
                        cout << "Finished loading" << endl;
                        initialized = true;
                    }
                    break;
            }
        default:
            res = DefWindowProcW(wnd, msg, wpara, lpara);
            break;
    }
    return res;
}


//int test[] = {8, 4, 2, 4, 8};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreviousInstance, LPSTR args, int nCmd){
    name = GetForegroundWindow();
    WNDCLASSW wclss = {0};
    wclss.hCursor = LoadCursor(NULL, IDC_ARROW);
    wclss.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wclss.lpszClassName = L"Windows Clas";
    wclss.lpfnWndProc = windowProc;
    wclss.hInstance = hInstance;
    if (!RegisterClassW(&wclss)) return -1;
    CreateWindowW(L"Windows Clas", L"TTS AI", WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, NULL, NULL, NULL, NULL);

    for (int i = 0; i < 16; i ++){ // AI code
        bitConvs[i] = ((float)1/(float)16)*i;
    }
    getExceptions();
    cout << "Getting File Names" << endl;
    getFilenames();
    /* cout << "Initializing" << endl;
    net.initAll(5, laySize);
    cout << "Randomizing" << endl;
    net.ran(); */
    //net.loadNetNew("TTS_AI_V3.txt");
    net.adjustBatch(batch);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0LL, 0LL)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    /* for (int i = 0; i < 16; i ++){
        bitConvs[i] = ((float)1/(float)16)*i;
    }
    cout << "Getting File Names" << endl;
    getFilenames(); // IMPORTANT: Is now using the small directory of 100 entries
    cout << "Initializing" << endl;
    net.loadNetNew("TTS_AI_V2.txt"); */
    //net.newInit(laySize);
    //cout << "Randomizing" << endl;
    //net.ran();
    /* net.input(getBits());
    net.calculateNew();
    bitsToFile(net);
    ofstream file("TTS_Test.txt");
    for (int i = 0; i < 32768; i ++){
        file << to_string(bits[i]) << "\t\t\t\t";
        //file << to_string(net.layers[net.lCount-1].neurons[i].act) << endl;
        if (bits[i] != bats[i]) file << "\t WARNING \t";
        file << to_string(bats[i]) << endl;
    }
    file.close(); */
    /* net.adjustBatch(batch);
    ofstream costFile("TTS_AI_cost.txt");
    for (int i = 0; i < 50; i ++){ // Batches
        for (int b = 0; b < batch; b ++){ // Runs in batch
            //cout << "Run " << b << endl; //<< " Input ";
            net.input(getBits()); // Take the bits as input and output
            //cout << "Calculate ";
            net.calculateNew();
            //cout << "Backprop" << endl;
            net.backpropNew(bits);
        }
        cout << "Batch " << i  << " finished" << endl; // << "Gradient descend" << endl;
        net.gradDesNew();
        costFile << net.totCost << endl;  // Send cost to file
        cout << "Cost: " << net.totCost << endl << endl;
        net.totCost = 0;
    }
    costFile.close(); */
    //net.saveNetNew(); // Save net as [FILENAME]
    /* for (int i = 0; i < 16; i ++){
        bitConvs[i] = ((float)1/(float)16)*i;
    }
    cout << "Getting File Names" << endl;
    getFilenames(); // IMPORTANT: Is now using the small directory of 100 entries
    //getBits();
    cout << "Initializing" << endl;
    //net.initAll(5, laySize);
    net.loadNet("TTS_AI_V1.txt");
    //cout << "Randomizing" << endl;
    //net.ran();
    ofstream costFile("TTS_AI_cost.txt"); // IMPORTANT: Adjust name before running
    net.adjustBatch(batch);
    for (int i = 0; i < 30; i ++){ // Batches
        for (int b = 0; b < batch; b ++){ // Runs in batch
            cout << "Run " << b << " Input ";
            net.input(getBits()); // Take the bits as input and output
            cout << "Calculate ";
            net.calculate();
            cout << "Backprop" << endl;
            net.backprop(bits);
        }
        cout << "Batch " << i  << " finished" << endl << "Gradient descend" << endl;
        net.gradDes();
        costFile << net.totCost << endl;  // Send cost to file
        cout << "Cost: " << net.totCost << endl << endl;
        net.totCost = 0;
    }
    costFile.close();
    net.saveNet(); // Save net as [FILENAME] */
    /* int f = 0;
    while (true){
        cout << " Run " << f << "\nInput" << endl;
        net.input(getBits());
        cout << "Calculate" << endl;
        net.calculate();
        cout << "Backprop" << endl << endl;
        net.backprop(getBits());
        cout << "Run " << f  << " finished" << endl;
        costFile << net.totCost << endl;
        cout << "Cost: " << net.totCost << endl << endl;
        net.totCost = 0;
        f ++;
        cout << "Randomizing" << endl;
        net.ran();
    } */
    return 0;
}
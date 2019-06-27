// Devin DeWitt
// C++ code to sample an analog-to-digital converter and send the data over TCP socket to Java application

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <cerrno>
#include <time.h>
#include <iostream>
#include <math.h>
#include <cmath>
#include <bitset>
#include <wiringPi.h>
#define    MOSI   0  // Physical pin 11
#define    MISO   2  // Physical pin 13
#define SCLK   1  // Physical pin 12
#define CS    10 // Physical pin 24

using namespace std;

void cmdLnCheck(int argc, char* argv[]);
void sendMessage(int clientSocket, string data);
int recvMessage(int clientSocket);
void changeFunction(int clientSocket, int& currentChannel, int& currentFilter);
int spiRW(int channel);

static const int START = 1;
static const int RECVLENGTH = 4;
static const int SINGLE = 1;
static const int CHANNELBIT = 6;
static const int FILTERBIT = 7;

int main(int argc, char *argv[])
{
    struct sockaddr_in serverIP, clientIP;
    int serverSocket, clientSocket, portNum, opt = 1, sinVal;
    int addrLen = sizeof(clientIP), channel = 0, filter = 1;
    double time1 = 0;
    string binary;
    int data[16] = {0};
    wiringPiSetup();
    pinMode(MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(SCLK, OUTPUT);
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
    digitalWrite(SCLK, HIGH);
    
    cout << "Creating socket...";
    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cerr << "Couldn't create socket...Exiting Program...";
        return errno;
    }
    
    cout << "Socket Created" << endl;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                  &opt, sizeof(opt))){
        cerr << "setsockopt error...Exiting program...";
        return errno;
    }
    
    serverIP.sin_family = AF_INET;
    serverIP.sin_port = htons(0);
    serverIP.sin_addr.s_addr = INADDR_ANY;

    // Binding socket
    cout << "Binding socket...";
    if(bind(serverSocket, (struct sockaddr*) &serverIP, sizeof(serverIP)) < 0){
        cerr << "Could not bind socket. Exiting program." << endl;
        return 0;
    }
    int len = sizeof(serverIP);
    getsockname(serverSocket, (struct sockaddr*)&serverIP, (socklen_t*)&len);
    int myPort = ntohs(serverIP.sin_port);
    printf("Local port: %u\n", myPort);
    cout << "Socket Bound" << endl;
    
    // Setting socket to listen for client
    if(listen(serverSocket, 2)){
        cerr << "Error listening for connection. Exiting program.";
        return 0;
    }
    
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientIP,
                          (socklen_t*)&addrLen);
    cout << "Client Connected" << endl;
    while(1){
        switch(filter){
            case 1: sinVal = spiRW(channel);
                break;
            case 2:
                //Test channel to generate sin wave
                sinVal = (512 * (channel + 1)) * sin(2 * M_PI * 1 * time1) + 2048;
                time1 += .0008;
                break;
        }
        binary = bitset<16>(sinVal).to_string();
        sendMessage(clientSocket, binary);
        changeFunction(clientSocket, channel, filter);
        usleep(800);
    }
    return 0;
}

void cmdLnCheck(int argc, char* argv[])
{
    if(argc < 2){
        cout << "Missing command line argument. Please enter port number:";
        cin >> *argv[1];
    }
}

void sendMessage(int clientSocket, string data)
{
    char buffer[17] = {0};
    int sendSize = 0, messageSize = 17;
    // Creating a buffer that stores size of the message, followed by the message
    strncpy(buffer, data.c_str(), 16);
    buffer[16] = '\n';
    // Sending data to client
    while(sendSize < messageSize){
        sendSize += send(clientSocket, buffer, messageSize, 0);
    }
    return;
}

int recvMessage(int clientSocket)
{
    long incomingNum = 0;
    recv(clientSocket, &incomingNum, sizeof(incomingNum), MSG_DONTWAIT);
    return incomingNum;
}

void changeFunction(int clientSocket, int& currentChannel, int& currentFilter)
{
    int newFunction = 0;
    int channelBitCheck = 1 << CHANNELBIT, filterBitCheck = 1 << FILTERBIT;
    newFunction = recvMessage(clientSocket);
    if((channelBitCheck & newFunction)){
        newFunction = newFunction - pow(2, CHANNELBIT);
        cout << "channel " << newFunction << endl;
        if(newFunction > 7 || newFunction < 0){
            return;
        }else{
            currentChannel = newFunction;
            return;
        }
    }else if(filterBitCheck & newFunction){
        newFunction = newFunction - pow(2, FILTERBIT);
        cout << "filter " << newFunction << endl;
        if (newFunction != 1 && newFunction != 2) {
            return;
        } else {
            currentFilter = newFunction;
            return;
        }
    }else{
        return;
    }
}
int spiRW(int channel)
{
    int sClock = 0, value = 0, delayTime = 1;
    int data[5] = {START, SINGLE};
    switch(channel + 1){
        case 1: data[2] = 0; data[3] = 0; data [4] = 0;
            break;
        case 2: data[2] = 0; data[3] = 0; data [4] = 1;
            break;
        case 3: data[2] = 0; data[3] = 1; data [4] = 0;
            break;
    }
    digitalWrite(CS, LOW);
    digitalWrite(SCLK, LOW);
    delayMicroseconds(delayTime);
    for(sClock = 0; sClock < 5; sClock++){
        digitalWrite(SCLK, HIGH);
        digitalWrite(MOSI, data[sClock]);
        delayMicroseconds(delayTime);
        digitalWrite(SCLK, LOW);
        delayMicroseconds(delayTime);
    }
    digitalWrite(SCLK, HIGH);
    delayMicroseconds(delayTime);
    digitalWrite(SCLK, LOW);
    delayMicroseconds(delayTime);
    digitalWrite(SCLK, HIGH);
    delayMicroseconds(delayTime);
    digitalWrite(SCLK, LOW);
    delayMicroseconds(delayTime);
    for(sClock = 0; sClock < 12; sClock++){
        digitalWrite(SCLK, HIGH);
        delayMicroseconds(delayTime);
        digitalWrite(SCLK, LOW);
        delayMicroseconds(delayTime);
        value += (0 | digitalRead(MISO)) << (11 - sClock);
    }
    digitalWrite(CS, HIGH);
    return value;
}

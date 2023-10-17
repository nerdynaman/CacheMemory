#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
using namespace std;
#define SETS 16
#define ASSOCIATIVITY 4

// [25bits: tag][4bits: index][3bits: offset]
struct CacheLine {
    int valid;
    int tag;
    int data[32]; // 32 bytes = 8 words
};
sem_t *address;
sem_t *data;
sem_t *transferAddress;
sem_t *transferData;
int addressInput;
int addressFIFO;
int dataFIFO;
int transfer_address, transfer_data;
struct CacheLine cache[SETS][ASSOCIATIVITY];

void load(){
	// check if any process has acquired the address bus and data bus then read the address
	while(1){
		if (sem_trywait(address) == 0) {
			cout << "address bus not acquired" << endl;
			sem_post(address);
		}
		else{
			cout << "address bus busy" << endl;
			break;
		}
		sleep(2);
	}
	addressFIFO = open("address_bus", O_RDONLY);
	dataFIFO = open("data_bus", O_WRONLY);
	read(addressFIFO, &addressInput, sizeof(addressInput));

    // convert adderss to binary and first 23 bits are tag, next 4 bits are index, last 5 bits are offset
    int tag = addressInput >> 7;
    int index = (addressInput >> 3) & 0b1111;
    int offset = addressInput & 0b111;
    // cout << "tag " << tag << endl;
    // cout << "index " << index << endl;
    // cout << "offset " << offset << endl;
    if (addressInput > 2047) {
        printf("Invalid address\n");
		write(dataFIFO, &addressInput, sizeof(addressInput));
		return;
    }
    int cache_hit = 0;
    int dataN = 0;
    for (int i = 0; i < 4; i++) {
        if (cache[index][i].valid && cache[index][i].tag == tag) {
            cache_hit = 1;
			std::cout << "cache hit" << std::endl;
            for (int j = 0; j < 4 ; j++) {  //this for loop is just for getting integer data from 4 blocks
                dataN = dataN * 10;
                dataN = dataN + cache[index][i].data[(offset*4)+j];
            }
			break;
        }
    }

    if (!cache_hit) {
        std::cout << "cache miss" << std::endl;
		sem_wait(transferAddress);
		sem_wait(transferData);
		
		write(transfer_address, &addressInput, sizeof(addressInput));
		sleep(1);
		sem_post(transferAddress);
		int data[32];
		read(transfer_data, &data, sizeof(data));
		sem_post(transferData);
		// close(transfer_address);
		// close(transfer_data);

        // dataN =  memoryTransfer(address);
		int replace_index = 0;
		int cacheLineValid = 0;
		for (int i = 0; i < ASSOCIATIVITY; i++) {
		    if (!cache[index][i].valid) {
		        cacheLineValid = 1;
		        replace_index = i;
		        break;
		    }
		}
		if (!cacheLineValid) {
			replace_index = rand() % ASSOCIATIVITY;
		}
		cache[index][replace_index].valid = 1;
		cache[index][replace_index].tag = tag;
		for (int i = 0; i < 32; i++) {
		    cache[index][replace_index].data[i] = data[i];
		}
		for (int j = 0; j < 4 ; j++) { //this for loop is just for getting integer data from 4 blocks
			dataN = dataN *10;
			dataN = dataN + data[(offset*4)+j];
		}
    }
	write(dataFIFO, &dataN, sizeof(dataN));
	close(addressFIFO);
	close(dataFIFO);
}

int main(){
	sem_unlink("address");
	sem_unlink("data");

	mknod("address_bus", S_IFIFO | 0666, 0);
	mknod("data_bus", S_IFIFO | 0666, 0);
	mknod("transfer_address_bus", S_IFIFO | 0666, 0);
	mknod("transfer_data_bus", S_IFIFO | 0666, 0);
	
	address = sem_open("address", O_CREAT, 0666, 1);
	// data = sem_open("data", O_CREAT, 0666, 1);
	sem_open("data", O_CREAT, 0666, 1);
	transferAddress = sem_open("transferAddress", O_CREAT, 0666, 1);
	transferData = sem_open("transferData", O_CREAT, 0666, 1);

	transfer_address = open("transfer_address_bus", O_WRONLY);
	transfer_data = open("transfer_data_bus", O_RDONLY); 

	while(1){
		load();
	}

}
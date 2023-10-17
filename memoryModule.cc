#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
using namespace std;

#define MAIN_MEMORY_SIZE 8192

int main_memory[MAIN_MEMORY_SIZE];
sem_t *transferAddress;
sem_t *transferData;

int READY = 0;
int VALID = 0;


// Initialize the main memory with incrementing values
void initialize_main_memory() {
    for (int i = 0; i < MAIN_MEMORY_SIZE; i++) {
        main_memory[i] = i%10;
    }
}

void mainMemoryAccess(int address, int *data) {
    
	if (!VALID){
        return;
    }
    if ( data == NULL) {
		READY = 1;
		cout << "READY high for address " << address << endl;
        return;
    }
    // int data[32];
    int addr = address & 0b11111111111;
    // Read 32 bytes from main memory
    for (int i = 0; i < 32; i++) {
        data[i] = main_memory[(addr/8)*32 + i];
    }
    READY = 0;
	cout << "READY low for address " << address << endl;
}

int* memoryTransfer(int address){

    if (READY || VALID) {
        return memoryTransfer(address);
    }
    VALID = 1;
	cout << "VALID high for address " << address << endl;
    int dataN[32];
    redo:
        mainMemoryAccess(address, NULL);
        if (READY){
			cout << "sending address " << address << endl;
            mainMemoryAccess(address, dataN);
        }
        else{
            goto redo;
        }
    VALID = 0;
	cout << "VALID low for address " << address << endl;
	return dataN;
}

int main() {
	// Initialize the main memory
	initialize_main_memory();
	// Initialize the semaphores
	sem_unlink("transferAddress");
	sem_unlink("transferData");
	transferAddress = sem_open("transferAddress", O_CREAT, 0666, 1);
	transferData = sem_open("transferData", O_CREAT, 0666, 1);
	// Initialize the FIFOs
	mknod("transfer_address_bus", S_IFIFO | 0666, 0);
	mknod("transfer_data_bus", S_IFIFO | 0666, 0);
	// Open the FIFOs
	int addressFIFO = open("transfer_address_bus", O_RDONLY);
	int dataFIFO = open("transfer_data_bus", O_WRONLY);
	// Read from the address FIFO
	int addressInput;
	int dataOutput[32];

	while (1) {
		while (1){
			if (sem_trywait(transferAddress) == 0) {
				cout << "transferAddress not acquired" << endl;
				sem_post(transferAddress);
			}
			else {
				cout << "transferAddress acquired" << endl;
				break;
			}
			sleep(2);
		}
		
		read(addressFIFO, &addressInput, sizeof(addressInput));
		cout << "Address input: " << addressInput << endl;
		int *data = memoryTransfer(addressInput);
		for (int i = 0; i < 32; i++) {
			dataOutput[i] = data[i];
		}
		write(dataFIFO, &dataOutput, sizeof(dataOutput));
	}
	return 0;
}
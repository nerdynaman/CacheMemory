#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std;
int main() {
    // Initialize the main memory
	mknod("address_bus", S_IFIFO | 0666, 0);
	mknod("data_bus", S_IFIFO | 0666, 0);
	

	
	sem_t *address = sem_open("address", O_CREAT, 0666, 1);
	sem_t *data = sem_open("data", O_CREAT, 0666, 1);
	
	sem_wait(address);
	cout << "address bus acquired" << endl;
	int fd = open("address_bus", O_WRONLY);
	sem_wait(data);
	cout << "data bus acquired" << endl;

	int addressInput;
	int dataOutput;	
	cout << "Enter an address(in dec): ";
	cin >> addressInput;
	
	write(fd, &addressInput, sizeof(addressInput));
	sem_post(address);
	int fd2 = open("data_bus", O_RDONLY);
	read(fd2, &dataOutput, sizeof(dataOutput));
	cout << "Data at address " << addressInput << " is " << dataOutput << endl;
	
	sem_post(data);
	close(fd);
	close(fd2);
    return 0;
}
This program simulates a 4 way set associative cache system as implemented in computer architecture.

Each module is a separate process and communicates with each other using FIFO. There are 2 FIFOs between memory and cache and 2 FIFOs between cache and user process. One FIFO is used to send data and the other is used to send the address. To control the flow of data so that each module knows when to read and write, semaphores are used.

## How to run
1. Clone the repository
2. ```bash
	g++ cacheModule.cpp -o cacheModule
	g++ memoryModule.cpp -o memoryModule
	g++ userProcess.cpp -o userProcess
   ```
3. In one terminal run ```./memoryModule```
4. In second terminal run ```./cacheModule```
5. In third terminal run ```./userProcess```
6. Follow the instructions on the screen

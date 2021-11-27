# Multi-Threading-Problems
A collection of problems solved using multi-threading

[Google Doc](https://docs.google.com/document/d/1LH7CpG3SVDEIzdhZy4txEEdMF-oicjZPpAZt9xP3dLE/edit)
## Q2: The Queue at the Polling Booth
### Entity Diagram
![Entity Diagram](/snapshots/evm_entity_diagram.png)
### System Diagram
![Thread Diagram](/snapshots/evm_thread_diagram.png)

### Instructions:
1. First input the number of Booths.
2. For each Booth input:
    1. Number of Evms per Booth.
    2. Maximum number of free slots in a Evm per Booth.
    3. Number of Voters initially assigned to enter the Booth.
### Results:
Results are displayed as logs of how each task is being performed.


## Q3: Concurrent Merge Sort

### To execute multiprocess mergesort
- Compile the code file process_mergesort.cpp 
    >g++ process_mergesort.cpp
- Run the code
    >./a.out
### To execute mergesort using two threads for every subarray
- Compile the code file fixed_thread_mergesort.cpp 
    >g++ fixed_thread_mergesort.cpp
- Run the code
    >./a.out
### To execute mergesort using fixed number of threads
- Compile the code file thread_mergesort.cpp 
    >g++ thread_mergesort.cpp
- Run the code
    >./a.out

Change the size of array in the code at #define size for executing the code for different size.

To print the array at any instance , uncomment the corresponding comment-lines in the code
(eg. For printing the merging step by step)

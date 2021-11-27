# Problems on Process Co-ordination

## Directory Structure

```
.
├── Project_Report_Kudos.pdf
├── Q1
│   └── badminton_academy.cpp
├── Q2
│   └── queue-booth-problem.cpp
├── Q3
│   ├── Part A
│   │   └── process_mergesort.cpp
│   └── Part B
│       ├── fixed_thread_mergesort.cpp
│       └── thread_mergesort.cpp
└── readme.md
```

---

## Problem 1 Badminton Academy Problem

### Run Instructions

- Compile the program by following command
    > g++ badminton_academy.cpp -pthread

- Run the program by following command
    > ./a.out

### Program Execution Steps

- Input number of groups expected to come as an integer

---

## Problem 2 The Queue at the Polling Booth

### Run Instructions

- Compile the program by following command
    > g++ queue-booth-problem.cpp -pthread

- Run the program by following command
    > ./a.out

### Program Execution Steps

1. First input the number of Booths.
2. For each Booth input:
    1. Number of Evms per Booth.
    2. Maximum number of free slots in a Evm per Booth.
    3. Number of Voters initially assigned to enter the Booth.

---

## Problem 3 Concurrent Merge Sort

### Run Instructions

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

- Change the size of array in the code at #define size for executing the code for different size.

- To print the array at any instance , uncomment the corresponding comment-lines in the code
(eg. For printing the merging step by step)

---

#include <iostream>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
using namespace std;
//change the size of array
#define size 1000
//array for sorting using thread
long long arr[size];

//array for sorting using process
long long arr_p[size];

//for storing the beginning and end index of arrays for threads
struct info
{
    long long beg;
    long long end;
};

//for printing array
void print_array(long long j, long long k, long long *arr)
{

    for (long long i = j; i < k; i++)
    {
        cout << arr[i] << " ";
    }

    cout << endl;
}

//for merging the sorted array
void merging(long long beg, long end, long long mid, long long *arr)
{
    long long helper_array[end - beg + 1];
    long long i = beg;
    long long j = mid + 1;
    long long index = 0;
    while (i <= mid && j <= end)
    {
        if (arr[i] < arr[j])
        {
            helper_array[index++] = arr[i];
            i++;
        }
        else
        {
            helper_array[index++] = arr[j];
            j++;
        }
    }
    while (i <= mid)
    {
        helper_array[index++] = arr[i++];
    }
    while (j <= end)
    {
        helper_array[index++] = arr[j++];
    }
    for (long long p = 0; p <= end - beg; p++)
    {
        arr[beg + p] = helper_array[p];
    }

}
//to check sorted or not
void issorted(long long arr[])
{

    for (int i = 1; i < size; i++)
    {
        if (arr[i] < arr[i - 1])
        {
            cout << "\nUnsorted at " << arr[i] << endl;
            return;
        }
    }
    printf("Array is in sorted order\n");
}
// Normal merge sort
void process_mergesort(long long low, long high, long long *arr_p)
{
    long long mid;
    if (low < high)
    {
        mid = (low + high) / 2;

        // Divide the array into two half.
        process_mergesort(low, mid, arr_p);
        process_mergesort(mid + 1, high, arr_p);

        // Merge them to get sorted array.
        merging(low, high, mid, arr_p);
    }
}

//merge sort for thread
void *merge_sort(void *param)
{
    struct info arr_info = *((info *)param);
    long long start = arr_info.beg;
    long long end = arr_info.end;

    if (start >= end)
        return NULL;

    pthread_t th1;
    pthread_t th2;
    struct info obj1;
    obj1.beg = start;
    long long mid = (start + (end)) / 2;
    obj1.end = mid;
    struct info obj2;
    obj2.beg = mid + 1;
    obj2.end = end;

    //create two threads for each sub array
    pthread_create(&th1, NULL, merge_sort, &obj1);
    pthread_create(&th2, NULL, merge_sort, &obj2);

    // wait for thread to terminate
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    //merging sorted array
    merging(start, end, mid, arr);

    //terminate thread execution
    pthread_exit(NULL);

}
int main()
{
    cout << "Program for merge sort using threads\n";

    //Filling random values in array to make it Unsorted
    
    srand(time(NULL));
    for (long long i = 0; i < size; i++)
    {
        arr[i] = rand() % size;
        arr_p[i] = arr[i];
    }
    //uncomment the code to print Unsorted array
    //cout << "Unsorted Array is\n";
    // print_array(0,size,arr);

    pthread_t t;
    struct info arr_info = {0, size - 1};

    //for storing execution time 
    clock_t beforesort;
    clock_t aftersort;

    //start time
    beforesort = clock();

    //creating a thread for starting merge sort operation
    pthread_create(&t, NULL, merge_sort, &arr_info);

    // wait for thread to terminate
    pthread_join(t, NULL);

    //completion time
    aftersort = clock();

    //uncomment to check the sorted array
    // issorted(arr);

    //uncomment to see the sorted array
    /*cout << "Sorted Array is\n";
     print_array(0,size,arr);*/

    clock_t beforesort1;
    clock_t aftersort1;

    // for Normal merge sort Execution
    cout << "Program for merge sort using normal code\n";

    //uncomment the code to print Unsorted array
    // print_array(0,size,arr_p);

    //starting time
    beforesort1 = clock();
    process_mergesort(0, size - 1, arr_p);

    //completion time
    aftersort1 = clock();

    //uncomment to check the sorted array
    // issorted(arr_p);

    //uncomment the code to print sorted array
    /*cout<<"Sorted Array\n";
    // print_array(0,size,arr_p);*/

    //Printing execution time
    cout << "\nTime elasped with thread: " << (aftersort - beforesort) / (double)CLOCKS_PER_SEC << endl;
    cout << "\nTime elasped with normal_code: " << (aftersort1 - beforesort1) / (double)CLOCKS_PER_SEC << endl;
    return 0;
}

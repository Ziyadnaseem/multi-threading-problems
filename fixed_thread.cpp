#include <iostream>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
using namespace std;
//change this for diiferent size
#define size 10000

//array for thread
long long arr[size];

//array for normal sort
long long arr_p[size];

//Initalizing the variables with some default value
long long total_thread = 2;
long long remainders = 0;
long long arr_div = size / total_thread;

//for printing array
void print_array(long long j, long long k, long long *arr)
{

    for (long long i = j; i < k; i++)
    {
        cout << arr[i] << " ";
    }

    cout << endl;
}

//for merging
void merging(long long beg, long long end, long long mid, long long *arr)
{
    if (end >= (size))
        end = size - 1;
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
void helper_merge_sort(long long *arr, long long low, long long high)
{
    long long mid;
    if (low < high)
    {
        mid = (low + high) / 2;
        // Divide the array into two half.
        helper_merge_sort(arr, low, mid);
        helper_merge_sort(arr, mid + 1, high);

        // Merge them to get sorted array.
        merging(low, high, mid, arr);
    }
}
void *merge_sort(void *param)
{
    long long thread_num = (long long)param;
    long long start = thread_num * (arr_div);
    long long end = ((thread_num + 1) * (arr_div)-1);

    // If it is last thread, assign the remainder number of array elements
    if (thread_num == total_thread - 1)
    {
        end += remainders;
    }
    long long mid = (start + (end)) / 2;
    if (start < end)
    {
        helper_merge_sort(arr, start, mid);
        helper_merge_sort(arr, mid + 1, end);
        merging(start, end, mid, arr);
        //uncomment the code to see the sorting of sub arrays
        /*cout << "\n"
             << start << " " << mid << " " << end;
        cout << endl;
        print_array(start, end + 1, arr);*/
    }
    return NULL;
}

//for merging the sorted sub arrays of threads
void merge_local_arrays(long long level, long long thread_cnt, long long *arr)
{
    if ((thread_cnt) < 1)
        return;

    for (int i = 0; i < thread_cnt; i += 2)
    {
        long long start = arr_div * i * level;
        long long end = ((arr_div * (i + 2) * level) - 1);
        long long mid = (start + end) / 2;
        merging(start, end, mid, arr);

        //uncomment the below line to see the merging step by step
        /*cout<<"\n"<<start<<" "<<mid<<" "<<end<<endl;
        print_array(start,end+1,arr);*/
    }
    merge_local_arrays(level * 2, thread_cnt / 2, arr);
}

//Normal merge sort
void process_mergesort(long long low, long high, long long *arr_p)
{
    long long mid;
    if (low < high)
    {
        mid = (low + high) / 2;
        // Split the data into two half.
        process_mergesort(low, mid, arr_p);
        process_mergesort(mid + 1, high, arr_p);

        // Merge them to get sorted output.
        merging(low, high, mid, arr_p);
    }
}

//To check if array is sorted
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
    //uncomment below line for printing unsorted array
    //cout << "Unsorted Array is\n";
    //print_array(0,size,arr);

    cout << "Enter number of threads you want to create\n";
    cin >> total_thread;

    //assigning each thread equal number of array to sort
    arr_div = size / total_thread;

    //in case number of array cannot be divided equally among threads
    remainders = size % total_thread;

    //for merging level
    long long merge_level = 1;

    pthread_t threads[total_thread];
    clock_t beforesort;
    clock_t aftersort;

    //starting time
    beforesort = clock();

    //creating threads
    for (long long int i = 0; i < total_thread; i++)
    {

        if (pthread_create(&threads[i], NULL, merge_sort, (void *)i))
        {
            cout << "Error\n";
            exit(-1);
        }
    }

    // wait for threads to terminate
    for (long i = 0; i < total_thread; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    // Now merge all the sorted sub arrays of threads
    merge_local_arrays(merge_level, total_thread, arr);

    //completion time
    aftersort = clock();

    //uncomment to print sorted array
    /*cout<<"\nSorted array is \n";
    print_array(0,size,arr);*/
    
    //uncomment to check if array is sorted
    //issorted(arr);

    clock_t beforesort1;
    clock_t aftersort1;

    //for normal merge sort
    cout<<"\nfor normal merge sort\n";

    //starting time
    beforesort1 = clock();

    process_mergesort(0, size - 1, arr_p);

    //completion time
    aftersort1 = clock();

    //uncomment to print sorted array
    /*cout<<"\nSorted array is \n";
    print_array(0,size,arr_p);*/

    //uncomment to check if array is sorted
    // issorted(arr_p);

    //printing execution time
    cout << "\nTime elasped with thread: " << (aftersort - beforesort) / (double)CLOCKS_PER_SEC << endl;
    cout << "\nTime elasped with normal_code: " << (aftersort1 - beforesort1) / (double)CLOCKS_PER_SEC << endl;
    return 0;
}
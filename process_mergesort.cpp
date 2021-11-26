#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <time.h>
#include <chrono>
#include <ctime>
using namespace std::chrono;
using namespace std;

void selectionSort(int arr[], int n)						//selection sort
{
    int i,j,min_idx;
 
    for(i=0;i<n-1;i++)
    {
        min_idx=i;
        for(j=i+1;j<n;j++)
        if(arr[j]<arr[min_idx])
        {
            min_idx=j;
 	}
        swap(arr[min_idx],arr[i]);
    }
}

void merge(int a[], int l1, int h1, int h2)					//simple merge function
{
    int count=h2-l1+1;
    int sorted[count];
    int i=l1, k=h1+1, m=0;
    while(i<=h1 && k<=h2)
    {
        if(a[i]<a[k])
        {
            sorted[m++]=a[i++];
        }
        else if(a[k]<a[i])
        {
            sorted[m++]=a[k++];
        }
        else if(a[i]==a[k])
        {
            sorted[m++]=a[i++];
            sorted[m++]=a[k++];
        }
    }
 
    while(i<=h1)
    {
        sorted[m++]=a[i++];
    }
    while(k<=h2)
    {
        sorted[m++]=a[k++];
    }
    int arr_count=l1;
    for(i=0;i<count;i++)
    {
        a[l1]=sorted[i];
        l1++;
    }
}
 
void multiproc_mergesort(int a[], int l, int h)
{
    int len=(h-l+1);
 
    if(len<5)
    {
        selectionSort(a+l,len);						//for elements less than 5, call selection sort
        return;
    }
 
    pid_t lpid,rpid;
    lpid=fork();								//fork call
    if(lpid<0)
    {
        cout<<"Left child process not created.\n";
        _exit(1);
    }
    else if(lpid==0)								//this child process will sort the left subarray
    {
        multiproc_mergesort(a,l,l+len/2-1);
        _exit(0);
    }
    else
    {
        rpid=fork();								//another fork call
        if(rpid<0)
        {
            cout<<"Right child process not created.\n";
            _exit(1);
        }
        else if(rpid==0)							//this child process will sort the right subarray
        {
            multiproc_mergesort(a,l+len/2,h);
            _exit(0);
        }
    }
 
    int status;
    waitpid(lpid,&status,0);
    waitpid(rpid,&status,0);							//the parent waits for the child processes to complete
 
    merge(a,l,l+len/2-1,h);							//the parent merges the result of the child processes
}
 

void normal_mergesort(int arr_p[],int low,int high)
{
    int mid;
    if(low<high)
    {
        mid=(low+high)/2;
        normal_mergesort(arr_p,low,mid);
        normal_mergesort(arr_p,mid+1,high);
        merge(arr_p,low,mid,high);
    }
}

bool isSorted(int a[],int n)							//function to check if the array is sorted or not
{
    if(n==1)
    {
        return 1;
    }
    for(int i=1;i<n;i++)
    {
        if(a[i]<a[i-1])
        {
            return 0;
        }
    }
    return 1;
}
 
void fillData(int a[],int n)							//function that fills random numbers into an array
{
    for(int i=0;i<n;i++)
    {    
    	a[i]=rand();
    }
    return;
}
 

int main()
{
    int *shm_array,*normal_array,n;
    cout<<"Enter the number of elements: ";
    cin>>n;
    int arr2[n];
    normal_array=arr2;
    
    int shmid=shmget(IPC_PRIVATE, n*sizeof(int), IPC_CREAT | 0666);		//create a shared memory space of size n*sizeof(int)
    if(shmid<0)								
    {
        cout<<"Error while allocating shared memory.\n";
        _exit(1);
    }
   
    if((shm_array=(int *)shmat(shmid, NULL, 0))==(int *)-1)			//attach a pointer to the created memory space
    {
	cout<<"Error while attaching our data space with allocated memory.\n";
        _exit(1);
    }
 
    srand(time(NULL));
    fillData(shm_array,n);							//fill the shared memory space with random numbers
    for(int i=0;i<n;i++)
    {
    	arr2[i]=*(shm_array+i);						//fill the normal array with the same numbers as above
    }
    
    auto start1=high_resolution_clock::now();
    normal_mergesort(normal_array,0,n-1);					//call normal merge sort
    auto stop1=high_resolution_clock::now();
    
    auto start2=high_resolution_clock::now();
    multiproc_mergesort(shm_array,0,n-1);					//call multiprocess merge sort
    auto stop2=high_resolution_clock::now();
    
    auto duration1=duration_cast<microseconds>(stop1-start1);		//calculate execution time in microseconds
    auto duration2=duration_cast<microseconds>(stop2-start2);
   
    if(isSorted(normal_array,n))						//check if normal merge sorting is done
    {
    	cout<<"\nSorted successfully using normal merge sort.";
    }
    else
    {
    	cout<<"\nFailed to sort using normal merge sort.";
    }
    if(isSorted(shm_array,n))							//check if multiprocessed merge sorting is done
    {
    	cout<<"\nSorted successfully using multiprocess merge sort.\n";
    }
    else
    {
    	cout<<"\nFailed to sort using multiprocess merge sort.\n";
    }
    
    cout<<"\nTime taken by normal merge sort: "<<duration1.count()<<" microseconds."<<endl;
    cout<<"Time elasped with multiprocessed merge sort: "<<duration2.count()<<" microseconds."<<endl;
    
    if(shmdt(shm_array)==-1)							//detach the pointer to the memory space
    {
        cout<<"Error while detaching the segment.\n";
        _exit(1);
    }
    if(shmctl(shmid,IPC_RMID,NULL)==-1)					//deallocate the memory space
    {
        cout<<"Error while deallocating the shared memory.\n";
        _exit(1);
    } 
    return 0;
}


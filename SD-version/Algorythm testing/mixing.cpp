// Example program
#include <iostream>
#include <string>
using namespace std;

// int numberCodes = {
//     {0, 1, 2},
//     {3, 4, 5},
//     {6, 7, 8},
//     {9, 10, 11},
//     {12, 13, 14},
//     {15, 16, 17},
//     {18, 19, 20},
//     {21, 22, 23},
//     {24, 25, 26},
//     {27, 28, 29},
//     {30, 31, 32},
//     {33, 34, 35},
//     {36, 37, 38},
//     {39, 40, 41},
//     {42, 43, 44},
//     {45, 46, 47},
// };

void reverseArray(int arr[], int start, int end)
{
    while (start < end)
    {
        int temp = arr[start];
        arr[start] = arr[end];
        arr[end] = temp;
        start++;
        end--;
    }
}

void mix(int arr[], int n){
    if(n > 16 && n < 1) return;
    int iterations;
    if(16 % n != 0){
        iterations = (16 / n) + 1;    
    }else{
        iterations = 16 / n; 
    }
    // cout<<iterations<<endl;
    int start = 0;
    int end = n - 1;
    for(int i = 0; i < iterations; i++){
        // cout<<start<<" "<<end<<"     ";
        reverseArray(arr, start, end);
        start += n;
        end += n;
        if(end > 15) end = 15;
        if(start > 15) start = 15;
        
    }
    cout<<endl;
    cout<<endl;
}

    
 

int main()
{
    int arr[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    for(int i = 1; i < 17; i++){
        mix(arr, i);
        for(int j = 0; j < 16; j++){
            cout<<arr[j]<<" ";
        }
        cout<<endl;
        mix(arr, i);
        for(int j = 0; j < 16; j++){
            cout<<arr[j]<<" ";
        }
        for(int j = 0; j < 16; j++){
            arr[j] = j;
        }
        cout<<endl;
        cout<<endl;
        cout<<endl;
    }
    
}
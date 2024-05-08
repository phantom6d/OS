#ifndef LAB2_QUICK_SORT_BURST_H
#define LAB2_QUICK_SORT_BURST_H
#include "task.h"

void swap(Task* a, Task* b)
{
    Task temp = *a;
    *a = *b;
    *b = temp;
}

int partition(Task arr[], int left, int right)
{

    int pivot = arr[left].burst*(-1);
    int i = left;
    int j = right;

    while (i < j) {

        while (arr[i].burst*(-1) <= pivot && i <= right - 1) {
            i++;
        }

        while (arr[j].burst*(-1) > pivot && j >= left + 1) {
            j--;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[left], &arr[j]);
    return j;
}

void quick_sort(Task arr[], int left, int right)
{
    if (left < right) {

        int partition_index = partition(arr, left, right);
        quick_sort(arr, left, partition_index - 1);
        quick_sort(arr, partition_index + 1, right);
    }
}

#endif //LAB2_QUICK_SORT_BURST_H

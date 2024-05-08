#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"
#include "quick_sort_burst.h"

int task_cnt = 0;
Task arr[100];

void add(char *name, int priority, int burst) {
    arr[task_cnt].name = name;
    arr[task_cnt].priority = priority;
    arr[task_cnt].burst = burst;
    task_cnt += 1;
}


void schedule() {
    quick_sort(arr, 0, task_cnt);
    while (task_cnt) {
        --task_cnt;
        run(&arr[task_cnt], arr[task_cnt].burst);
    }
}
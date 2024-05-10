#include <stdio.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"
#include "quick_sort_priority.h"
#define SIZE 100

int task_cnt = 0;
Task arr[SIZE];
int prior_cnt[SIZE] = {0};

struct node *tasks = NULL;
struct node *curr = NULL;

void add(char *name, int priority, int burst) {
    arr[task_cnt].name = name;
    arr[task_cnt].priority = priority;
    arr[task_cnt].burst = burst;
    prior_cnt[priority]++;
    task_cnt += 1;
}

void next_task(){
    curr = curr->next;
    if (curr == NULL) {
        curr = tasks;
    }
}

void schedule_rr() {
    curr = tasks;
    while (tasks) {
        if (curr->task->burst > QUANTUM){
            run(curr->task, QUANTUM);
            curr->task->burst -= QUANTUM;
        }
        else {
            run(curr->task, curr->task->burst);
            curr->task->burst = 0;
            delete(&tasks, curr->task);
        }
        next_task();
    }
}

void schedule() {
    quick_sort(arr, 0, task_cnt - 1);
    while (task_cnt) {
        if (prior_cnt[arr[task_cnt - 1].priority] == 1) {
            run(&arr[task_cnt - 1], arr[task_cnt - 1].burst);
            arr[task_cnt - 1].burst = 0;
            --task_cnt;
        }
        else {
            int priority = arr[task_cnt - 1].priority;
            while (prior_cnt[priority]) {
                insert(&tasks, &arr[task_cnt - 1]);
                --task_cnt; --prior_cnt[priority];
            }
            schedule_rr();
        }
    }
}
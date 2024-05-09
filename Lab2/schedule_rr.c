#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

struct node *tasks = NULL;
struct node *curr = NULL;

void add(char *name, int priority, int burst) {
    Task *new_task = malloc(sizeof(Task));
    new_task->name = name;
    new_task->priority = priority;
    new_task->burst = burst;
    insert(&tasks, new_task);
}

void next_task(){
    curr = curr->next;
    if (curr == NULL) {
        curr = tasks;
    }
}

void reverse (){
    struct node *buffer = NULL;
    while (tasks) {
        insert (&buffer, tasks->task);
        tasks = tasks->next;
    }
    free(tasks);
    tasks = buffer;
}

void schedule() {
    reverse();
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
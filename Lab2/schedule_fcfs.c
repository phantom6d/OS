#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

struct node *tasks = NULL;

Task *pick_next_task() {
    if (!tasks){
        return NULL;
    }

    Task *task = tasks->task;
    delete(&tasks, task);

    return task;
}

void reverse (){
    struct node *buffer = NULL;

    while (tasks) {
        insert (&buffer, tasks->task);
        pick_next_task();
    }
    free(tasks);
    tasks = buffer;
}

void add(char *name, int priority, int burst) {
    Task *new_task = malloc(sizeof(Task));
    new_task->name = name;
    new_task->priority = priority;
    new_task->burst = burst;
    insert(&tasks, new_task);
}

void schedule() {
    reverse();
    while (tasks) {
        run(tasks->task, tasks->task->burst);
        pick_next_task();
    }
}
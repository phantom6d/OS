#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

struct command {
    char *name;
    char *desc;
    void (*link) (char *[]);
};

struct command commands[];

int cmd_count();

void cmd_cd(char **args) {
    if (args[1] == NULL) {
        char path[256];
        getcwd(path,256);
        printf("%s\n", path);
        return;
    }

    if (chdir(args[1])) {
        perror("cd");
        return;
    }

}

void cmd_clr(char **args) {
    printf("\033c");
}

void cmd_dir(char **args) {
    if (args[1] == NULL) {
        printf("Usage: dir <directory>\n");
        return;
    }

    DIR *dir;
    struct dirent *entry;

    dir = opendir(args[1]);
    if (dir == NULL) {
        perror("dir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

void cmd_environ(char **args) {
    extern char **environ;

    int i = 0;
    while(environ[i]) {
        printf("%s\n", environ[i++]);
    }

}

void cmd_echo(char **args) {
    if (args[1] == NULL) {
        printf("to use echo, please, write something.\n");
        return;
    }
    printf("%s\n", args[1]);
}

void cmd_help(char **args) {
    for (int i = 0; i < cmd_count(); i++){
        printf(commands[i].desc);
        printf("\n");
    }
}

void cmd_pause(char **args) {
    printf("Press any key...\n");
    getchar();
}

void cmd_quit(char **args) {
    exit(0);
}

struct command commands[] = {
        {"cd", "switch directory to <directory>, if its impossible due to lack of <directory>, output current one.", &cmd_cd},
        {"clr", "clear the screen", &cmd_clr},
        {"dir", "show all files in <directory>", &cmd_dir},
        {"environ", "show all environmental variables", &cmd_environ},
        {"echo", "output <argument>", &cmd_echo},
        {"help", "show help", &cmd_help},
        {"pause", "wait for next output", &cmd_pause},
        {"quit", "stop work", &cmd_quit}
};

int cmd_count() {
    return sizeof(commands)/sizeof(struct command);
}

int main(int argc, char *argv[]) {

    char *in_pointer;
    size_t n;
    char *arr [20];
    char curr_path[256];
    realpath(argv[0], curr_path);
    setenv("shell", curr_path, 1);

    while (1) {


        in_pointer = NULL;
        getcwd(curr_path,256);
        printf("%s > ", curr_path);
        getline(&in_pointer, &n, stdin);
        arr[0] = strtok(in_pointer, " \n");
        int i = 1;
        while (arr[i - 1] != NULL) {
            arr[i] = strtok(NULL, " \n");
            i++;
        }

        int t = 0;
        for (; t < cmd_count(); t++){
            if (strcmp(arr[0], commands[t].name) == 0){
                commands[t].link(arr);
                break;
            }
        }
        if (t == cmd_count()){

            pid_t pid = fork();

            if (pid == 0) {
                execvp(arr[0], arr);
                perror("execvp");
            } else if (pid < 0) {
                perror("fork");
            } else {
                wait(NULL);
            }
        }

        free(in_pointer);
        printf ("\n");
        n = 0;

    }

    return 0;
}

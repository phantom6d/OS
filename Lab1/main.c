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
char is_background;

int cmd_count();

void cmd_cd(char **args) { // команда "cd" меняет нынешнюю директорию на указанную.
    if (args[1] == NULL) { // если аргумент для смены директории отсутствует, вывести текущую.
        char path[256];
        getcwd(path,256);
        printf("%s\n", path);
        return;
    }

    if (chdir(args[1])) { // если возможно, сменить директорию на указанную.
        perror("cd");
        return;
    }

}

void cmd_clr(char **args) { // команда "clr" очищает экран.
    printf("\033c");
}

void cmd_dir(char **args) { // команда "dir" выводит содержимое указанной директории.
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

    while ((entry = readdir(dir)) != NULL) { // полный обход указанной директории
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

void cmd_environ(char **args) { // команда "environ" выводит все переменные среды
    extern char **environ;

    int i = 0;
    while(environ[i]) {
        printf("%s\n", environ[i++]);
    }

}

void cmd_echo(char **args) { // команда "echo" выводит указанный текст
    if (args[1] == NULL) {
        printf("to use echo, please, write something.\n");
        return;
    }
    printf("%s\n", args[1]);
}

void cmd_help(char **args) { // команда "help" выводит список команд и пояснения к ним
    for (int i = 0; i < cmd_count(); i++){
        printf(commands[i].desc);
        printf("\n");
    }
}

void cmd_pause(char **args) { // команда "pause" останавливает работу до следующего нажатия клавиши
    printf("Press any key...\n");
    getchar();
}

void cmd_quit(char **args) { // команда "quit" завершает работу
    exit(0);
}

struct command commands[] = { // массив из структур, содержащих в себе имя, ссылку и помощь к команде.
        {"cd", "switch directory to <directory>, if its impossible due to lack of <directory>, output current one.", &cmd_cd},
        {"clr", "clear the screen", &cmd_clr},
        {"dir", "show all files in <directory>", &cmd_dir},
        {"environ", "show all environmental variables", &cmd_environ},
        {"echo", "output <argument>", &cmd_echo},
        {"help", "show help", &cmd_help},
        {"pause", "wait for next output", &cmd_pause},
        {"quit", "stop work", &cmd_quit}
};

int cmd_count() { // высчитывает кол-во команд посредством разделения размера массива на размер структуры, которая содержит одну команду
    return sizeof(commands)/sizeof(struct command);
}

int console_input;  // Стандартный консольный ввод
int console_output; // Стандартный консольный вывод

void restore_original_fd() {
    dup2(console_input, STDIN_FILENO);
    dup2(console_output, STDOUT_FILENO);
    dup2(console_output, STDERR_FILENO);
}

int handle_redirection(char **args) {
    int i = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    int append_mode = 0;

    // Поиск символов
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
            input_file = args[i + 1];
            args[i] = NULL; // Игнорируем текущий элемент
        } else if (strcmp(args[i], ">") == 0) {
            output_file = args[i + 1];
            args[i] = NULL; // Игнорируем текущий элемент
        } else if (strcmp(args[i], ">>") == 0) {
            output_file = args[i + 1];
            args[i] = NULL; // Игнорируем текущий элемент
            append_mode = 1;
        } else if (strcmp(args[i], "&") == 0) {
            args[i] = NULL;
            is_background = '1';
        }
        ++i;
    }

    // Перенаправление ввода
    if (input_file) {
        FILE *file = fopen(input_file, "r");
        if (file) {
            dup2(fileno(file), STDIN_FILENO);
            fclose(file);
        } else {
            perror("Error opening input file");
            restore_original_fd();
            return 1;
        }
    }

    // Перенаправление вывода
    if (output_file) {
        const char *mode = append_mode ? "a" : "w"; // Устанавливаем режим
        FILE *file = fopen(output_file, mode);

        if (file) {
            dup2(fileno(file), STDOUT_FILENO);
            dup2(fileno(file), STDERR_FILENO);
            fclose(file);
        } else {
            perror("Error opening output file");
            restore_original_fd();
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) { // основная функция

    if (argc == 2 && freopen(argv[1], "r", stdin) == NULL) {
        printf("Error: input file\n");
        exit(1);
    }

    console_input = dup(STDIN_FILENO); // Стандартный консольный ввод
    console_output = dup(STDOUT_FILENO); // Стандартный консольный выводь

    char *in_pointer; // указатель на строку
    size_t n; // размер строки
    char *arr [20]; // массив для аргументов
    char curr_path[256]; // массив для хранения пути
    realpath(argv[0], curr_path);
    setenv("shell", curr_path, 1); // присвоить переменной shell значение истинного пути

    while (1) { // бесконечный цикл для выполнения


        in_pointer = NULL;
        getcwd(curr_path,256); // получение пути для вывода
        printf("%s > ", curr_path);
        getline(&in_pointer, &n, stdin); // ввод от пользователя
        arr[0] = strtok(in_pointer, " \n");
        int i = 1;
        while (arr[i - 1] != NULL) { // разбиение полученной строки на аргументы
            arr[i] = strtok(NULL, " \n");
            i++;
        }

        if (handle_redirection(arr)) // попытка редиректа, в случае ошибки - пропуск команды
            continue;

        int t = 0;
        for (; t < cmd_count(); t++){ // при нахождении идентичной команды в массиве, выполнить ее
            if (strcmp(arr[0], commands[t].name) == 0){
                commands[t].link(arr);
                break;
            }
        }
        if (t == cmd_count()){ // если такой команды нет, то

            pid_t pid = fork(); // создание дочернего процесса

            if (pid == 0) {

                if (is_background == '1') {
                    // Перенаправляем stdout и stderr
                    freopen("/dev/null", "w", stdout);
                    freopen("/dev/null", "w", stderr);
                }

                execvp(arr[0], arr);
                perror("execvp");
            } else if (pid < 0) {
                perror("fork");
            } else {
                if (is_background == '0')
                    wait(NULL);
            }
        }

        free(in_pointer);
        restore_original_fd();
        is_background = '0';
        printf ("\n");
        n = 0;

    }

    return 0;
}

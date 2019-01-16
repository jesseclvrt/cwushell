#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

static const int BUFFER_SIZE = 256;
static char* DEFAULT_PROMPT = "cwushell";
static char* prompt = 0;

char* shell_read();
char** shell_parse(char*);
int execute(char**);
int startProcess(char**);

// BUILT-INs
int bi_manual();
int bi_cpuinfo(char** args);
int bi_meminfo(char** args);
int bi_prompt(char** args);
int bi_exit();

int main() {
    prompt = malloc(sizeof(char) * 9);
    char* input;
    char** parsed;
    int success = 0;
    prompt = DEFAULT_PROMPT;

    do {
        printf("%s> ", prompt);
        input = shell_read();
        parsed = shell_parse(input);
        success = execute(parsed);
        free(parsed);
    } while (success);
    if (parsed[1] != 0) {
        return atoi(parsed[1]);
    }
    return 0;
}

char* shell_read() {
    char* line = 0; //allow getline to decide size
    ssize_t size = 0;
    getline(&line, &size, stdin);
    return line;
}

char** shell_parse(char* line) {
    char** args = malloc(BUFFER_SIZE * sizeof(char*));
    char* token = 0;
    int position = 0;
    int newBuffer = BUFFER_SIZE;

    token = strtok(line, " \n\t\r\a");
    while (token != 0) {
        args[position] = token;
        position++;
        if (position >= newBuffer) { // buffer overrun
            newBuffer += BUFFER_SIZE;
            args = realloc(args, newBuffer * sizeof(char*));
        }
        token = strtok(0, " \n\t\r\a");
    }
    return args;
}

int execute(char** command) {
    if (command[0] == 0) {
        return 1;
    }
    if (strcmp(command[0], "manual") == 0) {
        return bi_manual();
    } else if (strcmp(command[0], "cpuinfo") == 0) {
        return bi_cpuinfo(command);
    } else if (strcmp(command[0], "meminfo") == 0) {
        return bi_meminfo(command);
    } else if (strcmp(command[0], "prompt") == 0) {
        return bi_prompt(command);
    } else if (strcmp(command[0], "exit") == 0) {
        return bi_exit();
    } else {
        return startProcess(command);
    }
}

int startProcess(char** command) {
    pid_t pid; // wpid;
    int status = 0;
    pid = fork();
    if (pid == 0) { // child
        execvp(command[0], command);
    } else {
        wait(&status); // may need waitpid
    }
    return 1;
}

int bi_manual() {
    printf("cwushell is a small shell written in C.\n");
    printf("Type commands with no arguments, -h, or --help to get usage instructions.\n");
    printf("BUILT-IN COMMANDS\n");
    printf("manual - print this page\n");
    printf("prompt - changes the command-line prompt message\n");
    printf("cpuinfo - print information about your CPU\n");
    printf("meminfo - print information about available memory\n");
    printf("exit - terminates the shell\n");
}

int bi_cpuinfo(char** args) {
    if (args[1] == 0
            || strcmp(args[1], "-h") == 0
            || strcmp(args[1], "--help") == 0) {
        printf("cpuinfo prints information about the machines CPU.\n");
        printf("OPTIONS\n");
        printf("[-h] or [--help] prints this message.\n");
        printf("[-c] prints the CPU's clockspeed.\n");
        printf("[-t] prints the CPU manufacturer.\n");
        printf("[-n] prints the number of cores available.\n");
    } else if (strcmp(args[1], "-c") == 0) {
        system("lscpu | grep MHz");
    } else if (strcmp(args[1], "-t") == 0) {
        system("lscpu | grep \"Model name\"");
    } else if (strcmp(args[1], "-n") == 0) {
        system("cat /proc/cpuinfo | awk '/^processor/{print $3}' | wc -l");
    }
    return 1;
}

int bi_meminfo(char** args) {
    if (args[1] == 0
            || strcmp(args[1], "-h") == 0
            || strcmp(args[1], "--help") == 0) {
        printf("meminfo prints information about the machines CPU.\n");
        printf("OPTIONS\n");
        printf("[-h] or [--help] prints this message.\n");
        printf("[-t] prints total RAM in MB.\n");
        printf("[-u] prints used RAM in MB.\n");
        printf("[-c] prints size of the CPU's L2 cache in bytes.\n");
    } else if (strcmp(args[1], "-t") == 0) {
        system("vmstat -s | grep \"total memory\"");
    } else if (strcmp(args[1], "-u") == 0) {
        system("vmstat -s | grep \"used memory\"");
    } else if (strcmp(args[1], "-c") == 0) {
        system("lscpu | grep L2");
    }
    return 1;
}

int bi_prompt(char** args) {
    if (args[1] == 0) {
        prompt = DEFAULT_PROMPT;
    } else if (strcmp(args[1], "-h") != 0
            && strcmp(args[1], "--help") != 0) {
        prompt = args[1];
    } else {
        printf("prompt changes the command-line prompt message to the argument.\n");
        printf("providing no arguments changes it to the default, cwushell.\n");
        printf("OPTIONS\n");
        printf("[-h], [--help] prints this message.\n");
    }
    return 1;
}

int bi_exit() {
    return 0;
}
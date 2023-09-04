#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      
#include <unistd.h>      
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0
#define STD_INPUT 0
#define STD_OUTPUT 1
#define SIZE 500

typedef struct information{
    int work;
    char **command1;
    char **command2;
    char *filename;
    int cmd1_len;
}INFORMATION;

char **SplitInput(char *command);
int execute(char **command, INFORMATION info);
INFORMATION analyze(char **command);
int back = 0;

int main(){
    while(TRUE){
        INFORMATION info;
        char directory[SIZE];
        char *command;
        char **split;
        command = (char*) malloc(SIZE * sizeof(char));
        split = (char**) malloc(SIZE * SIZE * sizeof(char));
        int count = 0;
        getcwd(directory, SIZE);
        printf("~%s$ ", directory);
        fgets(command, SIZE, stdin);
        
        if(command[0] == '\n'){
            continue;
        }
        split = SplitInput(command);
        info = analyze(split);
        int i;
        
        if(execute(split, info) == -1){
            break;
        }
        while(*split){
            split++;
            count++;
        }   
        for(i=0;i<count;i++){
            free(split[i]);
            split[i] == NULL;
        }
        free(command);
    }
    return 0;
}

char **SplitInput(char *command)
{   
    
    char **result;
    int index = 0;
    result = (char**) malloc(SIZE * SIZE * sizeof(char));
    const char *token = "  \t\n";
    char *str;
    str = strtok(command, token);
    
    while (str) {
        result[index] = str;
        index += 1;
        str = strtok(NULL, token);		   
    }
    return result;
    
}

int execute(char **command, INFORMATION info){
    int status;
    pid_t pid;
    if(!strcmp(command[0], "exit")){
        return -1;
    }
    if(info.work == 1){
        back++;
    }
    else{
        back = 0;
    }
    pid = fork();
    if(pid == 0){
        if(info.work == 0){
            
            if(!strcmp(command[0], "cd")){
                if(command[1] == NULL){
                    command[1] = (char*) "/";
                }
                if(chdir(command[1])){
                    if(command[2]){
                        printf("bash: cd: too many arguments\n");
                    }
                    else{
                        printf("bash: cd: %s: No such file or directory\n", command[1]);
                    }

                }
                
            }
            else{
                if(execvp(command[0], command) == -1){
                    printf("%s: command not found\n", command[0]);
                    return 1;
                }
            }
            return 1;
        }
        else if(info.work == 2){
            int fd[2]; 
            pid_t p2;
            if(pipe(fd) < 0){
                return 1;
            }
            p2 = fork();
            if(p2 < 0){
                return 1;
            }
            else if(p2 == 0){
                close(fd[0]);
                close(STD_OUTPUT);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                if(execvp(info.command1[0], info.command1) < 0){
                    printf("%s: command not found\n", info.command1[0]);
                    return 1;
                }
            }
            else{
                waitpid(p2,&status,0);
                close(fd[1]);
                close(STD_INPUT);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
        
                if(execvp(info.command2[0], info.command2) < 0){
                    printf("%s: command not found\n", info.command2[0]);
                    return 1;
                }
            }
        }
        else if(info.work == 3){
            int file;
                file = creat(info.filename, 0777);
                dup2(file, STDOUT_FILENO);
                close(file);
                if(execvp(info.command1[0], info.command1) < 0){
                    printf("%s: command not found\n", info.command1[0]);
                    return 1;
                }
                return 1;
        }
        else if(info.work == 4){
            int file;
                file = open(info.filename, O_RDONLY);
                if(file == -1){
                    printf("bash: %s: No such file or directory\n",info.filename);
                    return 1;
                }
                dup2(file, STDIN_FILENO);
                close(file);
                if(execvp(info.command1[0], info.command1) < 0){
                    printf("%s: command not found\n", info.command1[0]);
                    return 1;
                }
                return 1;
        }
        else{
            if(execvp(command[0], command) < 0){
                printf("%s: command not found\n", command[0]);
                return 1;
            }
        }
    }  
    else
    {
        if (info.work == 1)
            printf("[%d]: %d\n", back, pid);
        else
            waitpid(pid,&status,0);
    }
    return 1;
}

INFORMATION analyze(char **command){
    INFORMATION info = {0, NULL, NULL, NULL, 0};
    int index = 0;
    while(command[index]){
        index += 1 ;
    }
    index -= 1;
    if(!strcmp(command[index], "&")){
        info.work = 1;
        command[index] = NULL;
    }
    int i;
    info.command2 = (char**) malloc(SIZE * SIZE * sizeof(char));
    for(i = 0; i < index; i++){
        if(!strcmp(command[i], "|")){
            info.work = 2;
            int j;
            for(j = i + 1; j <= index; j++){
                info.command2[j-i-1] = command[j];
            }
            info.command1 = (char**) malloc(SIZE * SIZE * sizeof(char));
            for(j = 0; j < i; j++){
                info.command1[j] = command[j];
            }
            return info;
        }
        if(!strcmp(command[i], ">")){
            info.work = 3;
            info.filename = command[i + 1];
            info.command1 = (char**) malloc(SIZE * SIZE * sizeof(char));
            int j;
            for(j = 0; j < i; j++){
                info.command1[j] = command[j];
            }
            return info;
        }
        if(!strcmp(command[i], "<")){
            info.work = 4;
            info.filename = command[i + 1];
            info.command1 = (char**) malloc(SIZE * SIZE * sizeof(char));
            int j;
            for(j = 0; j < i; j++){
                info.command1[j] = command[j];
            }
            return info;
        }

    }
    return info;
}

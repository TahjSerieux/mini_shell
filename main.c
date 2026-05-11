#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
// 
#include <ctype.h>
#include <stdbool.h>
// fork() and pid_t 
#include <unistd.h>
#include <sys/types.h>
// waitpid()
#include <sys/wait.h>
// String comparator
#include <fcntl.h>
// Signals
#include <signal.h> 
#define RUNNING 0
#define STOPPED 1
#define DONE    2
#define MAX_JOBS 64
#define EMPTY_JOB_ID -1
typedef struct{
    int id;
    pid_t pgid;
    char* cmd;
    int state;
} Job;
Job jobTable[MAX_JOBS];

int jobCount = 0;


int addJob(pid_t pgid, char* cmd, int state){
    for(int i=0;i<MAX_JOBS;i++){
        if(jobTable[i].id == EMPTY_JOB_ID){
            jobTable[i].id = i+1;
            jobTable[i].pgid = pgid;
            jobTable[i].cmd = cmd;
            jobTable[i].state = state;
            jobCount++;
            return(jobTable[i].id);

        }
    }
    fprintf(stderr, "job table full\n");
    return -1;
}
void removeJob(int id){
    for(int i=0;i<MAX_JOBS;i++){
        if(jobTable[i].id == id){
            
            jobTable[i].id = -1;
            jobTable[i].pgid=-1;
            free(jobTable[i].cmd);
            jobTable[i].cmd=NULL;
            jobTable[i].state = DONE;
            jobCount--;
            return;
        }
    }
}

Job* getJobByPgid(pid_t pgid){
    for(int i = 0; i < MAX_JOBS; i++){
        if(jobTable[i].pgid == pgid){
            return &jobTable[i];
        }
    }
    return NULL;
}
void printJobs(){
    for(int i=0;i<MAX_JOBS;i++){
        if(jobTable[i].id != EMPTY_JOB_ID){
            char* stateStr = jobTable[i].state == RUNNING ? "Running": jobTable[i].state == STOPPED ? "Stopped": "Done";
            printf("[%d] %s %s\n",jobTable[i].id, stateStr, jobTable[i].cmd);
        }
    }
}

void sigchldHandler(int sig){
    int status;
    pid_t pid;


    while(pid = waitpid(-1,&status ,WNOHANG |WUNTRACED ) > 0){

        pid_t pgid = getpgid(pid);
        Job* job = getJobByPgid(pgid);
        if (job == NULL){
            continue;
        }
        if(WIFSTOPPED(status)){
            job->state = STOPPED;
            printf("\n[%d] Stopped %s\n", job->id, job->cmd);
        } else if(WIFEXITED(status) || WIFSIGNALED(status)){
            printf("\n[%d] Done %s\n", job->id, job->cmd);
            removeJob(job->id);
        }

    }


}

char* copyCharPointer(char* oldString, char* newString){
    // Assumes the oldPointer ends with NULL and newPointer has allocated enough space for all the characters + NULL.
    char* oldHead = oldString;
    char* newHead =  newString;

    char* oldCurr = oldHead;
    char* newCurr =  newHead;
    while(*oldCurr){
        printf("oldCurr: %c\n",*oldCurr);
        *newCurr = *oldCurr;
        printf("newCurr: %c\n",*newCurr);
        newCurr++;
        oldCurr++;
    }

    *newCurr = '\0';
    char* currCopy =  oldString;
    return(newString);
}

char** copyCharVector(char** oldArray, char**newArray){
    char** oldCurr =  oldArray;
    char** newCurr =  newArray;

    while(*oldCurr){
        *newCurr =  *oldCurr;

        oldCurr ++;
        newCurr ++;
    }
    *newCurr = NULL;

    return(newArray);
}

int countWords(char* c){
    bool inWord = false;
    int wordCount = 0;
    while(true){
        // While a valid character and it is not a white space
        if(*c && !isspace(*c)){
            inWord= true;
        }
        else if(inWord && (isspace(*c) || *c == '\0')){
            inWord =  false;
            wordCount++;
        }
        if(*c == '\0'){
            break;
        }
        c++;
    }
    // printf("[countWords] Finished word count is: %d\n", wordCount);
    return(wordCount);

}

char** tokenizer(char* line, int* count){
    int wordCount = countWords(line);
    char ** arr = malloc((wordCount + 1) * (sizeof(char *)));
    char** currentWord = arr;


    const char* start = line;
    while(*line){
        // skip white spaces
        while(*line && isspace(*line)){
            line++;
        }
        
        if(!*line){
            break;
        }
        
        start = line;
        while(*line && !isspace(*line)){
            line++;
        }
        int len =  line - start;
        // Hello World
        char* word = malloc(len+1);
        strncpy(word, start, len);
        word[len] = '\0';

        *currentWord =  word;
        currentWord++;
        (*count)++;

    }
    *currentWord = NULL;
    
    return(arr);
}

void handlePipe(char** tokens, int pipeCount){
    // printf("==== pipe detected, pipeCount=%d ====\n", pipeCount);
    
    char*** commands = malloc(sizeof(char**) * (pipeCount+2));
    char*** currCmd = commands;

    char** curr = tokens;
    char** start = curr;

    while(*curr){
        if(strcmp(*curr,"|") == 0){
            int length = curr - start;
            char** subCmd = malloc(sizeof(char*) * (length+1));
            char** currSubCmd = subCmd;
            while(start != curr){
                *currSubCmd = *start;
                start++;
                currSubCmd++;
            }
            *currSubCmd = NULL;   
            *currCmd = subCmd; 
            curr++;
            start++;
            currCmd++;
        }
        curr++;
    }

    // save last command
    int length = curr - start;
    char** subCmd = malloc(sizeof(char*) * (length+1));
    char** currSubCmd = subCmd;
    while(start != curr){
        *currSubCmd = *start;   // ✅ copy into subCmd not into currCmd
        start++;
        currSubCmd++;
    }
    *currSubCmd = NULL;
    *currCmd = subCmd;
    currCmd++;
    *currCmd = NULL;

    // printf("==== commands parsed ====\n");
    for(int i = 0; i < pipeCount+1; i++){
        // printf("commands[%d]: ", i);
        char** tok = commands[i];
        while(*tok){
            // printf("%s ", *tok);
            tok++;
        }
        // printf("\n");
    }

    int pipes[pipeCount][2];
    currCmd = commands;

    for(int i = 0; i < pipeCount; i++){
        pipe(pipes[i]);
        // printf("created pipe %d: read=%d write=%d\n", i, pipes[i][0], pipes[i][1]);
    }
    pid_t pgid = 0; 
    for(int i = 0; i < pipeCount+1; i++){
        pid_t p = fork();
        // printf("forked child %d, pid=%d\n", i, p);
        if(p == 0){
            if(pgid == 0){
                pgid = getpid();
            }
            setpgid(0, pgid);
            if(i == 0){
                // printf("---------------------------------\n");
                // printf("child %d executing: %s\n", i, commands[i][0]);
                dup2(pipes[i][1], STDOUT_FILENO);
            }else if(i == pipeCount){
                dup2(pipes[i-1][0], STDIN_FILENO);
            }else{
                dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            // close all pipe ends in child
            for(int j = 0; j < pipeCount; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            // printf("child %d executing: %s\n", i, commands[i][0]);
            signal(SIGINT, SIG_DFL);   // child responds to Ctrl+C normally
            signal(SIGTSTP, SIG_DFL);  // child responds to Ctrl+Z normally
            signal(SIGTTOU, SIG_DFL);
            
            execvp(commands[i][0], commands[i]);
            perror("execvp failed");
            exit(1);
        }else{
            if(pgid == 0){
                pgid = p;
            }
            setpgid(p, pgid);
        }
    }
    tcsetpgrp(STDIN_FILENO, pgid);
    for(int i = 0; i < pipeCount; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL); 
    int jobId = addJob(pgid,commands[0][0],RUNNING);
    sigprocmask(SIG_UNBLOCK, &mask, NULL); 
    for(int i = 0; i < pipeCount+1; i++){
        wait(NULL);
    }
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL); 
    removeJob(jobId);
    sigprocmask(SIG_UNBLOCK, &mask, NULL); 
    tcsetpgrp(STDIN_FILENO, getpgrp());
}


char** execute(char** tokens, int count){
    char** curr = tokens;
    char* redirect = NULL;
    char* file;



    // Detect for redirect in command
    while(*curr){
        char* token =  *curr;
        // printf("token: %s\n",token);
        if(strcmp(token,">") == 0 || strcmp(token,">>") == 0 || strcmp(token,"<") == 0){
            
            redirect =  token;
            int length = curr -  tokens;
            char ** newTokens =  malloc(sizeof(char *) * (length +1));
            for(int i = 0;i< length;i++){
                newTokens[i] = tokens[i];
            }
            newTokens[length]  = NULL;

            // printf("New Tokens\n");
            // for(int i =0;i< length;i++){
            //     printf("%s\n", newTokens[i]);
            // }
            // printf("End Of New Tokens\n");
            curr++;
            token =  *curr;
            file = token;
            free(tokens);
            tokens = NULL;
            tokens = newTokens;
            break;
        }
        // printf("%s\n",*curr);
        curr++;
    }
    
    bool pipeExists = false;
    int pipeCount = 0;
    // Detect pipe and count how many there are
    curr = tokens;
    while(*curr){
        if(strcmp(*curr,"|") == 0){
            pipeCount++;
            if(!pipeExists){
                pipeExists = true;
            }
        }
        curr++;
    }

    if(pipeExists && pipeCount>0){
        handlePipe(tokens,pipeCount);
        return(tokens);
    }
    pid_t p = fork();

    if(p == 0){
        if(redirect && file){
            if(strcmp(redirect,"<") == 0){
                int fd =  open(file,O_RDONLY,0644);
                if(fd < 0){
                    perror("open");
                    exit(1);
                }
                dup2(fd,0);
                close(fd);
            }else if(strcmp(redirect, ">") == 0){
                int fd = open(file, O_CREAT| O_WRONLY,0644);
                if(fd < 0){
                    perror("open");
                    exit(1);
                }
                dup2(fd,1);
                close(fd);

            }else if(strcmp(redirect,">>") == 0){
                int fd = open(file,O_CREAT| O_WRONLY| O_APPEND,0644);
                if(fd < 0){
                    perror("open");
                    exit(1);
                }
                dup2(fd,1);
                close(fd);
            }
        }
        setpgid(0, 0); 
        signal(SIGINT, SIG_DFL);   // child responds to Ctrl+C normally
        signal(SIGTSTP, SIG_DFL);  // child responds to Ctrl+Z normally
        signal(SIGTTOU, SIG_DFL);
        execvp(tokens[0], tokens);

        fprintf(stderr, "mini_shell: %s: command not found\n", tokens[0]);
        exit(127);
        // perror("[execute()] execvp failed");
        // exit(1);
    }else if(p > 0){
        setpgid(p, p);
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask, NULL); 
        int jobId = addJob(p, tokens[0], RUNNING);  
        sigprocmask(SIG_UNBLOCK, &mask, NULL); 
        tcsetpgrp(STDIN_FILENO, p); 
        waitpid(p, NULL, 0);

        tcsetpgrp(STDIN_FILENO, getpgrp());


        sigemptyset(&mask);
        sigaddset(&mask, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask, NULL); 
        sigprocmask(SIG_UNBLOCK, &mask, NULL); 
    }else{
        printf("fork failed");
    }
    // printf("%s\n",redirect );
    // printf("%s\n",file );
    return(tokens);
}

char** commandManager(char** tokens,int count){
    char* command = tokens[0];
    
    if(strcmp("cd",command)==0){
        if(chdir(tokens[1]) != 0){
            perror("cd failed");
        }
    }else if(strcmp("jobs",tokens[0]) == 0){
        printf("jobs command not implemented yet");
    }else if(strcmp("fg",tokens[0]) == 0){
        printf("fg command not not implemented yet");
    }else if(strcmp("bg",tokens[0]) == 0){
        printf("bg command not implemented yet");
    }else{
        tokens = execute(tokens,count);
    }
    return(tokens);
}

void freeTokens(char** tokens){
    char** curr = tokens;
    curr = tokens;
    while(*curr){
        free(*curr);
        curr++;
    }
    free(tokens);
}

void cleanup(char** tokens, int count, char* line){
    free(line);
    char** curr =  tokens;
    while(*curr){
        free(*curr);
        curr++;
    }
    free(tokens);
    clear_history();
}

int main(){
    signal(SIGCHLD, sigchldHandler);
    for(int i=0;i<MAX_JOBS;i++){
        jobTable[i].id =  EMPTY_JOB_ID;
        jobTable[i].pgid = -1;
        jobTable[i].cmd = NULL;
        jobTable[i].state = DONE;

    }
    // char* str = "Hello";
    // char* copy =  malloc(6 );

    // copy =  copyCharPointer(str,copy);
    // // free(str);
    // char* currCopy =  copy;
    // while(*currCopy){
    //     printf("%c",*currCopy);
    //     currCopy++;
    // }
    // printf("\n");

    printf("Mini Shell\n");

    using_history();
    HISTORY_STATE* state = history_get_history_state();

    char * line  = NULL;
    int countVal = 0;
    int* count = &countVal;
    signal(SIGINT, SIG_IGN);   // shell ignores Ctrl+C
    signal(SIGTSTP, SIG_IGN);  // shell ignores Ctrl+Z
    signal(SIGTTOU, SIG_IGN);
    while((line = readline(">> ")) != NULL){
        if(line[0] == '\0'){
            continue;
        }
        add_history(line);
        char** tokens = tokenizer(line,count);
        if(strcmp(tokens[0], "exit") == 0){
            cleanup(tokens, countVal, line);
            exit(0);
        }
        tokens = commandManager(tokens,*count);

        freeTokens(tokens);
        free(line);
        *count = 0;
        line = NULL;
        printJobs();
    }
    state = history_get_history_state();
    printf("The number of entries is: %d\n",state->length);
    return(0);
}
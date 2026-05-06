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
int countWords(char* c){
    // printf("[countWords] Start\n");
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
    // printf("Start tokenization\n");
    int wordCount = countWords(line);
    char ** arr = malloc(wordCount * (sizeof(char *)+1));
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

void execute(char** tokens){

    pid_t p = fork();

    if(p == 0){
        execvp(tokens[0], tokens);

        fprintf(stderr, "mini_shell: %s: command not found\n", tokens[0]);
        exit(127);
        // perror("[execute()] execvp failed");
        // exit(1);
    }else if(p > 0){
        waitpid(p, NULL, 0);
    }else{
        printf("fork failed");
    }
}

void commandManager(char** tokens){
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
        execute(tokens);
    }



}

void freeTokens(char** tokens, int count){
    for(int i=0;i<count;i++){
        free(tokens[i]);
    }
    free(tokens);
}
void cleanup(char** tokens, int count, char* line){
    free(line);
    for(int i=0;i< count;i++){
        free(tokens[i]);
    }
    free(tokens);
    clear_history();
}

int main(){

    printf("Mini Shell\n");
    using_history();
    HISTORY_STATE* state = history_get_history_state();

    char * line  = NULL;
    int countVal = 0;
    int* count = &countVal;
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
        commandManager(tokens);

        freeTokens(tokens,*count);
        free(line);
        *count = 0;
        line = NULL;
    }
    state = history_get_history_state();
    printf("The number of entries is: %d\n",state->length);

    return(0);
}
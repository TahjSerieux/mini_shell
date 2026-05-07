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

char** execute(char** tokens, int count){
    char** curr = tokens;
    char* redirect = NULL;
    char* file;

    // for(int i=0;i< count;i++){
    //     printf("token: %s\n", tokens[i]);
    // }

    
    while(*curr){

        char* token =  *curr;


        // printf("token: %s\n",token);
        if(strcmp(token,">") == 0 || strcmp(token,">>")== 0 || strcmp(token,"<") == 0){
            
            redirect =  token;
            int length = curr -  tokens;
            // printf("The length is: %d\n",length);
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
    // if(redirect && file){
    //     printf("redirect: %s\n",redirect);
    //     printf("file: %s\n", file);
    // }
    curr = tokens;
    // printf("listing tokens: \n");
    // while(*curr){
    //     printf("%s\n",*curr);
    //     curr++;
    // }
    // printf("End of listing tokens: \n");

    
    pid_t p = fork();

    if(p == 0){
        // int fd = open(file,O_RDWR);
        //     dup2(fd,1);
        //     printf("fd 1 now refers to %s\n", file);
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
                // O_APPEND
                // o
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
        tokens = commandManager(tokens,*count);

        freeTokens(tokens);
        free(line);
        *count = 0;
        line = NULL;
    }
    state = history_get_history_state();
    printf("The number of entries is: %d\n",state->length);

    return(0);
}
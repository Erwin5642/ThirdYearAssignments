#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#define MAX_CL_LEN 100
#define MAX_ARGS 10
#define MAX_P_NAME_LEN 16

typedef struct ProcessNode
{
    pid_t pid;
    char name[MAX_P_NAME_LEN];
    unsigned childrenNum;

    struct ProcessNode **children;
} ProcessNode;

ProcessNode *createNode(pid_t pid, char name[MAX_P_NAME_LEN])
{
    ProcessNode *new = malloc(sizeof(ProcessNode));
    strncpy(new->name, name, MAX_P_NAME_LEN);
    new->pid = pid;
    new->childrenNum = 0;
    new->children = NULL;
    return new;
}

void freeProcessTree(ProcessNode **root)
{
    if (root && (*root))
    {
        for (unsigned i = 0; i < (*root)->childrenNum; i++)
        {
            freeProcessTree(&(*root)->children[i]);
        }
        if(((*root)->childrenNum > 0)){
            free((*root)->children);
        }
        free(*root);
        *root = NULL;
    }
}

int tryInsertProcessTree(ProcessNode **root, pid_t ppid, pid_t pid, char name[MAX_P_NAME_LEN])
{
    if (!root) return 0;

    if (*root) {
        if ((*root)->pid == ppid) {
            (*root)->childrenNum++;
            (*root)->children = realloc((*root)->children, (*root)->childrenNum * sizeof(ProcessNode *));
            (*root)->children[(*root)->childrenNum - 1] = createNode(pid, name);
            return 1;
        } else {
            for (unsigned i = 0; i < (*root)->childrenNum; i++) {
                if (tryInsertProcessTree(&((*root)->children[i]), ppid, pid, name)) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

void showProcessTree(ProcessNode *root, int offSet){
    if(root == NULL){
        return;
    }
    
    printf("%d %s ", root->pid, root->name);
    if(root->childrenNum == 0){
        printf("\n");
    }
    offSet += strlen(root->name) + (floor(log10(root->pid)) + 1) + 2;
    for(unsigned i = 0; i < root->childrenNum; i++){
        if(i != 0){
            for(int j = 0; j < offSet; j++){
                printf(" ");
            }
        }
        if(root->childrenNum == 1){
            printf("── ");
        }
        else if(i == root->childrenNum - 1){
            printf("└─ ");
        }
        else if(root->childrenNum > 1){
            if(i == 0){
                printf("┬─ ");
            }
            else{
                printf("├─ ");
            }
        }
        showProcessTree(root->children[i], offSet + 3);
    }
}

void treePid(ProcessNode **root, pid_t rootpid)
{
    DIR *currentDir = opendir("/proc");
    struct dirent *processDirent;
    char pathname[FILENAME_MAX];
    while ((processDirent = readdir(currentDir)) != NULL)
    {
        if (isdigit(processDirent->d_name[0]))
        {
            snprintf(pathname, FILENAME_MAX, "/proc/%s/stat", processDirent->d_name);
            FILE *file = fopen(pathname, "r");
            if (file != NULL)
            {
                pid_t pid;
                char name[MAX_P_NAME_LEN];
                char status;
                pid_t ppid;
                fscanf(file, "%d %s %c %d", &pid, name, &status, &ppid);
               if(pid == rootpid){
                    *root = createNode(pid, name);
                }
                if(rootpid < pid){
                    tryInsertProcessTree(root, ppid, pid, name);
                }
                fclose(file);
            }
        }
    }
    closedir(currentDir);
}

int main()
{
    char commandLine[MAX_CL_LEN];
    char *tokens[MAX_ARGS];
    unsigned argsNumber = 0;

    while (1)
    {
        char host[128];
        char cwd[FILENAME_MAX];
        gethostname(host, 128);
        printf("\033[1;33m%s@%s:\033[1;35m%s\033[0m$ ", getpwuid(getuid())->pw_name, host, getcwd(cwd, FILENAME_MAX));

        fgets(commandLine, MAX_CL_LEN - 1, stdin);

        tokens[0] = strtok(commandLine, " \n");
        while (tokens[argsNumber] != NULL && argsNumber < MAX_ARGS)
        {
            tokens[++argsNumber] = strtok(NULL, " \n");
        }
        if (tokens[0] != NULL && (strcmp(tokens[0], "&") != 0))
        {
            int flag = 1;

            if (strcmp(tokens[argsNumber - 1], "&") == 0)
            {
                tokens[argsNumber - 1] = NULL;
                argsNumber--;
                flag = 0;
            }

            if (strcmp(tokens[0], "exit") == 0)
            {
                return 0;
            }
            if (strcmp(tokens[0], "cd") == 0)
            {
                if(tokens[1] == NULL){
                    chdir("/");
                }
                else if(argsNumber == 2){
                    if(chdir(tokens[1]) == -1){
                        perror("terminal: cd");
                    }
                }
                else{
                    printf("terminal: cd: erro: argumentos demais");
                }                
            }
            else if (strcmp(tokens[0], "tree") == 0)
            {
                ProcessNode *root = NULL;
                if(tokens[1] == NULL){
                    treePid(&root, 1);
                    showProcessTree(root, 0);
                    freeProcessTree(&root);
                }
                else if(argsNumber == 2 && isdigit(tokens[1][0])){
                    treePid(&root, atoi(tokens[1]));
                    if(root != NULL){
                        showProcessTree(root, 0);
                        freeProcessTree(&root);
                    }
                    else{
                        printf("tree: %s: processo inexistente\n", tokens[1]);
                    }
                }
                else{
                    printf("tree: erro: \nUso correto:\n\ttree [pid]\n");
                }
            }
            else if (fork() == 0)
            {
                execvp(tokens[0], tokens);
                printf("terminal: %s: comando não encontrado\n", tokens[0]);
                return -1;
            }
            if (flag)
            {
                wait(NULL);
            }
        }
        memset(commandLine, 0, sizeof commandLine);
        memset(tokens, 0, sizeof tokens);
        argsNumber = 0;
    }
}

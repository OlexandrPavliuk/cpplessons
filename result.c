#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_commands_count(char* command_line)
{
    int count = 0;
    int i = 0;
    
    while (command_line[i])
    {
        if (command_line[i] == '|')
        {
            ++count;
        }
        
        ++i;
    }
    
    return count + 1;
}

void trim(char * s) 
{
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}

void split_commands(char *command_line, char** commands, const char* sep)
{
    int i = 0;
    char *tok = strtok(command_line, sep);
    
    while (tok)
    {
        trim(tok);        
        commands[i] = tok;        
        tok = strtok(NULL, sep);
        ++i;
    }                    
}

int get_command_parts_count(char* command)
{
    int count = 0;
    int i = 0;
    
    while (command[i])
    {
        if (command[i] == ' ')
        {
            ++count;
        }
        
        ++i;
    }
    
    return count + 1;
}

void connect(char** commands, int count) 
{
    int status;
    int i = 0;
    int command = 0;
    pid_t pid;
    int numPipes = count - 1;
    int commandPartsCount = 0;

    int pipefds[2 * numPipes];

    for (i = 0; i < (numPipes); i++)
    {
        pipe(pipefds + i*2);        
    }

    int j = 0;
    for (command = 0; command < count; ++command) 
    {
        pid = fork();
        if (pid == 0) 
        {           
            if (command != count - 1)
            {
                dup2(pipefds[j + 1], 1);                
            }
            
            if (command != 0 )
            {
                dup2(pipefds[j-2], 0);                
            }


            for(i = 0; i < 2*numPipes; i++)
            {
                close(pipefds[i]);
            }
            
            commandPartsCount = get_command_parts_count(commands[command]);
            char** parts = malloc(sizeof(char*) * (commandPartsCount + 1));
            split_commands(commands[command], parts, " ");
            parts[commandPartsCount] = NULL;
            
            execvp(parts[0], parts);        
            
            free(parts);
        } 
        
        j+=2;
    }
    /**Parent closes the pipes and wait for children*/

    for(i = 0; i < 2 * numPipes; i++)
    {
        close(pipefds[i]);
    }

    for(i = 0; i < numPipes + 1; i++)
        wait(&status);
}

int main(int argc, char **argv) {         	
	size_t len = 1024;
	char *command_line = malloc(len);				
    int commands_count = 0;
    char** commands = 0;
    
	getline(&command_line, &len, stdin);    
    
    commands_count = get_commands_count(command_line);    
    
    commands = malloc(sizeof(char*) * commands_count);
    
    split_commands(command_line, commands, "|");

	connect(commands, commands_count);
    
    free(commands);

	return 0;
}


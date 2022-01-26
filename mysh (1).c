#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFSIZE 1024
#define MAXARGS 100
char current_dir[BUFFSIZE];


typedef struct stack_node
{
    char * command;
    struct stack_node * next;
}stack_node;

typedef struct Stack
{
    stack_node * head;
}Stack;

// ALLEC PEREIRA
int check_if_buffer_empty(char * buffer){

    if (buffer[0] == '\n')
        return 1;

    buffer[strlen(buffer) - 1] = '\0';

    int cnt = 0;
    int len = strlen(buffer);

    for (int i = 0; i < len; i++)
    {
        if (isspace(buffer[i]))
            cnt++;
    }
    return cnt == len;
}

// ALLEC PEREIRA
int read_input(char * buffer){
  fgets(buffer,BUFFSIZE,stdin);
  return check_if_buffer_empty(buffer);
}

char ** parse_input(char * buffer){
  int i = 0;
  char ** input = malloc(MAXARGS * sizeof(char*));
  char * token;

  char * temp = strdup(buffer);

  while ((token = strsep(&temp," ")) != NULL)
  {
      if (strcmp(token,"") == 0)
          continue;

      input[i] = strdup(token);
      i++;
  }

  return input;
}

// ALLEC PEREIRA
int move_to_dir(char * target_directory){

    if (target_directory == NULL)
        return 1;

    char static_working_directory[BUFFSIZE];
    getcwd(static_working_directory,BUFFSIZE);

    char buff[BUFFSIZE];
    buff[0] = '\0';

    struct stat stats;

    if (strcmp(target_directory,static_working_directory) == 0 || target_directory[0] == '/')
    {
        if (stat(target_directory,&stats) == 0)
        {
            strcpy(current_dir,target_directory);
            return 0;
        }
        return 1;
    }

    if (current_dir[0] == '/')
    {
        if (current_dir[1] == '\0')
        {
            strcpy(buff,current_dir);
            strcat(buff,target_directory);
        }

        else
        {
            strcpy(buff,current_dir);
            strcat(buff,"/");
            strcat(buff,target_directory);
        }

        if (stat(buff,&stats) == 0)
        {
            strcpy(current_dir,buff);
            return 0;
        }
        return 1;
    }

    strcpy(buff,current_dir);
    strcat(buff,"/");
    strcat(buff,target_directory);

    if (stat(buff,&stats) == 0)
    {
        strcpy(current_dir,buff);
        return 0;
    }
    return 1;
}

// ALLEC PEREIRA
void print_command_history(Stack * commands)
{
    stack_node * temp = commands->head;
    int size= 0;

    if (temp == NULL)
        printf("command history is empty");

    while (temp != NULL)
    {
        printf("%d: %s\n",size,temp->command);
        temp = temp->next;
        size++;
    }
}

// ALLEC PEREIRA
char * find_command(Stack * commands, int target_command)
{
    stack_node * temp = commands->head;

    while (temp != NULL)
    {
        if (target_command == 0)
            return temp->command;

        temp = temp->next;
        target_command--;
    }

    return NULL;
}

// ALLEC PEREIRA
int num_args(char ** args){
    int cnt = 0;
    int i = 2;

    while (args[i++] != NULL)
        cnt++;

    return cnt;
}

// ALLEC PEREIRA
char ** get_arguments(char ** argv, int len){

    char ** args = malloc(sizeof(char *) * (len));


    args[0] = malloc(sizeof(char) * (strlen(argv[1]) + 1));

    strcpy(args[0],argv[1]);

    args[len - 1] = NULL;


    for (int i = 1; i < len - 1; i++)
    {
        args[i] = malloc(sizeof(char) * (strlen(argv[i]) + 1));
        strcpy(args[i],argv[i + 1]);
    }

    return args;
}

// ALLEC PEREIRA
void background_process(char * program, char ** argv, int len){
  pid_t child_id, end_id;
  char path[BUFFSIZE], path2[BUFFSIZE];
  int index = 2;
  int flag = 0;

  path2[0] = '\0';
  path[0] = '\0';

  char ** args = get_arguments(argv,len + 2);

  if (program[0] != '/')
  {
      strcpy(path2,current_dir);
      strcat(path2,"/");
      strcat(path2,program);
  }

  else
      strcpy(path2,program);

  child_id = fork();

  if (child_id == -1)
      printf("There was an error with the fork\n");
  else if (child_id == 0)
  {
      setpgid(0,0);
      if (execv(path2,args) < 0)
      {
          flag = 1;
          exit(0);
      }
  }
  else
  {
      int pid = waitpid(child_id,NULL,WNOHANG);
      //printf("%d",pid);
      if (flag)
          printf("program not found\n");

      printf("PID: %d\n",child_id);

  }

}

// ALLEC PEREIRA
void start_program(char * program, char ** argv, int len){
  pid_t child_id, end_id;
  char path[BUFFSIZE], path2[BUFFSIZE];
  int index = 2;

  path2[0] = '\0';
  path[0] = '\0';

  char ** args = get_arguments(argv,len + 2);

  if (program[0] != '/')
  {
      strcpy(path2,current_dir);
      strcat(path2,"/");
      strcat(path2,program);
  }

  else
      strcpy(path2,program);

  child_id = fork();

  if (child_id == -1)
      printf("There was an error with the fork\n");
  else if (child_id == 0)
  {
      if (execv(path2,args) < 0)
      {
          printf("program does not exist\n");
          exit(0);
      }
  }
  else
  {
      end_id = waitpid(child_id,NULL,0);

      if (end_id == -1)
          perror("process cannot be executed\n");
  }

}

// ALLEN PEREIRA
int perform_command(char ** input, Stack * commands, char * buffer){


      if (strcmp(input[0],"movetodir") == 0)
      {

          if (input[2] != NULL || move_to_dir(input[1]))
              printf("No such file or directory\n");
      }

      else if (strcmp(input[0],"whereami") == 0)
          printf("%s\n",current_dir);

      else if (strcmp(input[0],"history") == 0)
      {
          if (input[1] == NULL)
              print_command_history(commands);

          else if (input[1] != NULL && strcmp(input[1],"-c") == 0)
              commands->head = NULL;
          else
              printf("command not found\n");
      }

      else if (strcmp(input[0],"start") == 0)
      {
          if (input[1] == NULL)
              printf("command not found\n");

          else
          {
              int len = num_args(input);
              start_program(input[1],input,len);
          }
      }

      else if (strcmp(input[0],"dalek") == 0)
      {
          if (input[1] == NULL)
              printf("command not found\n");

          else
          {
              if (kill(atoi(input[1]),SIGKILL) == 0)
                  printf("kill() successful\n");
              else
                  printf("kill() unsuccessful\n");

          }
      }

      else if (strcmp(input[0],"background") == 0)
      {
          if (input[1] == NULL)
              printf("command is not found\n");

          else
          {
              int len = num_args(input);
              background_process(input[1],input,len);
          }
      }

      else if (strcmp(input[0],"replay") == 0)
      {
          if (input[1] == NULL)
              printf("command not found\n");
          else
          {
              char * target = find_command(commands,atoi(input[1]) + 1);

              //printf("\n\n\n\n\target:%s",target);
              if (target == NULL)
                  printf("command number exceeds history\n");

              else
              {
                  char ** temp = input;
                  //printf("----target:%s\n\n",target);
                  char ** new_input = parse_input(target);
                  if (strcmp(input[0],new_input[0]) == 0 && strcmp(input[1],new_input[1]) == 0)
                      return 1;

                  free(temp);
                  return perform_command(new_input,commands,target);
              }

          }
      }

      else if (strcmp(input[0],"byebye") == 0)
          return 0;
      else
          printf("command not found!!\n");


      return 1;
}

// ALLEN PEREIRA
stack_node * create_stack_node(char * buffer){
    stack_node * node = malloc(sizeof(stack_node));
    node->command = malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(node->command,buffer);
    node->next = NULL;
    return node;
}

// ALLEN PEREIRA
void store_command(Stack * s,char * buffer){

    if (s->head == NULL)
    {
        s->head = create_stack_node(buffer);
        return;
    }

    stack_node * node = create_stack_node(buffer);
    node->next = s->head;
    s->head = node;
}

// ALLEN PEREIRA
stack_node * reverse_stack(stack_node * head){

    if (head == NULL)
        return NULL;

    if (head->next == NULL)
        return head;

    stack_node * new_head = reverse_stack(head->next);
    head->next->next = head;
    head->next = NULL;
    return new_head;
}

// ALLEN PEREIRA
void write_to_file(Stack * s, FILE *fp){
    char buffer[BUFFSIZE];

    s->head= reverse_stack(s->head);
    stack_node * temp = s->head;

    while (temp != NULL)
    {
        strcpy(buffer,temp->command);
        strcat(buffer,"\n");
        fputs(buffer,fp);
        temp = temp->next;
    }

    s->head = reverse_stack(s->head);
}

// ALLEN PEREIRA
void load_commands(Stack * commands,FILE * fp){

    if (fp == NULL)
        return;

    char buffer[BUFFSIZE];

    while (fgets(buffer,BUFFSIZE,fp) != NULL)
    {
        buffer[strlen(buffer) - 1] = '\0';

        if (strlen(buffer) == 0)
            continue;

        store_command(commands,buffer);
    }
}

// ALLEC PEREIRA
int main (){
  char buffer[BUFFSIZE];
  char **input;
  int flag = 1, isreplace = 0;
  getcwd(current_dir,BUFFSIZE);
  Stack commands;
  commands.head = NULL;
  FILE *fp = fopen("commands.txt","r");


  if (fp != NULL)
    load_commands(&commands,fp);


  if (fp != NULL)
    fclose(fp);

  while (flag){

    printf("# ");


    if (read_input(buffer))
        continue;

    store_command(&commands,buffer);

    input = parse_input(buffer);

    flag = perform_command(input,&commands,buffer);
  }


  FILE * fp2 = fopen("commands.txt","w");


  write_to_file(&commands,fp2);
  fclose(fp2);

}

/*
* File : grshv1.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
  size_t bufferSize = 0;
  char *line = NULL; //what was inputted into the command line
  char *command = NULL; //the command line input after being trimmed of excess space
  char* token;
  char **paths;
  char *pathname1 = NULL;
  char *pathname2 = NULL;

  if (argc == 1) {
      printf("grsh> ");

      //the shell interactive loop
      while (getline(&line, &bufferSize, stdin) > 0) {
          //allocate enough memory to store the trimmed command
          command = malloc((strlen(line)) + 1);
          //this goes outside the while loop to prevent a space before the trimmed command
          token = strtok_r(line, " \n\t", &line);

          if (token == NULL) {
              free(command);
              printf("grsh> ");
              continue;
          }

          strcat(command, token);
          int count = 1;

          //end result of this while loop is a command fully trimmed of excess spaces
          while ((token = strtok_r(line, " \n\t", &line))) {
              //all excess space is trimmed, but we still want one space between arguments
              strcat(command, " ");
              strcat(command, token);
              count++;
          }//end while

          //printf("%s\n", command);
          //printf("%d\n", count);
/*
          if ((strcmp(command, "exit")) == 0) {
              exit(0);
          }//end if
*/
          int i;
          char* ctoken;
          char *myargs[count+1];
          char *commandDup = strdup(command);
          for (i = 0; i < count; i++) {
            ctoken = strtok_r(commandDup, " ", &commandDup);
            myargs[i] = strdup(ctoken);
            //printf("%s ", myargs[i]);
          }
          //printf("\n");
          myargs[count] = NULL;

          //exit: built-in command
          if ((strcmp(command, "exit")) == 0) {
              exit(0);
          }//end if
          //cd: built-in command
          else if ((strcmp(myargs[0], "cd")) == 0) {
              if (count != 2) {
                  char error_message[30] = "An error has occurred";
                  write(STDERR_FILENO, error_message, strlen(error_message));
              }//end if
              else {
                  int cd = chdir(myargs[1]);
                  if (cd != 0) {
                      char error_message[30] = "An error has occurred";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                  }//end if
              }//end else
          }//end else if
          else if ((strcmp(myargs[0], "path")) == 0) {
              int pathCount = count - 1;
              paths = (char**)calloc(pathCount, sizeof(char *));
              //int j;
              for (i = 0; i < pathCount; i++){
                paths[i] = (char *)calloc(strlen(myargs[i+1]), sizeof(char));
                strcpy(paths[i], myargs[i+1]);
              }
              for (i = 0; i < pathCount; i++) {
                printf("%s\n", paths[i]);
              }
          }
          else {
              //-------------Code continue-----------------
/*            char *epath = getenv("PATH");
              char **path = malloc((strlen(epath)) + 1);
              char *ptoken;
              int count = 1;

              ptoken = strtok(epath, ":");
              while(ptoken != NULL) {
                  ptoken = strtok(NULL, ":");
                  printf("%s\n", ptoken);
                  count++;
              }
              printf("%d", count);

              while ((ptoken = strtok_r(epath, ":", &epath))) {
                  path[count] = malloc((strlen(epath)) + 1);
                  strcat(path[count], ptoken);
                  count++;
              }

              for (int i = ; i <
              printf("%s", *path);
*/
              //char *pathname1 = NULL;
              //char *pathname2 = NULL;

              pathname1 = malloc(5 + (strlen(myargs[0])));
              pathname2 = malloc(9 + (strlen(myargs[0])));

              strcat (pathname1, "/bin/");
              strcat (pathname1, myargs[0]);
              strcat (pathname2, "/usr/bin/");
              strcat (pathname2, myargs[0]);

              //printf("%s\n%s\n", pathname1, pathname2);

              if ((access(pathname1, X_OK)) == 0) {
                  //printf("\nFirst");
                  int rc = fork();
                  if (rc < 0) {
                      char error_message[30] = "An error has occurred";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                      free(command);
                      //free(pathname1);
                      //free(pathname2);
                      printf("\ngrsh> ");
                      continue;
                  }//end if
                  else if (rc == 0) {
                      printf("(pid:%d) Child OwO", (int) getpid());
                      execv(pathname1, myargs);
                      printf("Tried to print, didn't come out though.");
                  }//end else if
                  else {
                      int wc = wait(NULL);
                      printf("(pid:%d) Parent UwU (child:%d)", (int) getpid(), wc);
                  }//end else
              }//end pathname1 if
              else if ((access(pathname2, X_OK)) == 0) {
                  //printf("\nSecond");
                  int rc = fork();
                  if (rc < 0) {
                      char error_message[30] = "An error has occurred";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                      free(command);
                      //free(pathname1);
                      //free(pathname2);
                      printf("\ngrsh> ");
                      continue;
                  }//end if
                  else if (rc == 0) {
                      printf("(pid:%d) Child OwO", (int) getpid());
                      execv(pathname2, myargs);
                      printf("Tried to print, didn't come out though.");
                  }//end else if
                  else {
                      int wc = wait(NULL);
                      printf("(pid:%d) Parent UwU (child:%d)", (int) getpid(), wc);
                  }//end else
              }//end pathame2 else if
              else {
                  char error_message[30] = "An error has occurred";
                  write(STDERR_FILENO, error_message, strlen(error_message));
                  free(command);
                  //free(pathname1);
                  //free(pathname2);
                  printf("\ngrsh> ");
                  continue;
              }//end else
              //-------------Code      end-----------------
          }//end else

          free(command);
          //free(pathname1);
          //free(pathname2);

          printf("\ngrsh> ");
      }//end while
  }//end if

  else {
      printf("grsh: filename\n");
      exit(1);
  }//end else

  return 0;
}//end main

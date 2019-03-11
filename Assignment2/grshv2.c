/*
* File : grshv2.c
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
  char *pathname;
  int pathCount = 0;

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
                  char error_message[30] = "An error has occurred\n";
                  write(STDERR_FILENO, error_message, strlen(error_message));
              }//end if
              else {
                  int cd = chdir(myargs[1]);
                  if (cd != 0) {
                      char error_message[30] = "An error has occurred\n";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                  }//end if
              }//end else
          }//end else if
          else if ((strcmp(myargs[0], "path")) == 0) {
              pathCount = count - 1;
              paths = (char**)calloc(pathCount, sizeof(char *));
              for (i = 0; i < pathCount; i++){
                paths[i] = (char *)calloc(strlen(myargs[i+1]), sizeof(char));
                strcpy(paths[i], myargs[i+1]);
              }
              /*for (i = 0; i < pathCount; i++) {
                printf("%s\n", paths[i]);
              }*/
          }
          else {
              //-------------Code continue-----------------
              int success = 0;
              for (i = 0; i < pathCount; i++) {
                //printf("%s\n", paths[i]);
                pathname = malloc((strlen(paths[i])) + (strlen(myargs[0])) + 1);
                strcat(pathname, paths[i]);
                strcat(pathname, "/");
                strcat(pathname, myargs[0]);
                //printf("%s", pathname);

                if ((access(pathname, X_OK)) == 0) {
                  success = 1;
                  int rc = fork();
                  if (rc < 0) {
                      char error_message[30] = "An error has occurred\n";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                  }//end if
                  else if (rc == 0) {
                      //printf("(pid:%d) Child OwO\n", (int) getpid());
                      execv(pathname, myargs);
                      printf("Tried to print, didn't come out though.");
                  }//end else if
                  else {
                      //int wc = wait(NULL);
                      wait(NULL);
                      //printf("(pid:%d) Parent UwU (child:%d)\n", (int) getpid(), wc);
                  }//end else
                }//end if
              }//end for

              if (success == 0) {
                  char error_message[30] = "An error has occurred\n";
                  write(STDERR_FILENO, error_message, strlen(error_message));
              }//end if
              //-------------Code      end-----------------
          }//end else

          free(command);

          printf("grsh> ");
      }//end while
  }//end if

  else {
      printf("grsh: filename\n");
      exit(1);
  }//end else

  return 0;
}//end main

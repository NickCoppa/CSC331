/*
* File : grshv6.c
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
  int pathCalled = 0; //variable to track whether built-in command "path" has ever been called
  char** myargs;
  char** rargs;
  char** pargs;
  char** prargs;

  if (argc == 1) {
      printf("grsh> ");

      //the shell interactive loop
      while (getline(&line, &bufferSize, stdin) > 0) {
          //allocate enough memory to store the trimmed command
          command = (char *)calloc(1, (strlen(line)) + 1);
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

          int i;
          int redirectionCount = 0;
          int validRedirect = 0;
          int commandCount = 1; //number of commands inputted
          int validParallel = 0; //determines whether parallel commands were (validly) inputted
          char* ctoken;
          myargs = (char **)calloc((count+1), (sizeof(char *)));
          rargs  = (char **)calloc((count-1), (sizeof(char *)));
          char *commandDup = (char *)calloc(1, (strlen(command)) + 1);
          strcpy(commandDup, command);

          for (i = 0; i < count; i++) {
            ctoken = strtok_r(commandDup, " ", &commandDup);
            myargs[i] = calloc((strlen(ctoken)), (sizeof(char)));
            strcpy(myargs[i], ctoken);
            if ((strcmp(myargs[i], ">")) == 0) {
                redirectionCount++;
            }
            //counts the number of commands based on how many "&" arguments there were
            else if ((strcmp(myargs[i], "&")) == 0) {
                commandCount++;
            }
            //printf("%s ", myargs[i]);
          }

          //printf("\n");
          myargs[count] = NULL;
          char *fname = myargs[count-1];

          //reduce the command count by 1 if the first argument was a "&"
          if (((strcmp(myargs[0], "&")) == 0)) {
              commandCount--;
          }//end if

          int ampCount = commandCount - 1;
          int ampersandPositions[ampCount];
          int trailingAmp = 0; //keeps track of whether the inputted command ended in "&"

          if ((redirectionCount == 1) && (count >= 3) && ((strcmp(myargs[count-2], ">")) == 0)) {
              validRedirect = 1;
              for (i = 0; i < (count-2); i++) {
                rargs[i] = myargs[i];
              }//end for
              rargs[count-2] = NULL;
          }//end if

          //if there is only one command inputted, then no parallel commands are taking place
          if (commandCount > 1) {
              //ending a command with "&" is valid, so this if statement takes that into account (the next if won't)
              if (((strcmp(myargs[count-1], "&")) == 0)) {
                  validParallel = 1;
                  //if it ends with "&," then there is no additional command
                  commandCount--;
                  trailingAmp = 1;
              }//end if

              //starting a command with "&" is an error
              if (((strcmp(myargs[0], "&")) != 0)) {
                  //checks arguments 1 through (count-1) since 0 was previously checked
                  for (i = 1; i < (count-1); i++) {
                    //consecutive arguments of "&" are invalid
                    if (((strcmp(myargs[i], "&")) == 0) && ((strcmp(myargs[i], myargs[i+1])) == 0)) {
                        validParallel = 0;
                        //prevents the loop from setting validParallel to 1 later
                        break;
                    }//end if
                    else {
                        validParallel = 1;
                    }//end else
                  }//end for
              }//end if
              else {
                  validParallel = 0;
              }//end else

              //if it is not a valid parallel command input, then there is really only one command
              if (validParallel == 0) {
                  commandCount = 1;
              }//end if
              //now the "&" argument indexes are stored
              else if (validParallel == 1) {
                  int j = 0;
                  for (i = 0; i < count; i++) {
                    if (((strcmp(myargs[i], "&")) == 0)) {
                        ampersandPositions[j] = i;
                        j++;
                    }//end if
                  }//end for
              }//end if
          }//end if
/*
          printf("Valid: %d\n", validParallel);
          printf("& positions: ");
          for (i = 0; i < ampCount; i++) {
            printf("%d ", ampersandPositions[i]);
          }
          printf("\nCommand count: %d\n", commandCount);
*/
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
              if (pathCalled == 1) {
                  for (i = 0; i < pathCount; i++) {
                    free(paths[i]);
                  }//end for

                  free(paths);
              }//end if

              pathCount = count - 1;
              pathCalled = 1;
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

              if (validParallel == 0) {
                  for (i = 0; i < pathCount; i++) {
                    //printf("%s\n", paths[i]);
                    pathname = (char *)calloc(1, (strlen(paths[i])) + (strlen(myargs[0])) + 2);
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
                          if (validRedirect == 0) {
                              //printf("(pid:%d) Child OwO\n", (int) getpid());
                              execv(pathname, myargs);
                              printf("Tried to print, didn't come out though.");
                          }
                          else if (validRedirect == 1) {
                              freopen(fname, "w", stdout);
                              freopen(fname, "w", stderr);
                              execv(pathname, rargs);
                              printf("Uh, tried to print. Whatever.");
                          }
                      }//end else if
                      else {
                          //int wc = wait(NULL);
                          wait(NULL);
                          //printf("(pid:%d) Parent UwU (child:%d)\n", (int) getpid(), wc);
                      }//end else
                    }//end if

                    free(pathname);
                  }//end for

                  if (success == 0) {
                      char error_message[30] = "An error has occurred\n";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                  }//end if
              }//end if
              else if (validParallel == 1) {
                  int j;
                  int k;
                  for (i = 0; i < commandCount; i++) {
                    int psize;
                    redirectionCount = 0;
                    validRedirect = 0;

                    if (i == 0) {
                         pargs = (char **)calloc(ampersandPositions[i] + 1, (sizeof(char *)));
                         psize = ampersandPositions[i] + 1;

                         for (j = 0; j < (psize-1); j++) {
                           pargs[j] = myargs[j];
                         }//end for
                         pargs[psize-1] = NULL;
                         /*int l = 0;
                         for (l = 0; l < (psize-1); l++) {
                           printf("%s ", pargs[l]);
                         }
                         printf("\n");*/
                    }//end if
                    else if ((i < (commandCount-1)) || (trailingAmp == 1)) {
                        pargs = (char **)calloc((ampersandPositions[i])-(ampersandPositions[i-1]), (sizeof(char *)));
                        psize = (ampersandPositions[i])-(ampersandPositions[i-1]);

                        j++;
                        k = 0;

                        while (j < (ampersandPositions[i])) {
                            pargs[k] = myargs[j];
                            j++; //4
                            k++; //2
                        }//end while
                        pargs[k] = NULL;
                        /*int l = 0;
                        for (l = 0; l < (psize-1); l++) {
                          printf("%s ", pargs[l]);
                        }
                        printf("\n");*/
                    }//end else if
                    else {
                        pargs = (char **)calloc(count - (ampersandPositions[i-1]), (sizeof(char *)));
                        psize = count - (ampersandPositions[i-1]);

                        j++;
                        k = 0;

                        while (j < count) {
                            pargs[k] = myargs[j];
                            j++;
                            k++;
                        }//end while
                        pargs[k] = NULL;
                        /*int l = 0;
                        for (l = 0; l < (psize-1); l++) {
                          printf("%s ", pargs[l]);
                        }
                        printf("\n");*/
                    }//end else

                    prargs  = (char **)calloc((psize-2), (sizeof(char *)));

                    for (k = 0; k < (psize-1); k++) {
                      if ((strcmp(pargs[k], ">")) == 0) {
                          redirectionCount++;
                      }//end if
                    }//end for

                    //char *pfname = pargs[psize-2];

                    if ((redirectionCount == 1) && (psize >= 4) && ((strcmp(pargs[psize-3], ">")) == 0)) {
                        validRedirect = 1;
                        for (k = 0; k < (psize-3); k++) {
                          prargs[k] = pargs[k];
                        }//end for
                        prargs[psize-3] = NULL;
                    }//end if
                    /*if (validRedirect == 1) {
                        for (k = 0; k < (psize-3); k++) {
                          printf("%s ", prargs[k]);
                        }
                    }*/
/*
                    //-------------------FORK AND EXEC--------------------------------
                    for (i = 0; i < pathCount; i++) {
                      //printf("%s\n", paths[i]);
                      pathname = (char *)calloc(1, (strlen(paths[i])) + (strlen(myargs[0])) + 2);
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
                              if (validRedirect == 0) {
                                  //printf("(pid:%d) Child OwO\n", (int) getpid());
                                  execv(pathname, myargs);
                                  printf("Tried to print, didn't come out though.");
                              }
                              else if (validRedirect == 1) {
                                  freopen(fname, "w", stdout);
                                  freopen(fname, "w", stderr);
                                  execv(pathname, rargs);
                                  printf("Uh, tried to print. Whatever.");
                              }
                          }//end else if
                          else {
                              //int wc = wait(NULL);
                              wait(NULL);
                              //printf("(pid:%d) Parent UwU (child:%d)\n", (int) getpid(), wc);
                          }//end else
                      }//end if

                      free(pathname);
                    }//end for

                    if (success == 0) {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }//end if
                    //-------------------FORK AND EXEC--------------------------------
*/
                    printf("\n");
                    free(pargs);
                    free(prargs);
                  }//end for
              }//end else if
              //-------------Code      end-----------------
          }//end else

          free(command);

          for (i = 0; i < count; i++) {
            free(myargs[i]);
          }

          free(myargs);
          free(rargs);

          printf("grsh> ");
      }//end while
  }//end if

  else {
      printf("grsh: filename\n");
      exit(1);
  }//end else

  return 0;
}//end main

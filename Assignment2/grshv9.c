/*
* File : grshv9.c
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
  char **paths; //the list of paths specified by the user
  char *pathname; //the pathname, including the requested program
  int pathCount = 1; //number of paths specified by the user; initially the path contains /bin
  int pathSet = 0; //variable to track whether built-in command "path" has ever been called
  char** myargs; //the arguments in command
  char** rargs; //the arguments before ">" in a valid redirect statement
  char** pargs; //arguments in an individual command in a parallel command statement
  char** prargs; //the arguments before ">" in a valid redirect statement, in a command in a parallel command statement
  FILE* stream; //with either just be stdin or the given batch text file

  if ((argc == 1) || (argc == 2)) {
      //interactive mode
      if (argc == 1) {
          printf("grsh> ");
          stream = stdin;
      }//end if
      //batch mode
      else if (argc == 2) {
          stream = fopen(argv[1], "r");
          if (stream == NULL) {
              char error_message[30] = "An error has occurred\n";
              write(STDERR_FILENO, error_message, strlen(error_message));
              exit(1);
          }//end if
      }//end else if

      //the shell interactive loop
      while (getline(&line, &bufferSize, stream) > 0) {
          //allocate enough memory to store the trimmed command
          command = (char *)calloc(1, (strlen(line)) + 1);
          //this goes outside the while loop to prevent a space before the trimmed command
          token = strtok_r(line, " \n\t", &line);

          //just skip to the next iteration if nothing was inputted
          if (token == NULL) {
              free(command);
              if (argc == 1) {
                  printf("grsh> ");
              }//end if
              continue;
          }//end if

          strcat(command, token);
          int count = 1; //represents the number of arguments

          //end result of this while loop is a command fully trimmed of excess spaces
          while ((token = strtok_r(line, " \n\t", &line))) {
              //all excess space is trimmed, but we still want one space between arguments
              strcat(command, " ");
              strcat(command, token);
              count++;
          }//end while

          //exit: built-in command
          if ((strcmp(command, "exit")) == 0) {
              int q;
              if (pathSet == 1) {
                  for (q = 0; q < pathCount; q++) {
                    free(paths[q]);
                  }//end for

                  free(paths);
              }//end if

              free(command);
              exit(0);
          }//end if

          int i;
          int redirectionCount = 0; //tracks number of >s
          int validRedirect = 0; //determines whether the inputted command is a valid redirect statement
          int redirectError = 0; //determines whether there is a syntax error in regards to a statement using ">"
          int commandCount = 1; //number of commands inputted
          int validParallel = 0; //determines whether parallel commands were (validly) inputted
          char* ctoken;
          //allocate memory for storing the arguments
          myargs = (char **)calloc((count+1), (sizeof(char *)));
          //allocate memory for storing the arguments in a valid redirect statement
          rargs  = (char **)calloc((count-1), (sizeof(char *)));
          //strtok_r needs to be performed on a copy of the original command
          //char *commandDup = (char *)calloc(1, (strlen(command)) + 1);
          //strcpy(commandDup, command);
          char commDup[(strlen(command))+1];
          strcpy(commDup, command);
          char *commandDup = commDup;

          //now the individual arguments are put into myargs
          for (i = 0; i < count; i++) {
            ctoken = strtok_r(commandDup, " ", &commandDup);
            myargs[i] = calloc((strlen(ctoken)) + 1, (sizeof(char)));
            strcpy(myargs[i], ctoken);
            if ((strcmp(myargs[i], ">")) == 0) {
                redirectionCount++;
            }//end if
            //counts the number of commands based on how many "&" arguments there were
            else if ((strcmp(myargs[i], "&")) == 0) {
                commandCount++;
            }//end else if
          }//end for

          //execv requires that the last argument is NULL
          myargs[count] = NULL;
          //in a valid redirect statement, the last argument (other than NULL) must be the file
          char *fname = myargs[count-1];

          //sets the initial path to "/bin"
          if (pathSet == 0) {
              paths = (char**)calloc(pathCount, sizeof(char *));
              for (i = 0; i < pathCount; i++) {
                paths[i] = (char *)calloc(5, sizeof(char));
                strcpy(paths[i], "/bin");
              }//end for
          }//end if
          pathSet = 1;

          //reduce the command count by 1 if the first argument was a "&"
          if (((strcmp(myargs[0], "&")) == 0)) {
              commandCount--;
          }//end if

          int ampCount = commandCount - 1;
          int ampersandPositions[ampCount]; //will keep track of the indexes of each "&"
          int trailingAmp = 0; //keeps track of whether the inputted command ended in "&"

          //all of the arguments before the ">" in a valid redirect statement are put into rargs
          if ((redirectionCount == 1) && (count >= 3) && ((strcmp(myargs[count-2], ">")) == 0)) {
              validRedirect = 1;
              for (i = 0; i < (count-2); i++) {
                rargs[i] = myargs[i];
              }//end for
              rargs[count-2] = NULL;
          }//end if
          else if ((redirectionCount > 1) || ((redirectionCount == 1) && ( (count < 3) || ((strcmp(myargs[count-2], ">")) != 0)))) {
              redirectError = 1;
          }//end else if

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

          if ((strcmp(myargs[0], "cd")) == 0) {
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
          }//end if
          else if ((strcmp(myargs[0], "path")) == 0) {
              for (i = 0; i < pathCount; i++) {
                free(paths[i]);
              }//end for

              free(paths);

              pathCount = count - 1;
              paths = (char**)calloc(pathCount, sizeof(char *));
              for (i = 0; i < pathCount; i++) {
                paths[i] = (char *)calloc((strlen(myargs[i+1])) + 1, sizeof(char));
                strcpy(paths[i], myargs[i+1]);
              }//end for
          }//end else if
          else {
              int success = 0; //determines whether the pathname was successfully accessed at all

              if (validParallel == 0) {
                  if (redirectError == 0) {
                      //each path is looped through, and upon successful access, fork and execv are called
                      for (i = 0; i < pathCount; i++) {
                        pathname = (char *)calloc(1, (strlen(paths[i])) + (strlen(myargs[0])) + 2);
                        strcat(pathname, paths[i]);
                        strcat(pathname, "/");
                        strcat(pathname, myargs[0]);

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
                              }//end if
                              else if (validRedirect == 1) {
                                  freopen(fname, "w", stdout);
                                  freopen(fname, "w", stderr);
                                  execv(pathname, rargs);
                                  printf("Uh, tried to print. Whatever.");
                              }//end else if
                          }//end else if
                          else {
                              wait(NULL);
                              //printf("(pid:%d) Parent UwU (child:%d)\n", (int) getpid(), wc);
                          }//end else
                        }//end if

                        free(pathname); //must be freed before pathname is has memory allocated on the next iteration
                      }//end for

                      if (success == 0) {
                          char error_message[30] = "An error has occurred\n";
                          write(STDERR_FILENO, error_message, strlen(error_message));
                      }//end if
                  }//end if
                  else if (redirectError == 1) {
                      char error_message[30] = "An error has occurred\n";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                  }//end else if
              }//end if
              else if (validParallel == 1) {
                  int j; //index tracking; does not get reset after each iteration of the outer for loop
                  int k; //index tracking
                  //loops through every command according to indexes in pargs
                  for (i = 0; i < commandCount; i++) {
                    int psize; //size of pargs, including the required NULL
                    redirectionCount = 0;
                    validRedirect = 0;
                    redirectError = 0;
                    success = 0;

                    //this will contain arguments from index 0 to the first "&"
                    if (i == 0) {
                         pargs = (char **)calloc(ampersandPositions[i] + 1, (sizeof(char *)));
                         psize = ampersandPositions[i] + 1;

                         //arguments are added to pargs
                         for (j = 0; j < (psize-1); j++) {
                           pargs[j] = myargs[j];
                         }//end for
                         pargs[psize-1] = NULL;
                    }//end if
                    //this will contain arguments between &s
                    else if ((i < (commandCount-1)) || (trailingAmp == 1)) {
                        pargs = (char **)calloc((ampersandPositions[i])-(ampersandPositions[i-1]), (sizeof(char *)));
                        psize = (ampersandPositions[i])-(ampersandPositions[i-1]);

                        j++;
                        k = 0;

                        while (j < (ampersandPositions[i])) {
                            pargs[k] = myargs[j];
                            j++;
                            k++;
                        }//end while
                        pargs[k] = NULL;
                    }//end else if
                    //this will contain arguments from after the last relevant "&" to the very last argument
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
                    }//end else

                    //allocate memory for prargs
                    prargs  = (char **)calloc((psize-2), (sizeof(char *)));

                    //count number of >s
                    for (k = 0; k < (psize-1); k++) {
                      if ((strcmp(pargs[k], ">")) == 0) {
                          redirectionCount++;
                      }//end if
                    }//end for

                    char *pfname = pargs[psize-2];

                    //again, validating whether there is a valid redirect statement
                    if ((redirectionCount == 1) && (psize >= 4) && ((strcmp(pargs[psize-3], ">")) == 0)) {
                        validRedirect = 1;
                        for (k = 0; k < (psize-3); k++) {
                          prargs[k] = pargs[k];
                        }//end for
                        prargs[psize-3] = NULL;
                    }//end if
                    else if ((redirectionCount > 1) || ((redirectionCount == 1) 
                            && ((psize < 4) || ((strcmp(myargs[psize-3], ">")) != 0)))) {
                        redirectError = 1;
                    }//end else if

                    if (redirectError == 0) {
                        //each path will be looped through in an attempt to successfully access the program
                        for (k = 0; k < pathCount; k++) {
                          pathname = (char *)calloc(1, (strlen(paths[k])) + (strlen(pargs[0])) + 2);
                          strcat(pathname, paths[k]);
                          strcat(pathname, "/");
                          strcat(pathname, pargs[0]);

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
                                      execv(pathname, pargs);
                                      printf("Tried to print, didn't come out though.");
                                  }
                                  else if (validRedirect == 1) {
                                      freopen(pfname, "w", stdout);
                                      freopen(pfname, "w", stderr);
                                      execv(pathname, prargs);
                                      printf("Uh, tried to print. Whatever.");
                                  }
                              }//end else if
                          }//end if

                          free(pathname);
                        }//end for

                        if (success == 0) {
                            char error_message[30] = "An error has occurred\n";
                            write(STDERR_FILENO, error_message, strlen(error_message));
                        }//end if
                    }//end if
                    else if (redirectError == 1) {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }//end else if

                    free(pargs);
                    free(prargs);
                  }//end for

                  //after fork and execv have been called for all specified programs, now the parent process will
                  //wait for each child to terminate in some undetermined order
                  for (i = 0; i < commandCount; i++) {
                    wait(NULL);
                  }//end for
              }//end else if
          }//end else

          free(command);

          for (i = 0; i < count; i++) {
            free(myargs[i]);
          }

          free(myargs);
          free(rargs);

          //only in interactive mode
          if (argc == 1) {
              printf("grsh> ");
          }//end if
      }//end while
  }//end if

  else {
      printf("grsh: filename\n");
      exit(1);
  }//end else

  return 0;
}//end main


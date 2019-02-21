/*
* File : my-grep.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[]) {
  int i;
  int str_len;
  size_t bufferSize = 0;
  char *line = NULL;
  char *searchterm = argv[1];

  if (argc == 1) {
      printf("my-grep: searchterm [file ...]\n");
      exit(1);
  }//end if
  else if (argc == 2) {
      int k;
      char substring[strlen(searchterm)];
      while ((str_len = getline(&line, &bufferSize, stdin)) > 0) {
          for (k = 0; k < strlen(line); k++) {
            memcpy(substring, &line[k], strlen(searchterm));
            substring[strlen(searchterm)] = '\0';
            if ((strcmp(searchterm, substring)) == 0) {
                printf("%s", line);
                break;
            }//end if
          }//end for
      }//end while
  }//end else if
  else {
    for (i = 2; i < argc; i++) {
      FILE *fp = fopen(argv[i], "r");
      if (fp == NULL) {
          printf("my-grep: cannot open file\n");
          exit(1);
      }
/*
    while (fgets (buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
*/
      int j;
      char substring[strlen(searchterm)];
      while ((str_len = getline(&line, &bufferSize, fp)) > 0) {
          for (j = 0; j < str_len; j++) {
            memcpy(substring, &line[j], strlen(searchterm));
            substring[strlen(searchterm)] = '\0';
            if ((strcmp(searchterm, substring)) == 0) {
                printf("%s", line);
                break;
            }//end if
          }//end for
      }//end while

      fclose(fp);
    }//end for
  }//end else

  return 0;
}

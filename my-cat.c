/*
* File : my-cat.c
*/
#include <stdlib.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
  int i;
  char buffer[1000];
  for (i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
        printf("my-cat: cannot open file\n");
        exit(1);
    }

    while (fgets (buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    fclose(fp);
  }

  return 0;
}

/*
* File : my-unzip.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[]) {
  int i;
  int chara;
  int count;

  if (argc < 2) {
      printf("my-unzip: file1 [file2 ...]\n");
      exit(1);
  }

  for (i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
        printf("my-unzip: cannot open file\n");
        exit(1);
    }

    while ((fread(&count, sizeof(int), 1, fp)) > 0) {
        chara = fgetc(fp);
        for (int j = 0; j < count; j++) {
          printf("%c", chara);
        }
    }

    printf("\n");

    fclose(fp);
  }

  return 0;
}

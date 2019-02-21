/*
* File : my-zip.c
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[]) {
  int i;
  size_t bufferSize = 0;
  char *buffer = NULL;
  int chara;
  int other;
  int count;

  if (argc < 2) {
      printf("my-zip: file1 [file2 ...]\n");
      exit(1);
  }

  for (i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
        printf("my-zip: cannot open file\n");
        exit(1);
    }

    while ((getline(&buffer, &bufferSize, fp)) > 0) {
        //printf("%s", buffer);
    }

    int j;
    chara = buffer[0];
    other = buffer[0];
    count = 0;
    for (j = 0; j < strlen(buffer); j++) {
      if (chara != other) {
          fwrite(&count, sizeof(int), 1, stdout);
          fputc(other, stdout);
          //printf("%d", count);
          //printf("%c", other);
          count = 1;
      }
      else {
          count++;
      }

      other = chara;
      chara = buffer[j+1];
    }

    fclose(fp);
  }

  return 0;
}

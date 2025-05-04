#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TARGET_LINE 1000
#define MAX_LINE_LENGTH 1024
#define MAX_STRING_SIZE 3000
#define BATCH_SIZE 1000




int main(int argc, char *argv[]) {
  

    FILE *fp = fopen("/homes/dan/625/wiki_dump.txt", "r");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    char buffer[MAX_STRING_SIZE];
    int line_num = 0;
    int count = 0;

    char **lines = (char **)malloc(BATCH_SIZE * sizeof(char *));


    while (fgets(buffer, MAX_STRING_SIZE, fp) && count < 200) {
       
         buffer[strcspn(buffer, "\n")] = 0;
        
        lines[count] = strdup(buffer);
        
        count++;
        if(count == 200){
            fclose(fp);
            return 0;
        }
        if(count == BATCH_SIZE) {count = 0;}
        line_num++;
    }

    fclose(fp);
    return 0;
}


// char **lines = (char **)malloc(max_lines * sizeof(char *));\

// buffer[strcspn(buffer, "\n")] = 0; \

// while (fgets(buffer, 3000, fp) && totallines < max_lines{}

// lines[totallines] = buffer

// or lines[totallines] = strdup(buffer)






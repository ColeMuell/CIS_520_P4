 

#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 2200
#define MAX_LINES 200

int main() {


     FILE *fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
    FILE *output = fopen("200_lines.txt", "w");

  

    char buffer[MAX_LINE_LENGTH];
    int count = 0;

    // while (fgets(buffer, MAX_LINE_LENGTH, input)) {
    //     fputs(buffer, output);
    //     if(count == 999999){
    //     printf("count%s\n",buffer);
    //     }
    //     count++;
    // }

    size_t len;
  

        while (fgets(buffer, MAX_LINE_LENGTH, fd) ) {
       
                // buffer[strcspn(buffer, "\n")] = 0;
                
                // lines[count] = strdup(buffer);
                
               
                count++;



        }
        printf("count %d\n", count);
        for(int k = 0;k < 1000;k++){

        while (fgets(buffer, MAX_STRING_SIZE, fd) && count < BATCH_SIZE) {
       
                buffer[strcspn(buffer, "\n")] = 0;
                
                lines[count] = strdup(buffer);
                
               
                count++;
        }
        }
        // total_read +=count;

    // fclose(input);
    // fclose(output);

    // printf("Wrote %d lines to 200_lines.txt\n", count);
    return 0;
}

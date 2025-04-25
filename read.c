 

#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 3000
#define MAX_LINES 200

int main() {


     FILE *input = fopen("/homes/dan/625/wiki_dump.txt", "r");
    FILE *output = fopen("200_lines.txt", "w");

    if (!input || !output) {
        perror("Error opening file");
        return 1;
    }

    char buffer[MAX_LINE_LENGTH];
    int count = 0;

    while (fgets(buffer, MAX_LINE_LENGTH, input) && count < MAX_LINES) {
        fputs(buffer, output);
        count++;
    }

    fclose(input);
    fclose(output);

    printf("Wrote %d lines to 200_lines.txt\n", count);
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "m3u8_downloader.h"

static int m3u8_parser_count(FILE *f) {
    char buff[512];
    char *ptr;
    int total_segs = 0;

    while(fgets(buff, sizeof(buff), f) != NULL) {
        ptr = buff;

        while(*ptr == ' ') ptr++;

        if (strncasecmp("http", ptr, 4) == 0) {
            // found http or https protocol
            total_segs++;
        }
    }

    return total_segs;
}

int m3u8_parser_from_file(const char *m3u8_file_name, m3u8_seg **list_segs, int *number) {
    FILE* f;
    char buff[512];
    int total_segs = 0;
    m3u8_seg *segs;
    int i;

    f = fopen(m3u8_file_name, "r");

    if(f == NULL) {
        fprintf(stderr, "cannot open file %s\n", m3u8_file_name);

        return 0;
    }

    *number = total_segs = m3u8_parser_count(f);

    if (total_segs > 0) {

        *list_segs = segs = (m3u8_seg *) malloc(sizeof(m3u8_seg) * total_segs);
        memset(segs, 0, sizeof(m3u8_seg) * total_segs);

        fseek(f, 0, SEEK_SET);
        i = 0;
        while(fgets(buff, sizeof(buff), f) != NULL) {
            char *ptr = buff;

            while(*ptr == ' ') ptr++;

            if (strncasecmp("http", ptr, 4) == 0) {
                // found http or https protocol
                if(ptr[strlen(ptr) - 1] == '\n') {
                    ptr[strlen(ptr) - 1] = '\0';
                }

                snprintf(segs[i].url, 256, "%s", ptr);
                snprintf(segs[i].filename, 128, "seg-%03d", i);
                // printf("%s\n", segs[i].filename);
                i++;
            }
        }
    }

    fclose(f);

    return 1;
}
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "m3u8_downloader.h"

#define _DEBUG

void show_help(const char *app_name) {
    fprintf(stderr, "%s <path_of_m3u8> <out_file>\n", app_name);
}

static const char *status2str(int status) {
    switch (status) {
        case NOT_SARTED: return "NOT_SARTED";
        case ON_GOING: return "ON_GOING";
    }
    return "COMPLETED";
}

int main(int argc, char const *argv[])
{
    m3u8_seg *list_segs = NULL;
    int number;
    int completed = 0;
    int i;
    FILE *outfile;

    if (argc != 3) {
        show_help(argv[0]);
        return -1;
    }

    outfile = fopen(argv[2], "w");
    if (outfile == NULL) {
        show_help(argv[0]);
        return -1;
    }

    if (m3u8_parser_from_file(argv[1], &list_segs, &number)) {

        m3u8_download_m3u8_segs(list_segs, number);

        for (i = 0; i < number; ++i) {
            printf("%s - %s\n",
                list_segs[i].url,
                status2str(list_segs[i].status));
            if (list_segs[i].status == COMPLETED) {
                completed++;
            }
        }

        if (completed == number) {
            m3u8_merge(outfile, list_segs, number);
        }

        fclose(outfile);
    }

    if (list_segs) {
        free(list_segs);
    }

    return 0;
}

static int merge(FILE *outfile, const char* infile) {
    FILE *f;
    char buf[512];
    int n;

    f = fopen(infile, "r");
    if (f == NULL) {
        fprintf(stderr, "cannot open file %s\n", infile);
    }

    while((n = fread(buf, 1, 512, f)) > 0) {
        fwrite(buf, n, 1, outfile);
    }

    fclose(f);
    return 1;
}

int m3u8_merge(FILE *outfile, m3u8_seg *list_segs, int number) {
    int i;
    for (i = 0; i < number; ++i) {
        if (merge(outfile, list_segs[i].filename) == 0) {
            return 0;
        }
    }
    return 1;
}
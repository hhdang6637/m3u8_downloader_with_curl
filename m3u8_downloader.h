#ifndef M3U8_DOWNLOADER_H
#define M3U8_DOWNLOADER_H

#define NOT_SARTED  0
#define ON_GOING    1
#define COMPLETED   2

typedef struct m3u8_seg {
    char    url[256];
    char    filename[128];
    int     status;
} m3u8_seg;

int m3u8_parser_from_file(const char *m3u8_file_name, m3u8_seg **list_segs, int *number);

void m3u8_download_m3u8_segs(m3u8_seg *list_segs, int number);
int m3u8_merge(FILE*outfile, m3u8_seg *list_segs, int number);

#endif
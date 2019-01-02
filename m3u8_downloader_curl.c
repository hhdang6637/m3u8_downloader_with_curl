#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <curl/curl.h>

#include "m3u8_downloader.h"

typedef struct downloadItem
{
    FILE        *fileptr;
    CURL        *curl_handle;
    m3u8_seg    *seg;
} downloadItem;

static int maximum_concurrent_file = 20;

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    FILE* stream = (FILE*) userdata;
    fwrite(ptr, size, nmemb, stream);
    return size * nmemb;
}

static downloadItem * add_curl_handle(downloadItem *item, m3u8_seg *list_segs, int number) {
    int i;
    m3u8_seg *seg = NULL;

    for(i = 0; i < number; i++) {
        if (list_segs[i].status == NOT_SARTED) {
            list_segs[i].status = ON_GOING;
            seg = &list_segs[i];
            break;
        }
    }

    if (seg == NULL) {
        return NULL;
    }

    printf("processing url = %s\n", seg->url);

    if (item->curl_handle) {
        curl_easy_cleanup(item->curl_handle);
        item->curl_handle = NULL;
    }

    item->curl_handle = curl_easy_init();
    curl_easy_setopt(item->curl_handle, CURLOPT_URL, seg->url);
    curl_easy_setopt(item->curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    item->fileptr = fopen(seg->filename, "w+");
    curl_easy_setopt(item->curl_handle, CURLOPT_WRITEDATA, item->fileptr);
    item->seg = seg;
    // curl_easy_setopt(item->curl_handle, CURLOPT_VERBOSE, 1L);

    return item;
}

void set_maximum_concurrent_file(int number) {
    if (number > 2 || number < 20) {
        maximum_concurrent_file = number;
    }
}

void m3u8_download_m3u8_segs(m3u8_seg *list_segs, int number) {
    int i;
    CURLM   *multi_handle;
    CURLMcode retm;
    int     still_running = 0; /* keep number of running handles */
    downloadItem    *downloadItems = (downloadItem *)malloc(sizeof(downloadItem) * maximum_concurrent_file);

    memset(downloadItems, 0, sizeof(downloadItem) * maximum_concurrent_file);

    /* init a multi stack */
    multi_handle = curl_multi_init();

    /* Allocate one CURL handle per transfer */
    for (i = 0; i < maximum_concurrent_file; i++) {
        if (add_curl_handle(&downloadItems[i], list_segs, number)) {
            retm = curl_multi_add_handle(multi_handle, downloadItems[i].curl_handle);
            if (retm != 0) {
                printf("curl_multi_add_handle: curl_multi_wait failed, retm=%d\n", (int)retm);
            }
        }
    }

    /* we start some action by calling perform right away */
    retm = curl_multi_perform(multi_handle, &still_running);
    if (retm != 0) {
        printf("m3u8_download_m3u8_segs: curl_multi_wait failed, retm=%d\n", (int)retm);
    }

    while (still_running) {
        int numfds = 0;

        retm = curl_multi_wait(multi_handle, NULL, 0, 1000, &numfds);
        if (retm != 0) {
            printf("m3u8_download_m3u8_segs: curl_multi_wait failed, retm=%d\n", (int)retm);
        }

        curl_multi_perform(multi_handle, &still_running);

        CURLMsg * msg;
        do
        {
            int msgq = 0;

            msg = curl_multi_info_read(multi_handle, &msgq);
            if(msg && (msg->msg == CURLMSG_DONE))
            {
                curl_multi_remove_handle(multi_handle, msg->easy_handle);
                // dump msg->easy_handle and msg->data.result
                for (i = 0; i < maximum_concurrent_file; i++) {
                    if (downloadItems[i].curl_handle == msg->easy_handle) {
                        fclose(downloadItems[i].fileptr);
                        downloadItems[i].seg->status = COMPLETED;
                        if (add_curl_handle(&downloadItems[i], list_segs, number)) {
                            curl_multi_add_handle(multi_handle, downloadItems[i].curl_handle);
                        }
                    }
                }
            }
        } while(msg != NULL);
    }

    curl_multi_cleanup(multi_handle);
    /* Free the CURL curl_handles */
    for (i = 0; i < maximum_concurrent_file; i++) {
        curl_easy_cleanup(downloadItems[i].curl_handle);
    }
    free(downloadItems);
}

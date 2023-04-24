#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>

struct sz_buffer {
    char * buf ;
    size_t sz ;
};

static size_t sz_buffer_cb(void * contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb ;
    struct sz_buffer * szb = (struct sz_buffer*)userp ;

    char * ptr = realloc(szb->buf, szb->sz + realsize +1);
    if (!ptr)
        return 0;
    szb->buf = ptr ;
    memcpy(&(szb->buf[szb->sz]), contents, realsize);
    szb->sz += realsize ;
    szb->buf[szb->sz] = 0 ;
    return realsize;
}

char * http_get(char * url, size_t * sz)
{
    CURL    * curl_handle ;
    CURLcode  r ;
    struct sz_buffer chunk ;

    chunk.buf = malloc(1);
    chunk.sz  = 0 ;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, sz_buffer_cb);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "licurl-agent/1.0");
    r = curl_easy_perform(curl_handle);
    if (r!=CURLE_OK) {
        printf("err getting [%s]: %s\n", url, curl_easy_strerror(r));
        return NULL ;
    }
    *sz = chunk.sz ;
    return chunk.buf ;
}

#if 0
int main(int argc, char * argv[])
{
    char * url = "http://po:8080/a.c";
    char * buf ;
    size_t sz ;

	if (argc>1) {
        url = argv[1];
	}
    buf = http_get(url, &sz);
    printf("url[%s] sz[%d]\n", url, sz);
    printf("---\n");
    printf("%s", buf);
    printf("---\n");
    free(buf);

	return 0 ;
}
#endif

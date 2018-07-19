#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <unistd.h>

struct string
{
    char *ptr;
    size_t len;
};

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = (char *)calloc(s->len + 1, sizeof(char));
    s->ptr[0] = '\0';
}

size_t callback(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_size = size * nmemb;
    size_t new_len = s->len + new_size;
    s->ptr = realloc(s->ptr, new_len + 1);

    memcpy(s->ptr + s->len, ptr, new_size);
    s->ptr[new_len] = '\0';
    s->len = new_len;
    return new_size;
}

int main(int argc, char **argv)
{
    char URL[255];
    if (argc == 2)
    {
        strcpy(URL, argv[1]);
    }
    else
    {
        printf("curl_poster will send a json array to URL and print response json array\n");
        printf("Usage:\n\tcurl_poster [URL]\n");
        printf("Example:\n\tcurl_poster localhost:8888/hello\n");
        exit(0);
    }

    struct json_object *root = json_object_new_object();
    struct json_object *array = json_object_new_array();
    for (int i = 0; i < 10; i++)
    {
        json_object_array_add(array, json_object_new_int(i));
    }

    json_object_object_add(root, "meta-data", array);

    CURL *session = curl_easy_init();
    char *POST;

    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(session, CURLOPT_HTTPHEADER, headers);

    POST = strdup(json_object_to_json_string(root));
    json_object_put(root);

    struct string s;
    init_string(&s);

    curl_easy_setopt(session, CURLOPT_URL, URL);
    curl_easy_setopt(session, CURLOPT_POSTFIELDS, POST);
    curl_easy_setopt(session, CURLOPT_POSTFIELDSIZE, -1L);
    curl_easy_setopt(session, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(session, CURLOPT_WRITEDATA, &s);

    for (int i = 0; i < 10000; i++)
    {
        CURLcode res = curl_easy_perform(session);
        if (res != CURLE_OK)
        {

            fprintf(stderr, "%s\n", curl_easy_strerror(res));
            break;
        }

        long http_code = 0;
        curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &http_code);

        printf("%s\n", s.ptr);
        free(s.ptr);

        if (http_code != 200)
        {
            printf("%ld\n", http_code);
            break;
        }

        s.len = 0;
        init_string(&s);
        usleep(10000);
    }

    curl_easy_cleanup(session);
    return 0;
}
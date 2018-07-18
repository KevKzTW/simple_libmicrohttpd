#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

int main(int argc, char **argv)
{
    struct json_object *root = json_object_new_object();
    json_object_object_add(root, "key", json_object_new_double(3.1415926));

    CURL *curl = curl_easy_init();
    char POST[255];

    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    const char *jsonString = json_object_to_json_string(root);
    strcpy(POST, jsonString);
    json_object_put(root);

    curl_easy_setopt(curl, CURLOPT_URL, "localhost:8888/hello");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POST);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    return 0;
}
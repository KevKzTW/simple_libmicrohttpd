#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

bool terminated = false;

static void ctrlHandler(int fdwCtrlType)
{
    terminated = true;
}

typedef struct _ExampleTimeStats
{
    unsigned long *values;
    unsigned long valuesSize;
    unsigned long valuesMax;
    double average;
    unsigned long min;
    unsigned long max;
    unsigned long count;
} ExampleTimeStats;

static void exampleInitTimeStats(ExampleTimeStats *stats)
{
    stats->values = (unsigned long *)calloc(50000, sizeof(unsigned long));
    stats->valuesSize = 0;
    stats->valuesMax = 50000;
    stats->average = 0;
    stats->min = 0;
    stats->max = 0;
    stats->count = 0;
}

static void exampleResetTimeStats(ExampleTimeStats *stats)
{
    unsigned long valuesMax = stats->valuesMax;
    for (unsigned long i = 0; i < valuesMax; i++)
        stats->values[i] = 0;
    stats->valuesSize = 0;
    stats->average = 0;
    stats->min = 0;
    stats->max = 0;
    stats->count = 0;
}

static void exampleDeleteTimeStats(ExampleTimeStats *stats)
{
    free(stats->values);
}

static ExampleTimeStats *exampleAddTimingToTimeStats(ExampleTimeStats *stats, unsigned long timing)
{
    if (stats->valuesSize >= stats->valuesMax)
    {
        unsigned long *temp = (unsigned long *)realloc(stats->values, (stats->valuesMax + 50000) * sizeof(unsigned long));
        stats->values = temp;
        stats->valuesMax += 50000;
    }

    stats->values[stats->valuesSize++] = timing;
    stats->average = (stats->count * stats->average + timing) / (stats->count + 1);
    stats->min = (stats->count == 0 || timing < stats->min) ? timing : stats->min;
    stats->max = (stats->count == 0 || timing > stats->max) ? timing : stats->max;
    stats->count++;

    return stats;
}

static int exampleCompareul(const void *a, const void *b)
{
    unsigned long *_a = (unsigned long *)a;
    unsigned long *_b = (unsigned long *)b;

    if (*_a < *_b)
        return -1;
    else if (*_a > *_b)
        return 1;
    else
        return 0;
}

static double exampleGetMedianFromTimeStats(ExampleTimeStats *stats)
{
    qsort(stats->values, stats->valuesSize, sizeof(unsigned long), exampleCompareul);

    if (stats->valuesSize % 2 == 0)
    {
        return (double)(stats->values[stats->valuesSize / 2 - 1] + stats->values[stats->valuesSize / 2]) / 2;
    }
    else
    {
        return (double)stats->values[stats->valuesSize / 2];
    }
}

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
    unsigned long payloadSize = 1;
    if (argc >= 2)
    {
        strcpy(URL, argv[1]);
    }
    if (argc >= 3)
    {
        payloadSize = atoi(argv[2]);
    }
    if (argc < 2 || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)))
    {
        printf("Usage:\n./ping [URL] [payloadSize (bytes)]\n");
        printf("./ping localhost:8888/hello 256\n");
        exit(0);
    }

    struct json_object *root = json_object_new_object();
    struct json_object *array = json_object_new_array();
    for (int i = 0; i < payloadSize; i++)
    {
        json_object_array_add(array, json_object_new_string("a"));
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

    struct sigaction sat, oldAction;
    sat.sa_handler = ctrlHandler;
    sigemptyset(&sat.sa_mask);
    sat.sa_flags = 0;
    sigaction(SIGINT, &sat, &oldAction);

    ExampleTimeStats roundTrip;
    exampleInitTimeStats(&roundTrip);

    ExampleTimeStats roundTripOverall;
    exampleInitTimeStats(&roundTripOverall);

    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    unsigned long elapsed = 0;
    printf("# Round trip measurements (in us) , payloadSize: %lu\n", payloadSize);
    printf("# Seconds      Count   median     min\n");
    while (!terminated)
    {
        struct timeval preTakeTime;
        struct timeval postTakeTime;

        gettimeofday(&preTakeTime, NULL);
        CURLcode res = curl_easy_perform(session);
        //printf("===%s\n", s.ptr);
        gettimeofday(&postTakeTime, NULL);
        unsigned long difference = 1000000 * (postTakeTime.tv_sec - preTakeTime.tv_sec) + (postTakeTime.tv_usec - preTakeTime.tv_usec);

        roundTrip = *exampleAddTimingToTimeStats(&roundTrip, difference);
        roundTripOverall = *exampleAddTimingToTimeStats(&roundTripOverall, difference);

        difference = 1000000 * (postTakeTime.tv_sec - startTime.tv_sec) + (postTakeTime.tv_usec - startTime.tv_usec);
        if (difference > 1000000)
        {
            printf("%9" PRIi64 " %9lu %8.0f %8" PRIi64 "\n", elapsed++, roundTrip.count, exampleGetMedianFromTimeStats(&roundTrip), roundTrip.min);
            exampleResetTimeStats(&roundTrip);
            gettimeofday(&startTime, NULL);
        }

        if (res != CURLE_OK)
        {

            fprintf(stderr, "%s\n", curl_easy_strerror(res));
            break;
        }

        long http_code = 0;
        curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &http_code);

        //printf("%s\n", s.ptr);
        free(s.ptr);

        if (http_code != 200)
        {
            printf("%ld\n", http_code);
            break;
        }

        s.len = 0;
        init_string(&s);
    }

    sigaction(SIGINT, &oldAction, 0);

    printf("\n# Overall %9lu %8.0f %8" PRIi64 "\n", roundTripOverall.count, exampleGetMedianFromTimeStats(&roundTripOverall), roundTripOverall.min);

    exampleDeleteTimeStats(&roundTrip);
    exampleDeleteTimeStats(&roundTripOverall);
    curl_easy_cleanup(session);
    printf("clean up\n");
    return 0;
}

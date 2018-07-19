/* Feel free to use this example code in any way
   you see fit (Public Domain) */

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <json-c/json.h>

#if defined(_MSC_VER) && _MSC_VER + 0 <= 1800
/* Substitution is OK while return value is not used */
#define snprintf _snprintf
#endif

#define PORT 8888

struct postStatus
{
  int len;
  bool status;
  char *buffer;
};

static int
send_page(struct MHD_Connection *connection, const char *page, int status_code, enum MHD_ResponseMemoryMode mode, char *mimetype)
{
  int ret;
  struct MHD_Response *response;

  response =
      MHD_create_response_from_buffer(strlen(page), (void *)page,
                                      mode);
  if (!response)
    return MHD_NO;

  MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, mimetype);

  ret = MHD_queue_response(connection, status_code, response);

  MHD_destroy_response(response);

  return ret;
}

static int
answer_to_connection(void *cls, struct MHD_Connection *connection,
                     const char *url, const char *method,
                     const char *version, const char *upload_data,
                     size_t *upload_data_size, void **con_cls)
{
  struct postStatus *post = (struct postStatus *)*con_cls;

  if (strcmp(method, "POST") == 0)
  {
    const char *param = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_LENGTH);
    int length_of_content = atoi(param);
    const char *req_content_type = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Content-Type");

    if (strcmp(url, "/hello") != 0)
      return send_page(connection, "Not found: using url [ip]:8888/hello", MHD_HTTP_NOT_FOUND, MHD_RESPMEM_PERSISTENT, "application/json");

    if (post == NULL)
    {
      post = (struct postStatus *)calloc(1, sizeof(struct postStatus));
      post->status = false;
      post->len = 0;
      post->buffer = (char *)calloc(length_of_content + 1, sizeof(char));
      *con_cls = post;
    }

    if (!post->status)
    {
      post->status = true;
      return MHD_YES;
    }

    if (*upload_data_size != 0)
    {
      memcpy(post->buffer + post->len, upload_data, *upload_data_size);
      post->len = post->len + *upload_data_size;
      post->buffer[post->len] = '\0';

      *upload_data_size = 0;
      return MHD_YES;
    }
    else
    {
      if (post->buffer)
      {
        printf("Request \n");
        if (strcmp(req_content_type, "application/json") == 0)
        {
          struct json_object *root = json_tokener_parse(post->buffer);
          struct json_object *array = json_object_object_get(root, "meta-data");
          if (array)
          {
            size_t LL = json_object_array_length(array);
            for (size_t i = 0; i < LL; i++)
            {
              struct json_object *element = json_object_array_get_idx(array, i);
              int val = json_object_get_int(element);
              json_object_set_int(element, val * val);
            }
          }

          char *response_page = strdup(json_object_to_json_string(root));
          json_object_put(root);

          free(post->buffer);
          free(post);
          static int count = 0;
          printf("Response %d\n", count++);
          int ret = send_page(connection, response_page, MHD_HTTP_OK, MHD_RESPMEM_MUST_FREE, "application/json");

          return ret;
        }
        else
        {
          free(post->buffer);
          free(post);

          return send_page(connection, "Bad request", MHD_HTTP_BAD_REQUEST, MHD_RESPMEM_PERSISTENT, "application/json");
        }
      }
    }
  }
  return MHD_NO;
}

int main()
{
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                            &answer_to_connection, NULL,
                            MHD_OPTION_END);
  if (NULL == daemon)
    return 1;

  printf("\n\n\nhttpd up\n press any key to exit\n");
  (void)getchar();

  MHD_stop_daemon(daemon);

  return 0;
}
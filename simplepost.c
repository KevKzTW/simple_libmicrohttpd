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
#define POSTBUFFERSIZE 512
#define MAXNAMESIZE 20
#define MAXANSWERSIZE 512

#define GET 0
#define POST 1

const char *errorpage =
    "<html><body>This doesn't seem to be right.</body></html>";

struct postStatus
{
  int len;
  bool status;
  char *buffer;
};

static int
send_page(struct MHD_Connection *connection, const char *page)
{
  int ret;
  struct MHD_Response *response;

  response =
      MHD_create_response_from_buffer(strlen(page), (void *)page,
                                      MHD_RESPMEM_PERSISTENT);
  if (!response)
    return MHD_NO;

  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
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
    else
    {
      if (*upload_data_size != 0)
      {
        printf("url = %s\n", url);
        memcpy(post->buffer + post->len, upload_data, *upload_data_size);
        post->len = post->len + *upload_data_size;
        post->buffer[post->len] = '\0';
        struct json_object *root = json_tokener_parse(post->buffer);
        if (root)
        {

          struct json_object *val = json_object_object_get(root, "key");
          double aaa = json_object_get_double(val);
          printf("%lf\n", aaa);
          json_object_put(root);
        }
        *upload_data_size = 0;
        return MHD_YES;
      }
    }

    return send_page(connection, errorpage);
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

  (void)getchar();

  MHD_stop_daemon(daemon);

  return 0;
}
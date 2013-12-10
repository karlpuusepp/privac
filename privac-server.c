/*
 * privac-server.c
 * dumb broadcast chat server
 * Karl Puusepp, 2013
 */

#ifdef OSX
#include </usr/local/include/libwebsockets.h>
#else
#include <libwebsockets.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RX_BUF_SIZE 4096

char global_buf[LWS_SEND_BUFFER_PRE_PADDING + RX_BUF_SIZE + LWS_SEND_BUFFER_POST_PADDING];
size_t global_buf_len;

/* forward declare callback */
static int privac_callback(struct libwebsocket_context *, struct libwebsocket *,
    enum libwebsocket_callback_reasons, void *, void *, size_t);

/* libwebsockets protocols */
static struct libwebsocket_protocols protocols[] = {
  {
    "privac-protocol",
    privac_callback,
    0,
    RX_BUF_SIZE,
  },
  { NULL, NULL, 0, 0 }
};

/* libwebsockets callback*/
static int privac_callback(struct libwebsocket_context *context,
    struct libwebsocket *wsi,
    enum libwebsocket_callback_reasons reason, void *user,
    void *in, size_t len)
{
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED:
      break;
  case LWS_CALLBACK_RECEIVE:
    memcpy(global_buf, in, len);
    global_buf_len = len;
    libwebsocket_callback_on_writable_all_protocol(protocols);
    fprintf(stderr, "(debug) Received message\n");
    break;
  case LWS_CALLBACK_SERVER_WRITEABLE:
    // send current buffer, WHATEVER IT MIGHT BE
    // this may or may not be a good idea
    libwebsocket_write(wsi, (void*)global_buf, global_buf_len, LWS_WRITE_BINARY);
    fprintf(stderr, "(debug) Broadcast msg %.*s to client\n", (int) global_buf_len, global_buf);
    break;
  default:
    fprintf(stderr, "(warning) Received unhandled callback type: %d\n", reason);
    break;
  }
  
  return 0;
}


int main(int argc, const char *argv[]) {
  int port = 1337;

  struct libwebsocket_context *ctx;
  struct lws_context_creation_info info;

  memset(&info, 0, sizeof info);

  info.port = port;
  info.protocols = protocols;
  info.gid = -1;
  info.uid = -1;

  ctx = libwebsocket_create_context(&info);
  if (ctx == NULL) {
    fprintf(stderr, "init failed");
    return EXIT_FAILURE;
  }
  fprintf(stdout, "starting server...\n");

  while (1) {
    // poll sockets, call callbacks...
    libwebsocket_service(ctx, 50);
  }
  libwebsocket_context_destroy(ctx);

  return EXIT_SUCCESS;
}


#include </usr/local/include/libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RX_BUF_SIZE 4096

/* doubly linked list of websocket pointers */
// TODO this might actually not be neccessary at all :(
typedef struct socket_list {
	struct socket_list *next;
	struct socket_list *prev;
	struct libwebsocket *socket;
} SocketList;

/* push socket to end of list */
void push_socket(SocketList *list, struct libwebsocket *socket) {
	SocketList *iter = list;
	while (iter->next != NULL) {
		iter = iter->next;
	}
	SocketList *node = malloc(sizeof(SocketList));
	node->next = NULL;
	node->prev = iter;
	iter->next = node;
}

/* remove a given websocket from the list */
void remove_socket(SocketList *list, struct libwebsocket *socket) {
	SocketList *iter = list;
	while (iter->socket != socket) {
		if (iter->next == NULL) {
			return;
		}
		iter = iter->next;
	}
	// found socket at `iter`
	iter->prev->next = iter->next; 
	iter->next->prev = iter->prev;
	// iter->socket should be released by libwebsockets
	free(iter);
}

SocketList *sockets;

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

/* libwebsockets http callback*/
static int privac_callback(struct libwebsocket_context *context,
    struct libwebsocket *wsi,
    enum libwebsocket_callback_reasons reason, void *user,
    void *in, size_t len)
{
  char universal_response[] = "Hello, World!";
  fprintf(stderr, "%d\n", reason);
  switch (reason) {
	case LWS_CALLBACK_ESTABLISHED:
      break;
	case LWS_CALLBACK_RECEIVE:
	  global_buf = in;
	  // if its a join
	  // send it to everyone as a new join
	  // else
	  // send it to everyone as a message string
	  // either way
	  // send it to everyone really
	  break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
	  // send current buffer, WHATEVER IT MIGHT BE
	  // this may or may not be a good idea
      libwebsocket_write(wsi, (void*)universal_response, strlen(universal_response), LWS_WRITE_TEXT);
	  break;
	default:
	  fprintf(stderr, "(warning) Received unhandled callback type\n");
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
    // TODO
    libwebsocket_service(ctx, 50);
  }
  libwebsocket_context_destroy(ctx);

  return EXIT_SUCCESS;
}


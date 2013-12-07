#include <ncurses.h>
#include </usr/local/include/libwebsockets.h>
#include <stdlib.h>
#include <string.h>

#define QUEUE_BUF_SIZE 4096

struct libwebsocket_context *ctx;
struct libwebsocket *sck;
unsigned char buffer[LWS_SEND_BUFFER_PRE_PADDING + QUEUE_BUF_SIZE + LWS_SEND_BUFFER_POST_PADDING];
int blen, listening;

/* XOR str with given key */
void apply_key(char *str, int len, const char *key) {
  asm(""/* TODO ASM XOR CODE HERE */);
}

/* utility function for more flexible error logging */
int privac_err(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  return EXIT_FAILURE;
}

/* callback used by libwebsockets */
static int privac_callback(struct libwebsocket_context *context, struct libwebsocket *sock, 
    enum libwebsocket_callback_reasons reason,
    void *user, void *data, size_t len) 
{
  switch (reason) {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
      break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
      ctx = NULL, sck = NULL;
      return privac_err("Error creating connection");
    case LWS_CALLBACK_CLOSED:
      return privac_err("Connection closed");
    case LWS_CALLBACK_CLIENT_RECEIVE:
      // do something with the data
	  privac_err("did get data");
	  printw((char *)data, len);
      break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
      // write any queued data to the server
      //libwebsocket_write(sck, &buffer[LWS_SEND_BUFFER_PRE_PADDING], blen, LWS_WRITE_TEXT);
      listening = 1;
      break;
    default:
      privac_err("(warning) Received unhandled callback type");
      break;
  }
  return 0;
}

/* define protocol for libwebsocket connection with callback */
static struct libwebsocket_protocols protocols[] = {
  {
    "privac-protocol",
    privac_callback,
    0,
    QUEUE_BUF_SIZE,
  },
  { NULL, NULL, 0, 0 }
};

/* main application loop */
int loop() {
  int cond = 1; int blen = 0; 
  int y, x;
  unsigned char *p = buffer + LWS_SEND_BUFFER_PRE_PADDING;
  listening = 1;
  while (cond && listening) {
    int ch = getch();
    switch (ch)
    {
      case ERR: // timed out, do nothing
        break;
      case 27: // ESC or ALT. break loop
        return 0;
      case 127:
      case KEY_DC:
      case KEY_BACKSPACE:
        // backtrace
        if ((unsigned long)p >= (unsigned long)buffer + LWS_SEND_BUFFER_PRE_PADDING) {
          getyx(stdscr, y, x);
          move(y, x-1);
          delch();
          --p; --blen;
          *p = '\0';
        }
        break;
      case '\n':
        // send to server
        if (ctx && sck) {
          //libwebsocket_callback_on_writable(ctx, sck);
          listening = 0;
        }
        // clean buffer (should happen only once write is finished)
        printw(" || logged: [%s]\n", buffer + LWS_SEND_BUFFER_PRE_PADDING);
        memset(&buffer, 0, LWS_SEND_BUFFER_PRE_PADDING + QUEUE_BUF_SIZE);
        p = buffer + LWS_SEND_BUFFER_PRE_PADDING;
        blen = 0;
        break;
      default:
        // pop char to buffer and print it
        printw("%c", ch);
        *p = (char)ch;
        ++p; ++blen;
    }

    // if everything is fine, loop again
    libwebsocket_service(ctx, 10);  
  }
  return 0;
}

int main(int argc, const char * argv[]) {  

  /* initialise ncurses window */

  WINDOW *w = initscr();
  timeout(100);
  raw();
  noecho();

  /* initialise libwebsockets */

  const char *addr = "127.0.0.1";
  int port = 1337;
  struct lws_context_creation_info info;

  memset(&info, 0, sizeof info);
  memset(&buffer, '\0', LWS_SEND_BUFFER_PRE_PADDING + QUEUE_BUF_SIZE + LWS_SEND_BUFFER_POST_PADDING);

  info.port = CONTEXT_PORT_NO_LISTEN;
  info.protocols = protocols;
  info.gid = -1;
  info.uid = -1;
  
  ctx = libwebsocket_create_context(&info);
  if (ctx == NULL) return privac_err("Creating libwebsocket context failed");
  
  sck = libwebsocket_client_connect(ctx, addr, port, 0, "/", "a", "b", "privac-protocol", -1);
  if (sck == NULL) return privac_err("libwebsocket connect failed");

  /* start client */

  printw("----- welcome to privac -----\n");
  loop();
  refresh();
  endwin();
  return EXIT_SUCCESS;
}

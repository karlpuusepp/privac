#ifdef OSX
  #include </usr/local/include/libwebsockets.h>
#else
  #include <libwebsockets.h>
#endif
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define QUEUE_BUF_SIZE 4096

char *nick;
const char *cryptkey;
struct libwebsocket_context *ctx;
struct libwebsocket *sck;
unsigned char iobuffer[QUEUE_BUF_SIZE];
unsigned char sendbuffer[LWS_SEND_BUFFER_PRE_PADDING + 
                         QUEUE_BUF_SIZE + 
                         LWS_SEND_BUFFER_POST_PADDING];
size_t sendbuffer_len;
size_t blen;
int listening;
int pendingline = 1;

/* XOR str with given key */
void apply_key(char *str, int len) {
  asm(""/* TODO ASM XOR CODE HERE */);
}

/* utility function for more flexible error logging */
int privac_err(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  return EXIT_FAILURE;
}

/* print data to the pending line in window */
void print_msg(char *data, size_t len) {
  // move to last pending line
  int y, x;
  getyx(stdscr, y, x);
  if (pendingline >= getmaxy(stdscr)-2) {
    // clear history
    for (; pendingline > 1; --pendingline) {
      move(pendingline, 0);
      clrtoeol();
    }
  }
  move(pendingline, 0);
  time_t now = time(0);
  struct tm *timeinfo = localtime(&now);
  printw("[%02d:%02d] %.*s", timeinfo->tm_hour, timeinfo->tm_min, len, data);

  move(y, x);
  ++pendingline;
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
      print_msg(data, len);
      break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
      // write any queued data to the server
      libwebsocket_write(sck, sendbuffer+LWS_SEND_BUFFER_PRE_PADDING, sendbuffer_len, LWS_WRITE_TEXT);
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
  unsigned char *p = iobuffer + strlen(nick);
  listening = 1;

  getmaxyx(stdscr, y, x);
  move(y-1, 0);
  printw("[message] ");

  while (cond) {
    if (listening) {
      int ch = getch();
      switch (ch) {
        case ERR: // timed out, do nothing
          break;
        case KEY_RESIZE: // ignore tty resize
          break;
        case 27: // ESC or ALT. break loop
          return 0;
        case 127:
        case KEY_DC:
        case KEY_BACKSPACE:
          // backtrace
          if ((unsigned long) p >= (unsigned long) iobuffer + strlen(nick)) {
            getyx(stdscr, y, x);
            move(y, x-1);
            delch();
            --p; --blen;
            *p = '\0';
          }
          break;
        case '\n':
          // copy data to sendbuffer
          sendbuffer_len = blen + strlen(nick);
          memcpy(sendbuffer+LWS_SEND_BUFFER_PRE_PADDING, iobuffer, sendbuffer_len);
          // clean buffer
          p = iobuffer + strlen(nick);
          memset(p, 0, QUEUE_BUF_SIZE - strlen(nick));
          blen = 0;
          // send to server
          if (ctx && sck) {
            libwebsocket_callback_on_writable(ctx, sck);
            listening = 0;
          }
          // clear curses line
          move(getmaxy(stdscr)-1, 0);
          clrtoeol();
          printw("[message] ");
          break;
        default:
          // pop char to buffer and print it
          printw("%c", ch);
          *p = (char)ch;
          ++p; ++blen;
      }

      // if everything is fine, loop again
      libwebsocket_service(ctx, 0);  
    }
  }
  return 0;
}

int main(int argc, const char * argv[]) {  

  /* parse args */

  if (argc != 5) {
    privac_err("Usage: privac {server ip} {port} {nickname} {crypt key}");
    return EXIT_FAILURE;
  }
  const char *addr = argv[1];
  int port = atoi(argv[2]);
  nick = malloc(sizeof(char)*(strlen(argv[3]) + 2));
  sprintf(nick, "<%s> ", argv[3]);
  cryptkey = argv[4];

  if (port == 0) {
    privac_err("Invalid port number");
    return EXIT_FAILURE;
  }

  /* initialise ncurses window */

  WINDOW *w = initscr();
  timeout(100);
  raw();
  noecho();

  int wy, wx;
  getmaxyx(stdscr, wy, wx);
  printw("----- privac 0.1a - send message with ENTER - quit with ESC -----\n");

  /* initialise libwebsockets */

  struct lws_context_creation_info info;

  memset(&info, 0, sizeof info);
  memset(&iobuffer, '\0', QUEUE_BUF_SIZE);
  memcpy(&iobuffer, nick, strlen(nick));

  info.port = CONTEXT_PORT_NO_LISTEN;
  info.protocols = protocols;
  info.gid = -1;
  info.uid = -1;

  ctx = libwebsocket_create_context(&info);
  if (ctx == NULL) return privac_err("Creating libwebsocket context failed");

  sck = libwebsocket_client_connect(ctx, addr, port, 0, "/", "a", "b", "privac-protocol", -1);
  if (sck == NULL) return privac_err("libwebsocket connect failed");

  /* start client */

  loop();

  refresh();
  endwin();
  libwebsocket_context_destroy(ctx);
  free(nick);
  return EXIT_SUCCESS;
}

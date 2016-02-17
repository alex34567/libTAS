#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include "../shared/tasflags.h"
#include "keymapping.h"

#define MAGIC_NUMBER 42
#define SOCKET_FILENAME "/tmp/libTAS.socket"

void draw_cli(void);
int proceed_command(unsigned int command, int socket_fd);

struct TasFlags tasflags;

unsigned long int frame_counter = 0;

char keyboard_state[32];
KeySym hotkeys[HOTKEY_LEN];


static int MyErrorHandler(Display *display, XErrorEvent *theEvent)
{
    (void) fprintf(stderr,
		   "Ignoring Xlib error: error code %d request code %d\n",
		   theEvent->error_code,
		   theEvent->request_code);

    return 0;
}

int main(void)
{
    const struct sockaddr_un addr = { AF_UNIX, SOCKET_FILENAME };
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    Display *display;
    XEvent event;
    // Find the window which has the current keyboard focus
    Window win_focus;
    int revert;
    struct timespec tim;

    XSetErrorHandler(MyErrorHandler);

    /* open connection with the server */
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }


    printf("Connecting to libTAS...\n");

    if (connect(socket_fd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)))
    {
        printf("Couldn’t connect to socket.\n");
        return 1;
    }

    printf("Connected.\n");

    tim.tv_sec  = 1;
    tim.tv_nsec = 0L;

    nanosleep(&tim, NULL);

    XGetInputFocus(display, &win_focus, &revert);
    XSelectInput(display, win_focus, KeyPressMask);

    unsigned int command = 0;
    tasflags = DEFAULTFLAGS;
    default_hotkeys(hotkeys);

    while (1)
    {
        tim.tv_sec  = 0;
        tim.tv_nsec = 10000000L;

        nanosleep(&tim, NULL);

        XGetInputFocus(display, &win_focus, &revert);
        XSelectInput(display, win_focus, KeyPressMask);

        while( XPending( display ) > 0 ) {

            XNextEvent(display, &event);

            if (event.type == KeyPress)
            {
                KeyCode kc = ((XKeyPressedEvent*)&event)->keycode;
                KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);
                //s = XKeysymToString(ks);

                if (ks == hotkeys[HOTKEY_FRAMEADVANCE]){
                    XQueryKeymap(display, keyboard_state);
                    command = 8;
                }
                if (ks == hotkeys[HOTKEY_PLAYPAUSE]){
                    tasflags.running = !tasflags.running;
                }
            }
        }

        if (tasflags.running)
            XQueryKeymap(display, keyboard_state);

        if (proceed_command(command, socket_fd))
            break;

        if (tasflags.running)
            command = 8;
        else
            command = 0;
    }

    close(socket_fd);
    return 0;
}

void draw_cli(void)
{
    printf("%s      Speed divisor: %u     Frame counter: %lu\n\n",
           tasflags.running ? "\033[7m[RUNNING ]\033[0m" : "[ PAUSED ]", tasflags.speed_divisor, frame_counter);
    printf("Available commands:\n\n");
    printf("1 - Toggle UP.\n");
    printf("2 - Toggle DOWN.\n");
    printf("3 - Toggle LEFT.\n");
    printf("4 - Toggle RIGHT.\n\n");
    printf("5 - Toggle SPACE.\n");
    printf("6 - Toggle SHIFT.\n\n");
    printf("7 - Toggle PAUSE/RUNNING.\n");
    printf("8 - Advance 1 frame.\n");
    printf("9 - Set speed divisor.\n\n");
    printf("10 - Save inputs.\n");
    printf("11 - Load inputs.\n\n");
    printf("0 - Exit.\n\n");
}

int proceed_command(unsigned int command, int socket_fd)
{
    if (!command)
        return 0;

    if (command > 11)
    {
        printf("This command does not exist.\n");
        return 0;
    }

    send(socket_fd, &command, sizeof(unsigned int), 0);

    char filename_buffer[1024];

    switch (command)
    {
    case 7:
        tasflags.running = !tasflags.running;
        break;

    case 8:
        send(socket_fd, keyboard_state, 32 * sizeof(char), 0);
        recv(socket_fd, &frame_counter, sizeof(unsigned long), 0);
        break;

    case 9:
        tasflags.speed_divisor = 0;
        do
        {
            printf("Enter non-null speed divisor factor: ");
            scanf("%d", &(tasflags.speed_divisor));
        }
        while (!tasflags.speed_divisor);

        send(socket_fd, &(tasflags.speed_divisor), sizeof(unsigned int), 0);
        break;

    case 10:
        printf("Enter filename to save inputs in: ");
        scanf("%s", filename_buffer);
        send(socket_fd, filename_buffer, 1024, 0);

        unsigned long first_frame = 0;
        do
            printf("Enter first frame to record: ");
        while (!scanf("%lu", &first_frame));

        send(socket_fd, &first_frame, sizeof(unsigned long), 0);
        break;

    case 11:
        printf("Enter filename from which to load inputs: ");
        scanf("%s", filename_buffer);
        send(socket_fd, filename_buffer, 1024, 0);

        /* Check if loading can be done. */
        unsigned char answer;
        recv(socket_fd, &answer, sizeof(unsigned char), 0);
        if (!answer)
        {
            printf("libTAS couldn’t load inputs.\n");
            break;
        }

        /* Update inputs. */
        /*recv(socket_fd, &answer, sizeof(unsigned char), 0);
        for (unsigned int i = 0; i < KEYS_NUMBER; ++i)
        {
            keys[i] = answer & 0x1;
            answer >>= 1;
        }*/

    default:;
    }

    //command = 0;

    //send(socket_fd, &command, sizeof(unsigned int), 0);

    return 0;
}

/*****************************************************************************************************************************************************
 *
 * @file        andropad.c
 * @author      kaharman
 * @date        Aug 27, 2015
 *
 * @brief       Andropad main file
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ****************************************************************************************************************************************************/

/*****************************************************************************************************************************************************
 * INCLUDES
 ****************************************************************************************************************************************************/
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

/*****************************************************************************************************************************************************
 * DEFINES
 ****************************************************************************************************************************************************/
#ifndef DIR_RUN
#define DIR_RUN             "/var/run/"
#endif

#ifndef BTN_DPAD_UP
#define BTN_DPAD_UP         0x220
#endif
#ifndef BTN_DPAD_DOWN
#define BTN_DPAD_DOWN       0x221
#endif
#ifndef BTN_DPAD_LEFT
#define BTN_DPAD_LEFT       0x222
#endif
#ifndef BTN_DPAD_RIGHT
#define BTN_DPAD_RIGHT      0x223
#endif
#ifndef BTN_NORTH
#define BTN_NORTH           0x133
#endif
#ifndef BTN_SOUTH
#define BTN_SOUTH           0x130
#endif
#ifndef BTN_WEST
#define BTN_WEST            0x134
#endif
#ifndef BTN_EAST
#define BTN_EAST            0x131
#endif

#define BTN_A               0x12D
#define BTN_B               0x12E
#define BTN_C               0x12F
#define BTN_X               0x130
#define BTN_Y               0x131
#define BTN_Z               0x132
#define BTN_TL              0x133
#define BTN_TR              0x134
#define BTN_TL2             0x135
#define BTN_TR2             0x136
#define BTN_SELECT          0x137
#define BTN_START           0x138
#define BTN_MODE            0x139
#define BTN_THUMBL          0x13A
#define BTN_THUMBR          0x13B

/*****************************************************************************************************************************************************
 * GLOBAL VARIABLES
 ****************************************************************************************************************************************************/
static sem_t semKill;
struct input_event ev;
int fd;

/*****************************************************************************************************************************************************
 * FUNCTIONS PROTOTYPE
 ****************************************************************************************************************************************************/
static int requestCallback(void * cls,
                           struct MHD_Connection *connection,
                           const char *url,
                           const char *method,
                           const char *version,
                           const char *uploadData,
                           size_t * uploadDataSize,
                           void **ptr);
static int parseButton(const char *url,
                       int *button,
                       int *value);
static void signalHandler(int sig,
                          siginfo_t *siginfo,
                          void *context);

/*****************************************************************************************************************************************************
 * FUNCTIONS
 ****************************************************************************************************************************************************/
int main(int argc,
         char ** argv)
{
    struct sigaction act;
    struct uinput_user_dev uidev;
    struct MHD_Daemon *d;
    FILE *pf;
    char spid[10];

    if (argc != 2)
    {
        printf("%s PORT\n", argv[0]);
        return 1;
    }

    /* Signal */
    memset(&act, '\0', sizeof(act));
    sem_init(&semKill, 0, 0);
    act.sa_sigaction = &signalHandler;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGTERM, &act, NULL) < 0)
    {
        return -1;
    }

    if (sigaction(SIGQUIT, &act, NULL) < 0)
    {
        return -1;
    }

    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        return -1;
    }

    /* Check & save pid */
    pf = fopen(DIR_RUN "andropad.pid", "rb");
    if (pf)
    {
        memset(spid, 0, sizeof(spid));
        if (NULL == fgets(spid, sizeof(spid) - 1, pf))
        {
            fprintf(stderr, "Error fgets\n");
            //return -1;
        }
        fclose(pf);
        if (kill(strtol(spid, NULL, 10), 0) == 0)
        {
            fprintf(stderr, "andropad already exists, exit...\n");
            return -1;
        }
        else
        {
            remove(DIR_RUN "andropad.pid");
        }
    }
    pf = fopen(DIR_RUN "andropad.pid", "wb");
    if (pf)
    {
        fprintf(pf, "%d", getpid());
        fclose(pf);
    }

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        fprintf(stderr, "Cannot find uinput\n");
        return -1;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_ABS) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_ABSBIT, ABS_X) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_ABSBIT, ABS_Y) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_A) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_B) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_C) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_X) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_Y) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_Z) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TL) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TR) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TL2) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_TR2) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_SELECT) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_START) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_MODE) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_THUMBL) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }
    if (ioctl(fd, UI_SET_KEYBIT, BTN_THUMBR) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Andropad");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x7777;
    uidev.id.product = 0x7777;
    uidev.id.version = 1;
    uidev.absmin[ABS_X] = -1;
    uidev.absmax[ABS_X] = 1;
    uidev.absmin[ABS_Y] = -1;
    uidev.absmax[ABS_Y] = 1;

    if (write(fd, &uidev, sizeof(uidev)) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }

    sleep(2);

    d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                         atoi(argv[1]),
                         NULL,
                         NULL,
                         &requestCallback,
                         NULL,
                         MHD_OPTION_END);
    if (d == NULL)
    {
        close(fd);
        return 1;
    }

    /* Main loop */
    while(1)
    {
        if(!sem_wait(&semKill))
        {
            break;
        }
    }

    MHD_stop_daemon(d);

    if (ioctl(fd, UI_DEV_DESTROY) < 0)
    {
        fprintf(stderr, "Error ioctl\n");
        close(fd);
        return -1;
    }

    close(fd);

    /* Remove pid file */
    pf = fopen(DIR_RUN "andropad.pid", "rb");
    if (pf)
    {
        fclose(pf);
        remove(DIR_RUN "andropad.pid");
    }

    return 0;
}

static int requestCallback(void * cls,
                           struct MHD_Connection *connection,
                           const char *url,
                           const char *method,
                           const char *version,
                           const char *uploadData,
                           size_t *uploadDataSize,
                           void **ptr)
{
    static int dummy;
    const char * page = cls;
    struct MHD_Response *response = NULL;
    int ret;
    int button = 0;
    int value = 1;
    char resp[100];

    if (0 != strcmp(method, "GET"))
    {
        return MHD_NO; /* unexpected method */
    }

    if (&dummy != *ptr)
    {
        /* The first time only the headers are valid,
         do not respond in the first round... */
        *ptr = &dummy;
        return MHD_YES;
    }

    if (0 != *uploadDataSize)
    {
        return MHD_NO; /* upload data in a GET!? */
    }

    if (parseButton(url, &button, &value) == 0)
    {
        switch (button)
        {
        case BTN_DPAD_UP:
            sprintf(resp, "Up button");
            break;
        case BTN_DPAD_DOWN:
            sprintf(resp, "Down button");
            break;
        case BTN_DPAD_LEFT:
            sprintf(resp, "Left button");
            break;
        case BTN_DPAD_RIGHT:
            sprintf(resp, "Right button");
            break;
        case BTN_A:
            sprintf(resp, "A button");
            break;
        case BTN_B:
            sprintf(resp, "B button");
            break;
        case BTN_C:
            sprintf(resp, "C button");
            break;
        case BTN_X:
            sprintf(resp, "X button");
            break;
        case BTN_Y:
            sprintf(resp, "Y button");
            break;
        case BTN_Z:
            sprintf(resp, "Z button");
            break;
        case BTN_TL:
            sprintf(resp, "TL button");
            break;
        case BTN_TR:
            sprintf(resp, "TR button");
            break;
        case BTN_TL2:
            sprintf(resp, "TL2 button");
            break;
        case BTN_TR2:
            sprintf(resp, "TR2 button");
            break;
        case BTN_SELECT:
            sprintf(resp, "Select button");
            break;
        case BTN_START:
            sprintf(resp, "Start button");
            break;
        default:
            sprintf(resp, "Wrong button!");
        }

        memset(&ev, 0, sizeof(struct input_event));
        if (button < BTN_DPAD_UP)
        {
            ev.type = EV_KEY;
            ev.code = button;
            ev.value = value;
        }
        else
        {
            ev.type = EV_ABS;
            switch (button)
            {
            case BTN_DPAD_UP:
                ev.code = ABS_Y;
                if (value)
                {
                    ev.value = 1;
                }
                else
                {
                    ev.value = 0;
                }
                break;
            case BTN_DPAD_DOWN:
                ev.code = ABS_Y;
                if (value)
                {
                    ev.value = -1;
                }
                else
                {
                    ev.value = 0;
                }
                break;
            case BTN_DPAD_LEFT:
                ev.code = ABS_X;
                if (value)
                {
                    ev.value = -1;
                }
                else
                {
                    ev.value = 0;
                }
                break;
            case BTN_DPAD_RIGHT:
                ev.code = ABS_X;
                if (value)
                {
                    ev.value = 1;
                }
                else
                {
                    ev.value = 0;
                }
                break;
            }
        }
        if (write(fd, &ev, sizeof(struct input_event)) < 0)
        {
            return MHD_NO;
        }

        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_SYN;
        ev.code = 0;
        ev.value = 0;
        if (write(fd, &ev, sizeof(struct input_event)) < 0)
        {
            return MHD_NO;
        }
    }

    *ptr = NULL; /* clear context pointer */
    response = MHD_create_response_from_data(strlen(resp), (void *) resp, MHD_NO, MHD_NO);
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

static int parseButton(const char *url,
                       int *button,
                       int *value)
{
    char *pch = NULL;
    int ret = -1;

    *button = 0;
    *value = 1;

    pch = strtok((char *) url, "/");
    if (pch == NULL)
    {
        return ret;
    }

    pch = strtok(NULL, "/");
    if (pch == NULL)
    {
        return ret;
    }

    if (strcmp(pch, "up") == 0)
    {
        *button = BTN_DPAD_UP;
    }
    else if (strcmp(pch, "down") == 0)
    {
        *button = BTN_DPAD_DOWN;
    }
    else if (strcmp(pch, "left") == 0)
    {
        *button = BTN_DPAD_LEFT;
    }
    else if (strcmp(pch, "right") == 0)
    {
        *button = BTN_DPAD_RIGHT;
    }
    else if (strcmp(pch, "a") == 0)
    {
        *button = BTN_A;
    }
    else if (strcmp(pch, "b") == 0)
    {
        *button = BTN_B;
    }
    else if (strcmp(pch, "c") == 0)
    {
        *button = BTN_C;
    }
    else if (strcmp(pch, "x") == 0)
    {
        *button = BTN_X;
    }
    else if (strcmp(pch, "y") == 0)
    {
        *button = BTN_Y;
    }
    else if (strcmp(pch, "z") == 0)
    {
        *button = BTN_Z;
    }
    else if (strcmp(pch, "tl") == 0)
    {
        *button = BTN_TL;
    }
    else if (strcmp(pch, "tr") == 0)
    {
        *button = BTN_TR;
    }
    else if (strcmp(pch, "tl2") == 0)
    {
        *button = BTN_TL2;
    }
    else if (strcmp(pch, "tr2") == 0)
    {
        *button = BTN_TR2;
    }
    else if (strcmp(pch, "select") == 0)
    {
        *button = BTN_SELECT;
    }
    else if (strcmp(pch, "start") == 0)
    {
        *button = BTN_START;
    }
    else
    {
        return ret;
    }
    ret = 0;

    pch = strtok(NULL, "/");
    if (pch == NULL)
    {
        return ret;
    }

    if (atoi(pch) == 0)
    {
        *value = 0;
    }

    return ret;
}

static void signalHandler(int sig,
                          siginfo_t *siginfo,
                          void *context)
{
    sem_post(&semKill);
}

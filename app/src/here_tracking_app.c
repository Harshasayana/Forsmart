/**************************************************************************************************
 * Copyright (C) 2017 HERE Europe B.V.                                                            *
 * All rights reserved.                                                                           *
 *                                                                                                *
 * MIT License                                                                                    *
 * Permission is hereby granted, free of charge, to any person obtaining a copy                   *
 * of this software and associated documentation files (the "Software"), to deal                  *
 * in the Software without restriction, including without limitation the rights                   *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                      *
 * copies of the Software, and to permit persons to whom the Software is                          *
 * furnished to do so, subject to the following conditions:                                       *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all                 *
 * copies or substantial portions of the Software.                                                *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                     *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                       *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE                    *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                         *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,                  *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE                  *
 * SOFTWARE.                                                                                      *
 **************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "here_tracking.h"
#include "here_tracking_time.h"

/**************************************************************************************************/

#define HERE_TRACKING_APP_DATA_BUFFER_SIZE 4096

typedef struct
{
    here_tracking_client client;
    char data_buffer[HERE_TRACKING_APP_DATA_BUFFER_SIZE];
} here_tracking_app;

static here_tracking_app app;

/**************************************************************************************************/

static void here_tracking_app_data_cb(here_tracking_error err,
                                      const char* data,
                                      uint32_t data_size,
                                      void* user_data)
{
    return;
}

/**************************************************************************************************/

int main(int argc, char** argv)
{
    here_tracking_error err;

    if(argc >= 4)
    {
        char* device_id = argv[1];
        char* device_secret = argv[2];
        char* base_url = argv[3];
        uint8_t samples_to_send = 10; /* Default to 10 samples. */
        uint8_t sample_interval = 1; /* Default to 1 second interval */

        if(argc >= 5)
        {
            int samples = atoi(argv[4]);

            /* Check for max limit and ignore <= 0 */
            if(samples > UINT8_MAX)
            {
                samples_to_send = UINT8_MAX;
            }
            else if(samples > 0)
            {
                samples_to_send = samples;
            }
        }

        if(argc >= 6)
        {
            int interval = atoi(argv[5]);

            /* Check for max limit and ignore <= 0 */
            if(interval > UINT8_MAX)
            {
                sample_interval = UINT8_MAX;
            }
            else if(interval > 0)
            {
                sample_interval = interval;
            }
        }

        err = here_tracking_init(&app.client, device_id, device_secret, base_url);

        if(err == HERE_TRACKING_OK)
        {
            err = here_tracking_set_recv_data_cb(&app.client, here_tracking_app_data_cb, &app);
        }

        while(samples_to_send > 0)
        {
            uint32_t ts;

            memset(app.data_buffer, 0x00, HERE_TRACKING_APP_DATA_BUFFER_SIZE);
            here_tracking_get_unixtime(&ts);
            snprintf(app.data_buffer,
                     HERE_TRACKING_APP_DATA_BUFFER_SIZE,
                     "[{\"payload\":{\"clientName\":\"here-tracking-c\"},\"timestamp\":%llu}]",
                     ((unsigned long long)ts) * 1000);
            err = here_tracking_send(&app.client,
                                     app.data_buffer,
                                     strlen(app.data_buffer),
                                     HERE_TRACKING_APP_DATA_BUFFER_SIZE);

            if(err != HERE_TRACKING_OK)
            {
                break;
            }

            sleep(sample_interval);
            samples_to_send--;
        }

        here_tracking_free(&app.client);
    }
    else
    {
        fprintf(stderr,
                "Usage: ./here_tracking_app device_id device_secret base_url "
                "[sample_count] [sample_interval]\n");
        err = HERE_TRACKING_ERROR_INVALID_INPUT;
    }

    return (err == HERE_TRACKING_OK) ? 0 : -1;
}

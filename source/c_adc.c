/*
Copyright (c) 2013 Adafruit
Author: Justin Cooper

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "c_adc.h"
#include "common.h"

#ifdef BBBVERSION41
    char adc_prefix_dir[49];
#else
    char adc_prefix_dir[40];
#endif

int adc_initialized = 0;

int initialize_adc(void)
{
#ifdef BBBVERSION41
    char test_path[49];
#else
    char test_path[40];
#endif
    FILE *fh;
    if (adc_initialized) {
        return 1;
    }

#ifdef BBBVERSION41
    if (load_device_tree("BB-ADC")) {
        strncat(adc_prefix_dir, "/sys/bus/iio/devices/iio:device0/in_voltage", sizeof(adc_prefix_dir));
        snprintf(test_path, sizeof(test_path), "%s%d_raw", adc_prefix_dir, 1);
        fh = fopen(test_path, "r");

        if (!fh) {
            puts("wiiii");
            return 0;
        }
        fclose(fh);

        adc_initialized = 1;
        return 1;
    }
#else
    if (load_device_tree("cape-bone-iio")) {
        build_path("/sys/devices", "ocp.", ocp_dir, sizeof(ocp_dir));
        build_path(ocp_dir, "helper.", adc_prefix_dir, sizeof(adc_prefix_dir));
        strncat(adc_prefix_dir, "/AIN", sizeof(adc_prefix_dir));
        snprintf(test_path, sizeof(test_path), "%s%d", adc_prefix_dir, 0);
        fh = fopen(test_path, "r");

        if (!fh) {
            return 0;
        }
        fclose(fh);

        adc_initialized = 1;
        return 1;
    }
#endif

    return 0;
}

int read_value(unsigned int ain, float *value)
{
    FILE * fh;
#ifdef BBBVERSION41
    char ain_path[49];
    snprintf(ain_path, sizeof(ain_path), "%s%d_raw", adc_prefix_dir, ain);
#else
    char ain_path[40];
    snprintf(ain_path, sizeof(ain_path), "%s%d", adc_prefix_dir, ain);
#endif

    int err, try_count=0;
    int read_successful;
    
    read_successful = 0;

    // Workaround to AIN bug where reading from more than one AIN would cause access failures
    while (!read_successful && try_count < 3)
    {
        fh = fopen(ain_path, "r");

        // Likely a bad path to the ocp device driver 
        if (!fh) {
            return -1;
        }

        fseek(fh, 0, SEEK_SET);
        err = fscanf(fh, "%f", value);

        if (err != EOF) read_successful = 1;
        fclose(fh);

        try_count++;
    }

    if (read_successful) return 1;

    // Fall through and fail
    return -1;
}

int adc_setup()
{
    return initialize_adc();
}

void adc_cleanup(void)
{
#ifdef BBBVERSION41
    unload_device_tree("BB-ADC");
#else
    unload_device_tree("cape-bone-iio");
#endif
}

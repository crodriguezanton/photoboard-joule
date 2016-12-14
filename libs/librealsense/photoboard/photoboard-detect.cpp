// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#include <librealsense/rs.hpp>
#include <cstdio>
#include <iostream>
#include <fstream>
using namespace std;
#include <string>

int main() try
{
    // Create a context object. This object owns the handles to all connected realsense devices.
    rs::context ctx;
    printf("There are %d connected RealSense devices.\n", ctx.get_device_count());
    if(ctx.get_device_count() == 0) return EXIT_FAILURE;

    // This tutorial will access only a single device, but it is trivial to extend to multiple devices
    rs::device * dev = ctx.get_device(0);
    printf("\nUsing device 0, an %s\n", dev->get_name());
    printf("    Serial number: %s\n", dev->get_serial());
    printf("    Firmware version: %s\n", dev->get_firmware_version());

    // Configure depth to run at VGA resolution at 30 frames per second
    dev->enable_stream(rs::stream::depth, 640, 480, rs::format::z16, 30);

    dev->start();

    // Determine depth value corresponding to one meter
    const uint16_t start_zone = static_cast<uint16_t>(1.0f / dev->get_depth_scale());
    const uint16_t end_zone = static_cast<uint16_t>(3.0f / dev->get_depth_scale());

    ofstream file;
    file.open("photoboard.txt");

    int i = 0;
    bool changed = true;

    for (int i = 0; i < 30; ++i) dev->wait_for_frames();

    //do{
        // This call waits until a new coherent set of frames is available on a device
        // Calls to get_frame_data(...) and get_frame_timestamp(...) on a device will return stable values until wait_for_frames(...) is called
        dev->wait_for_frames();

        // Retrieve depth data, which was previously configured as a 640 x 480 image of 16-bit depth values
        const uint16_t * depth_frame = reinterpret_cast<const uint16_t *>(dev->get_frame_data(rs::stream::depth));

        // Print a simple text-based representation of the image, by breaking it into 10x20 pixel regions and and approximating the coverage of pixels within one meter
        char buffer[(640/10+1)*(480/20)+1];
        char buffer2[(640/80+1)*(480/80)+1];
        char buffer3[(640/80+1)*(480/80)+1];
        char * out = buffer;
        char * out2 = buffer2;
        int coverage[64] = {};
        int area_coverage[6][8] = {};

        for(int y=0; y<480; ++y)
        {
            for(int x=0; x<640; ++x)
            {
                int depth = *depth_frame++;
                if(depth > 0 && depth < end_zone && depth > start_zone) {
                  ++coverage[x/10];
                  ++area_coverage[y/80][x/80];
                }
            }

            if(y%20 == 19)
            {
                for(int & c : coverage)
                {
                    *out++ = " .:nhBXWW"[c/25];
                    c = 0;
                }
                *out++ = '\n';
            }

            if(y%80 == 79) {

              for(int & c : area_coverage[y/80])
              {
                if (c > 100){
                  *out2++ = 'W';
                  file << "W";
                } else {
                  *out2++ = '.';
                  file << ".";
                }

              }
              *out2++ = '\n';
              file << '\n';
            }
        }
        *out++ = 0;
        *out2++ = 0;
        ++i;
        std::system("clear");
        printf("\n%s", buffer);
        printf("\n%s", buffer2);

        if (changed){
          printf("\nPhoto taken\n");
        }

    //} while (i<20);

    file.close();

    return EXIT_SUCCESS;
}
catch(const rs::error & e)
{
    // Method calls against librealsense objects may throw exceptions of type rs::error
    printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), e.get_failed_args().c_str());
    printf("    %s\n", e.what());
    return EXIT_FAILURE;
}

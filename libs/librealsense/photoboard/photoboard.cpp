// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

/////////////////////////////////////////////////////
// librealsense tutorial #1 - Accessing depth data //
/////////////////////////////////////////////////////

// First include the librealsense C++ header file
#include <librealsense/rs.hpp>
#include <cstdio>
#include <stdint.h>
#include <vector>
#include <map>
#include <limits>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"

void normalize_depth_to_rgb(uint8_t rgb_image[], const uint16_t depth_image[], int width, int height)
{
    for (int i = 0; i < width * height; ++i)
    {
        if (auto d = depth_image[i])
        {
            uint8_t v = d * 255 / std::numeric_limits<uint16_t>::max();
            rgb_image[i*3 + 0] = 255 - v;
            rgb_image[i*3 + 1] = 255 - v;
            rgb_image[i*3 + 2] = 255 - v;
        }
        else
        {
            rgb_image[i*3 + 0] = 0;
            rgb_image[i*3 + 1] = 0;
            rgb_image[i*3 + 2] = 0;
        }
    }
}

std::map<rs::stream,int> components_map =
{
    { rs::stream::depth,     3  },      // RGB
    { rs::stream::color,     3  },
    { rs::stream::infrared , 1  },      // Monochromatic
    { rs::stream::infrared2, 1  },
    { rs::stream::fisheye,   1  }
};

struct stream_record
{
    stream_record(void): frame_data(nullptr) {};
    stream_record(rs::stream value): stream(value), frame_data(nullptr) {};
    ~stream_record() { frame_data = nullptr;}
    rs::stream          stream;
    rs::intrinsics      intrinsics;
    unsigned char   *   frame_data;
};

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
    const uint16_t start_zone = static_cast<uint16_t>(1.5f / dev->get_depth_scale());
    const uint16_t end_zone = static_cast<uint16_t>(2.0f / dev->get_depth_scale());

    bool changed = true;

    std::vector<stream_record> supported_streams;

    for (int i=(int)rs::capabilities::depth; i <=(int)rs::capabilities::fish_eye; i++)
        if (dev->supports((rs::capabilities)i))
            supported_streams.push_back(stream_record((rs::stream)i));

    for (auto & stream_record : supported_streams)
        dev->enable_stream(stream_record.stream, rs::preset::best_quality);


    /* retrieve actual frame size for each enabled stream*/
    for (auto & stream_record : supported_streams)
        stream_record.intrinsics = dev->get_stream_intrinsics(stream_record.stream);


    for (int i = 0; i < 30; ++i) dev->wait_for_frames();

    while(true)
    {
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
        char * out3 = buffer3;
        int coverage[64] = {};
        int area_coverage[8] = {};
        for(int y=0; y<480; ++y)
        {
            for(int x=0; x<640; ++x)
            {
                int depth = *depth_frame++;
                if(depth > 0 && depth < end_zone && depth > start_zone) {
                  ++coverage[x/10];
                  ++area_coverage[x/80];
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
              for(int & c : area_coverage)
              {
                if (c > 100){
                  *out2++ = 'W';
                  if(*out3 == '.') *out3++ = '.';
                  else *out3++ = 'W';
                } else {
                  if(*out3 == 'W') changed = true;
                  *out2++ = '.';
                  *out3++ = '.';
                }

              }
              *out2++ = '\n';
              *out3++ = '\n';
            }
        }
        *out++ = 0;
        *out2++ = 0;
        *out3++ = 0;
        ++i;
        std::system("clear");
        printf("\n%s", buffer);
        printf("\n%s", buffer2);
        printf("\n\n%s", buffer3);

        if (changed){

          /* Retrieve data from all the enabled streams */
          for (auto & stream_record : supported_streams)
              stream_record.frame_data = const_cast<uint8_t *>((const uint8_t*)dev->get_frame_data(stream_record.stream));

          /* Transform Depth range map into color map */
          stream_record depth = supported_streams[(int)rs::stream::depth];
          std::vector<uint8_t> coloredDepth(depth.intrinsics.width * depth.intrinsics.height * components_map[depth.stream]);

          /* Encode depth data into color image */
          normalize_depth_to_rgb(coloredDepth.data(), (const uint16_t *)depth.frame_data, depth.intrinsics.width, depth.intrinsics.height);

          /* Update captured data */
          supported_streams[(int)rs::stream::depth].frame_data = coloredDepth.data();

          /* Store captured frames into current directory */
          for (auto & captured : supported_streams)
          {
              std::stringstream ss;
              ss << "photoboard-image-" << captured.stream << ".png";

              std::cout << "Writing " << ss.str().data() << ", " << captured.intrinsics.width << " x " << captured.intrinsics.height << " pixels"   << std::endl;

              stbi_write_png(ss.str().data(),
                  captured.intrinsics.width,captured.intrinsics.height,
                  components_map[captured.stream],
                  captured.frame_data,
                  captured.intrinsics.width * components_map[captured.stream] );
          }

          printf("wrote frames to current working directory.\n");

          changed = false;
        }

    }

    return EXIT_SUCCESS;
}
catch(const rs::error & e)
{
    // Method calls against librealsense objects may throw exceptions of type rs::error
    printf("rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), e.get_failed_args().c_str());
    printf("    %s\n", e.what());
    return EXIT_FAILURE;
}

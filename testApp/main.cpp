/**
 * @author  Akram Hussein
 * Copyright (C) 2012  Akram Hussein
 *                     Imperial College London
 **/

#include <sys/stat.h>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] ){
    
    // Setup Video Source
    FirewireVideo video = FirewireVideo(0, 
                                        DC1394_VIDEO_MODE_640x480_RGB8, 
                                        DC1394_FRAMERATE_30, 
                                        DC1394_ISO_SPEED_400
                                        );
     // Simplified constructor

    //ExifData exifData;

        
    
    return 0;

}
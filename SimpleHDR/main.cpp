/**
 * @author  Steven Lovegrove
 * Copyright (C) 2010  Steven Lovegrove
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>
#include <dc1394/dc1394.h>
#include <boost/thread/thread.hpp>

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] )
{

    // Setup Video Source
    // FirewireVideo video = FirewireVideo(); // generic constructor
    
    // For libdc1394 types - see http://damien.douxchamps.net/ieee1394/libdc1394/api/types/
    FirewireVideo video =  FirewireVideo();
//                                            0,                               //device id
//                                            DC1394_VIDEO_MODE_640x480_RGB8,  // video mode 
//                                            DC1394_FRAMERATE_30,             // frame rate
//                                            DC1394_ISO_SPEED_400,            // iso speed
//                                            10                               // no. of dma buffers
//                                         );
    
    video.SetAutoShutterTime();
    video.SetAutoGain();
        
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    dc1394video_frame_t* empty_frame = NULL; // need to be here - perhaps in firewire
    
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    // Print out camera report
    video.PrintReport();
    
    video.SetShutterTimeManual();
    
    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* img = new unsigned char[video.SizeBytes()];
    bool over_exposed = true;
    
    for(int frame_number=0; !pangolin::ShouldQuit(); ++frame_number)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // Grabs frame and also saves ppm
        //video.SaveFrame(frame_number, empty_frame, img, true); 
        
        
        if (over_exposed){
            cout << "Over" << endl;
            //video.SetShutterTimeQuant(250);
            video.SetExposure(2.00);
            //video.SetShutterTime(0.000100);
            over_exposed = false;
            //cout << video.GetShutterTime() << endl;
            cout << video.GetExposure() << endl;
        }
        else if (!over_exposed){
            cout << "Under" << endl;
            //video.SetShutterTimeQuant(700);
            //video.SetShutterTime(0.020000);
            video.SetExposure(0.00);
            over_exposed = true;
            //cout << video.GetShutterTime() << endl;
            cout << video.GetExposure() << endl;
        }
        
        // wait 1/30th of a second to sync change in settings and display
        //boost::this_thread::sleep(boost::posix_time::seconds(1/30));
        //sleep(1/30);
        
        // Grab frame without saving
        video.SaveOneShot(frame_number, empty_frame, img, true);

        texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);

        // Activate video viewport and render texture
        vVideo.Activate();
        texVideo.RenderToViewportFlipY();

        // Swap back buffer with front and process window events via GLUT
        pangolin::FinishGlutFrame();
    }

    delete[] img;
    
    return 0;
}




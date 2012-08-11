/**
 * @author  Akram Hussein
 * Copyright (C) 2012  Akram Hussein
 *                     Imperial College London
 **/

#include <sys/stat.h>
#include <iostream>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

#include <boost/thread.hpp>  

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] )
{
    /*-----------------------------------------------------------------------
     *  SETUP SOURCE
     *-----------------------------------------------------------------------*/    
    
    FirewireVideo video = FirewireVideo(0,
                                        DC1394_VIDEO_MODE_640x480_RGB8,
                                        DC1394_FRAMERATE_30,
                                        DC1394_ISO_SPEED_400,
                                        100
                                        );
    video.SetAllFeaturesAuto();
    video.CreateShutterMaps();
    video.SetMetaDataFlags( META_ALL );
    video.SetMultiShotOn(100);
    //video.SetMultiShotOff();
    //video.StopForOneShot();
    video.SetHDRRegister(true);
    
    uint32_t s0 = video.GetShutterMapQuant(0.00450313);
    uint32_t s1 = video.GetShutterMapQuant(0.00200000);
    uint32_t s2 = video.GetShutterMapQuant(0.00450313);
    uint32_t s3 = video.GetShutterMapQuant(0.00200000);
    
    video.SetHDRShutterFlags(s0,s1,s2,s3);
    
    unsigned char* img = new unsigned char[video.SizeBytes()];
    
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();
    
    /*-----------------------------------------------------------------------
     *  GUI
     *-----------------------------------------------------------------------*/    
    
    // Create Glut window
    const int panel_width = 200;
    pangolin::CreateGlutWindowAndBind("SimpleHDR",w + panel_width,h);
    
    // Create viewport for video with fixed aspect
    View& d_panel = pangolin::CreatePanel("ui.")
    .SetBounds(0.0, 1.0, 0.0, Attach::Pix(panel_width));
    
    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video")
    .SetBounds(0.0, 1.0, Attach::Pix(panel_width), 1.0)
    .SetAspect((float)w/h);
    
    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    /*-----------------------------------------------------------------------
     *  CAPTURE
     *-----------------------------------------------------------------------*/     

    cout << "Camera transmission starting..." << endl;
    uint32_t ss0,ss1,ss2,ss3;
    
    // loop until quit
    for(int frame_number=0; !pangolin::ShouldQuit(); ++frame_number)
    {     
        //Screen refresh -- thread?
        if(pangolin::HasResized())
            DisplayBase().ActivateScissorAndClear();
        
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        video.GrabNewest(img);
        video.GetHDRShutterFlags(ss0, ss1, ss2, ss3);
        cout << "SHUTTER: " << video.ReadShutter(img) << endl;
         cout << "TIME STAMP: " << video.ReadTimeStamp(img) << endl;
        /*
        bitset<32> sh0(ss0);
        bitset<32> sh1(ss1);
        bitset<32> sh2(ss2);
        bitset<32> sh3(ss3);   
        
        cout << sh0 << endl;
        cout << sh1 << endl;
        cout << sh2 << endl;
        cout << sh3 << endl;
        */
        //cout << "Shutter: " << video.ReadShutter(img) << endl;
        
        // refresh screen 
        texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);
        // Activate video viewport and render texture
        vVideo.ActivateScissorAndClear();
        texVideo.RenderToViewportFlipY();
        // Swap back buffer with front and process window events via GLUT
        d_panel.Render();
        pangolin::FinishGlutFrame();
        
    }
    
    delete[] img;
    
    return 0;
}


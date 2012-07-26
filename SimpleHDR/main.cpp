/**
 * @author  Akram Hussein
 * Copyright (C) 2012  Akram Hussein
 *                     Imperial College London
 **/

#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <cassert>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

#include <boost/thread/thread.hpp>

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] )
{

    // Setup Video Source
    FirewireVideo video = FirewireVideo(); // Simplified constructor

    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();
    
    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* img = new unsigned char[video.SizeBytes()];
    bool over_exposed = true;
    
    //video.SetAutoAll();
        
    video.PrintCameraReport();
    
    for(int frame_number=0; !pangolin::ShouldQuit(); ++frame_number)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        /*
        if (over_exposed){
            cout << "Over" << endl;
            video.SetShutterTimeQuant(400);
            over_exposed = false;
            cout << video.GetShutterTimeQuant() << endl;
        }
        else {
            cout << "Under" << endl;
            video.SetShutterTimeQuant(200);
            over_exposed = true;
            cout << video.GetShutterTimeQuant() << endl;
        }
        */
        
        // wait 1/30th of a second to sync change in settings and display
        boost::this_thread::sleep(boost::posix_time::seconds(1/30));
        
        // Grab frame and save to JPG with exif
        //video.SaveFrame(frame_number, img, true, true);
        video.GrabOneShot(img);
        
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




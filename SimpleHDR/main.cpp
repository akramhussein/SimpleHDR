/**
 * @author  Akram Hussein
 * Copyright (C) 2012  Akram Hussein
 *                     Imperial College London
 **/

#include <sys/stat.h>
#include <iostream>
#include <iomanip>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

#include <boost/thread/thread.hpp>

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] )
{

    /*-----------------------------------------------------------------------
     *  SETUP SOURCE
     *-----------------------------------------------------------------------*/    
    
    FirewireVideo video = FirewireVideo();     
    video.SetAutoAll();
    video.PrintCameraReport();

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
    
    //keyboard shortcuts
    pangolin::RegisterKeyPressCallback( 'h', SetVarFunctor<bool>("ui.HDR Mode", true));                 // hdr mode 
    pangolin::RegisterKeyPressCallback( 'n', SetVarFunctor<bool>("ui.HDR Mode", false));                // normal mode
    pangolin::RegisterKeyPressCallback( 32 , SetVarFunctor<bool>("ui.Record", true));                   // grab multiple frames in ppm & jpg
    pangolin::RegisterKeyPressCallback( 'c', SetVarFunctor<bool>("ui.Capture Frame", true));            // grab single frame in ppm & jpg
    pangolin::RegisterKeyPressCallback( 'm', SetVarFunctor<bool>("ui.Manual Camera Settings", true));   // manual on
    pangolin::RegisterKeyPressCallback( 'a', SetVarFunctor<bool>("ui.Manual Camera Settings", false));  // manual off
    pangolin::RegisterKeyPressCallback( 'r', SetVarFunctor<bool>("ui.Reset Camera Settings", true));    // reset settings
    pangolin::RegisterKeyPressCallback( 'p', SetVarFunctor<bool>("ui.Print", true));                    // print settings
    //pangolin::RegisterKeyPressCallback( 'f', SetVarFunctor<bool>("ui.Reset Frame Count", true));
    
    // options
    static Var<bool> record("ui.Record",false,false);
    static Var<bool> capture("ui.Capture Frame",false,false);
    static Var<bool> capture_hdr("ui.Capture HDR Frame",false,false);
    static Var<bool> hdr("ui.HDR Mode",false,true);
    //static Var<bool> AEC("ui.Automatic Exposure Control",false,true);
    //static Var<bool> motion("ui.Motion Correction",false,true);
    static Var<bool> manual("ui.Manual Camera Settings",false,true);

    static Var<bool> print("ui.Print",false,false);
    
    // camera settings
    //static Var<float> shutter("ui.Shutter",video.GetShutterTimeQuant(),video.GetShutterTimeQuantMin()+numeric_limits<double>::epsilon(),video.GetShutterTimeQuantMax(), false);
    static Var<float> shutter("ui.Shutter (s)",video.GetShutterTime(),video.GetShutterTimeMin(),video.GetShutterTimeMax(), true);
    static Var<float> exposure("ui.Exposure (EV)",video.GetExposure(),video.GetExposureMin(),video.GetExposureMax(), false);      //faulty  
    static Var<float> brightness("ui.Brightness (%)",video.GetBrightness(),video.GetBrightnessMin(),video.GetBrightnessMax(), false); 
    static Var<float> gain("ui.Gain (dB)",video.GetGain(),video.GetGainMin(),video.GetGainMax(), false);
    static Var<float> gamma("ui.Gamma",video.GetGamma(),video.GetGammaMin(),video.GetGammaMax(), false);
    static Var<float> saturation("ui.Saturation (%)",video.GetSaturation(),video.GetSaturationMin(),video.GetSaturationMax(), false);
    static Var<int> hue("ui.Hue (deg)",video.GetHueQuant(),video.GetHueQuantMin(),video.GetHueQuantMax(), false);
    static Var<int> sharpness("ui.Sharpness",video.GetSharpnessQuant(),video.GetSharpnessQuantMin(),video.GetSharpnessQuantMax(), false);
    static Var<float> framerate("ui.Framerate (fps)",video.GetFramerate(),video.GetFramerateMin(),video.GetFramerateMax(), true);
    //static Var<float> tilt("ui.Tilt",0,0,100);
    //static Var<float> pan("ui.Pan",0,0,100);
     
    static Var<bool> reset("ui.Reset Camera Settings",false,false);
    
    // info
    static Var<float> current_framerate("ui.Framerate (fps)",0);
    static Var<int> recorded_frames("ui.Recorded Frames",0);
    //static Var<bool> reset_frame("ui.Reset Frame Count",false,false);
    static Var<string> vendor("ui.Vendor", video.GetCameraVendor());
    static Var<string> model("ui.Model", video.GetCameraModel());

    /*-----------------------------------------------------------------------
     *  CAPTURE
     *-----------------------------------------------------------------------*/    
      
    bool over_exposed = true;
    bool save = false;
    
    for(int frame_number=0; !pangolin::ShouldQuit(); ++frame_number)
    {
        //cout << "Exposure: " << video.GetExposure() << endl;
        //cout << "Exposure Quant: " << video.GetExposureQuant() << endl;
        
        
        if(pangolin::HasResized())
            DisplayBase().ActivateScissorAndClear();
        
        //current_frame.operator=(frame_number);
        current_framerate.operator=(video.GetFramerate());
        
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        if(pangolin::Pushed(capture_hdr)){
            //hdr_frame(); -- TO DO
        }
            
        if(pangolin::Pushed(reset)){
            shutter.Reset();
            exposure.Reset();
            brightness.Reset();
            gain.Reset();
            gamma.Reset();
            saturation.Reset();
            hue.Reset();
            sharpness.Reset();
            framerate.Reset();
            
        }
        
        if(pangolin::Pushed(print)){
            
            cout << "Shutter: " << video.GetShutterTime() << endl;
            cout << "Exposure: " << video.GetExposure() << endl;
            cout << "Brightness: " << video.GetBrightness() << endl;
            cout << "Gain: " << video.GetGain() << endl;
            cout << "Gamma: " << video.GetGamma() << endl;
            cout << "Saturation: " << video.GetSaturation() << endl;
            cout << "Hue: " << video.GetHueQuant() << endl;
            cout << "Sharpness: " << video.GetSharpnessQuant() << endl;
            cout << "Framerate: " << video.GetFramerate() << endl << endl;            
            
        }
        
        if( hdr ) {
            if (over_exposed){
                //cout << "Over" << endl;
                video.SetShutterTimeQuant(150);
                over_exposed = false;
                //cout << video.GetShutterTimeQuant() << endl;
            }
            else {
                //cout << "Under" << endl;
                video.SetShutterTimeQuant(100);
                over_exposed = true;
                //cout << video.GetShutterTimeQuant() << endl;
            }
        }
        else{
            video.SetAutoAll();
        }
        
        if (manual){ 
            video.SetShutterTime(shutter);
            video.SetExposure(exposure);
            video.SetBrightness(brightness);
            video.SetGain(gain);
            video.SetGamma(gamma);
            video.SetSaturation(saturation);
            video.SetHueQuant(hue);
            video.SetSharpnessQuant(sharpness);
            video.SetFramerate(framerate);
            
        }
        
        if(pangolin::Pushed(capture)){ video.SaveFrame(frame_number, img, true, "single", true); } 
        
        if(pangolin::Pushed(capture_hdr)){ /**/ } 

        /*
        if(pangolin::Pushed(reset_frame)){ 
            frame_number = 0; 
            recorded_frames.operator=(frame_number);
        }
        */
        
        // start/stop recording
        if (!save && pangolin::Pushed(record)){ 
            save = true; 
            frame_number = 0; 
            recorded_frames.operator=(frame_number);
        }
        
        if (save && pangolin::Pushed(record)){ save = false; }

        // save mode
        if (save && hdr){
            // wait 1/30th of a second to sync change in settings and display
            boost::this_thread::sleep(boost::posix_time::seconds(1/30));
            video.SaveFrame(frame_number, img, true, "hdr", true); 
            recorded_frames.operator=(frame_number);
            
        } 
        else if (save){
            video.SaveFrame(frame_number, img, true, "normal", true);  
            recorded_frames.operator=(frame_number);
        }        
        else {
            video.GrabOneShot(img);
        }
        
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




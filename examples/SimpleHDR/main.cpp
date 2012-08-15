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
    
    FirewireVideo video = FirewireVideo();
    //video.ReadConfigFile();
    video.SetHDRRegister(false);
    video.SetMetaDataFlags( META_ALL );
    video.CreateShutterMaps();
    video.SetAllFeaturesAuto();
    //video.Stop();
    //video.FlushDMABuffer();//remove spurious frames
    //video.PrintCameraReport();
    
    unsigned char* img = new unsigned char[video.SizeBytes()];
    
    /*-----------------------------------------------------------------------
     *  GUI
     *-----------------------------------------------------------------------*/    
   
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();
    
    // Create Glut window
    const int panel_width = 200;
    double scale = 1.25;
    pangolin::CreateGlutWindowAndBind("SimpleHDR", w*scale + panel_width, h*scale);
    
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
     *  KEYBOARD SHORCUTS
     *-----------------------------------------------------------------------*/ 

    pangolin::RegisterKeyPressCallback( 'h', SetVarFunctor<bool>("ui.HDR Mode", true));                 // hdr mode 
    pangolin::RegisterKeyPressCallback( 'n', SetVarFunctor<bool>("ui.HDR Mode", false));                // normal mode
    pangolin::RegisterKeyPressCallback( 32 , SetVarFunctor<bool>("ui.Record", true));                   // grab multiple frames in ppm & jpeg
    pangolin::RegisterKeyPressCallback( 'c', SetVarFunctor<bool>("ui.Capture Frame", true));            // grab single frame in ppm & jpeg
    pangolin::RegisterKeyPressCallback( 'm', SetVarFunctor<bool>("ui.Manual Camera Settings", true));   // manual on
    pangolin::RegisterKeyPressCallback( 'a', SetVarFunctor<bool>("ui.Manual Camera Settings", false));  // manual off
    pangolin::RegisterKeyPressCallback( 'r', SetVarFunctor<bool>("ui.Reset Camera Settings", true));    // reset settings
    pangolin::RegisterKeyPressCallback( 'f', SetVarFunctor<bool>("ui.Get Response Function", true));    // get response function
    
    /*-----------------------------------------------------------------------
     *  CONTROL PANEL
     *-----------------------------------------------------------------------*/ 
    
    // camera brand/model info
    
    static Var<string> vendor("ui.Vendor", video.GetCameraVendor());
    static Var<string> camera("ui.Camera", video.GetCameraModel());
    
    // capture options
    static Var<bool> record("ui.Record",false,false);
    // hdr controls
    static Var<bool> hdr("ui.HDR Mode",false,true);
    
    static Var<int> recorded_frames("ui.Recorded Frames",0);
    static Var<int> recorded_time("ui.Recorded (secs)", 0);
    
    static Var<bool> capture("ui.Capture Frame",false,false);
    static Var<bool> capture_hdr("ui.Capture HDR Frame",false,false);
    static Var<bool> response_function("ui.Get Response Function",false,false);
    
    //static Var<bool> AEC("ui.Automatic Exposure Control",false,true);
    
    //other options
    static Var<bool> manual("ui.Manual Camera Settings",false,true);

    // camera settings
    static Var<float> shutter("ui.Shutter (s)", video.GetFeatureValue(DC1394_FEATURE_SHUTTER),
                               video.GetFeatureValueMin(DC1394_FEATURE_SHUTTER),
                               video.GetFeatureValueMax(DC1394_FEATURE_SHUTTER), true);

    static Var<float> exposure("ui.Exposure (EV)", video.GetFeatureValue(DC1394_FEATURE_EXPOSURE),
                                video.GetFeatureValueMin(DC1394_FEATURE_EXPOSURE),
                                video.GetFeatureValueMax(DC1394_FEATURE_EXPOSURE),false);    

    static Var<float> brightness("ui.Brightness (%)", video.GetFeatureValue(DC1394_FEATURE_BRIGHTNESS),
                                 video.GetFeatureValueMin(DC1394_FEATURE_BRIGHTNESS),
                                 video.GetFeatureValueMax(DC1394_FEATURE_BRIGHTNESS),false); 
    
    static Var<float> gain("ui.Gain (dB)", video.GetFeatureValue(DC1394_FEATURE_GAIN),
                           video.GetFeatureValueMin(DC1394_FEATURE_GAIN),
                           video.GetFeatureValueMax(DC1394_FEATURE_GAIN),false);    
    
    static Var<float> gamma("ui.Gamma", video.GetFeatureValue(DC1394_FEATURE_GAMMA),
                            video.GetFeatureValueMin(DC1394_FEATURE_GAMMA),
                            video.GetFeatureValueMax(DC1394_FEATURE_GAMMA),false);    
    
    static Var<float> saturation("ui.Saturation (%)", video.GetFeatureValue(DC1394_FEATURE_SATURATION),
                                 video.GetFeatureValueMin(DC1394_FEATURE_SATURATION),
                                 video.GetFeatureValueMax(DC1394_FEATURE_SATURATION),false);  
    
    static Var<float> hue("ui.Hue (deg)", video.GetFeatureValue(DC1394_FEATURE_HUE),
                        video.GetFeatureValueMin(DC1394_FEATURE_HUE),
                        video.GetFeatureValueMax(DC1394_FEATURE_HUE),false);  

    static Var<int> sharpness("ui.Sharpness",video.GetFeatureQuant(DC1394_FEATURE_SHARPNESS),
                              video.GetFeatureQuantMin(DC1394_FEATURE_SHARPNESS),
                              video.GetFeatureQuantMax(DC1394_FEATURE_SHARPNESS),false);  
    
    static Var<int> whitebalance_B_U("ui.White Balance (Blue/U)",video.GetWhiteBalanceBlueU(),
                                     video.GetFeatureQuantMin(DC1394_FEATURE_WHITE_BALANCE),
                                     video.GetFeatureQuantMax(DC1394_FEATURE_WHITE_BALANCE),false);  
    
    static Var<int> whitebalance_R_V("ui.White Balance (Red/V)",video.GetWhiteBalanceRedV(),
                                     video.GetFeatureQuantMin(DC1394_FEATURE_WHITE_BALANCE),
                                     video.GetFeatureQuantMax(DC1394_FEATURE_WHITE_BALANCE),false);  
                                             
    static Var<bool> reset("ui.Reset Camera Settings",false,false);
     
    /*-----------------------------------------------------------------------
     *  CAPTURE LOOP
     *-----------------------------------------------------------------------*/     
    
    bool save = false;    
    time_t start, end;
    uint32_t s0 = video.GetShutterMapQuant(0.004857);
    uint32_t s1 = video.GetShutterMapQuant(0.01392); 

    // loop until quit (ESC key)
    for(int frame_number=0; !pangolin::ShouldQuit(); ++frame_number)
    {     
        // Screen refresh
        if(pangolin::HasResized())
            DisplayBase().ActivateScissorAndClear();

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
        /*-----------------------------------------------------------------------
         *  AUXILLARY CONTROLS
         *-----------------------------------------------------------------------*/ 
        
        if( pangolin::Pushed(response_function) ){ video.GetResponseFunction(); }
        
        /*-----------------------------------------------------------------------
         *  CONTROL LOGIC
         *-----------------------------------------------------------------------*/

        // reset feature settings
        if(pangolin::Pushed(reset)){
            shutter.Reset();
            exposure.Reset();
            brightness.Reset();
            gain.Reset();
            gamma.Reset();
            saturation.Reset();
            hue.Reset();
            sharpness.Reset(); 
            whitebalance_B_U.Reset();
            whitebalance_R_V.Reset();
        }

        /*
        if (AEC){
        GetAEC(image,&s);
         
        }
        */
  
        // HDR MODE
        
        // checks if hdr mode has been switched and sets register on
        if(pangolin::Pushed(hdr.var->meta_gui_changed)){
            hdr ? video.SetHDRRegister(true) : video.SetHDRRegister(false);
        }
        // shutter settings set seperately for AEC mode capabilities
        if (hdr){ video.SetHDRShutterFlags(s0,s1,s0,s1); }
        
        //with AEC controls
        //if (hdr && !AEC){ video.SetHDRShutterFlags(s0,s1,s0,s1); }
        //if (hdr && AEC){ video.SetHDRShutterFlags(aec1,aec2,aec1,aec2);

        
        // MANUAL SETTINGS

        if ( manual && !hdr ) { 
            
            /* 
             * exposure & shutter inter-linked, therefore need special logic control
             *  if: exposure val is changed, shutter val is changed (camera works out new value)
             *  else: shutter val is set to gui value 
             */
            
            video.SetFeatureValue(DC1394_FEATURE_EXPOSURE, exposure); 
            
            if(pangolin::Pushed(exposure.var->meta_gui_changed)){
                video.SetFeatureAuto(DC1394_FEATURE_SHUTTER);
                shutter.operator=(video.GetFeatureValue(DC1394_FEATURE_SHUTTER));
            }
            else{
                video.SetFeatureValue(DC1394_FEATURE_SHUTTER, shutter);
            }
            
        }

        if ( manual ){    
            video.SetFeatureValue(DC1394_FEATURE_BRIGHTNESS, brightness);
            video.SetFeatureValue(DC1394_FEATURE_GAIN, gain);
            video.SetFeatureValue(DC1394_FEATURE_GAMMA, gamma); 
            video.SetFeatureValue(DC1394_FEATURE_SATURATION, saturation);
            video.SetFeatureValue(DC1394_FEATURE_HUE, hue); 
            video.SetFeatureQuant(DC1394_FEATURE_SHARPNESS, sharpness);
            video.SetWhiteBalance(whitebalance_B_U, whitebalance_R_V);
        } 

        // resets all features to automatic mode
        else if( !manual && !hdr ) { 
            
            video.SetAllFeaturesAuto(); 
            
            // updates all trackbars upon switching back to automatic mode
            exposure.operator=(video.GetFeatureValue(DC1394_FEATURE_EXPOSURE));
            shutter.operator=(video.GetFeatureValue(DC1394_FEATURE_SHUTTER));
            brightness.operator=(video.GetFeatureValue(DC1394_FEATURE_BRIGHTNESS));
            gain.operator=(video.GetFeatureValue(DC1394_FEATURE_GAIN));
            gamma.operator=(video.GetFeatureValue(DC1394_FEATURE_GAMMA));
            saturation.operator=(video.GetFeatureValue(DC1394_FEATURE_SATURATION));
            hue.operator=(video.GetFeatureValue(DC1394_FEATURE_HUE));
            sharpness.operator=(video.GetFeatureQuant(DC1394_FEATURE_SHARPNESS));
            whitebalance_B_U.operator=(video.GetWhiteBalanceBlueU());
            whitebalance_R_V.operator=(video.GetWhiteBalanceRedV());
        }
        
        if( pangolin::Pushed(capture) ) { video.SaveSingleFrame(img); } 
        
        if( pangolin::Pushed(capture_hdr) ){
            
            // see if response function has already been generated
            if (!video.CheckResponseFunction()) {
                cout << "[HDR]: No response function found, generating one" << endl;
                video.GetResponseFunction();
            } 
            // float current_shutter = video.GetFeatureValue(DC1394_FEATURE_SHUTTER);
            float shutter[4] = {0.038, 0.001, 0.038, 0.001};
            video.CaptureHDRFrame(img, 4, shutter);
        } 
    

        /*-----------------------------------------------------------------------
         *  Save options
         *-----------------------------------------------------------------------*/    
        
        // start/stop recording
        if ( save && pangolin::Pushed(record) ){ 
            
            char time_stamp[32];
            char command[64];
            
            // set save flag to false
            save = false;
            
            // get time stamp for file name
            video.GetTimeStamp(time_stamp);
            
            const char *new_stamp = time_stamp;
            cout << new_stamp << endl;
            // create command string: convert video, remove files and then echo completed - should be thread safe this way
            sprintf(command, "convert -quality 100 ./video/ppm/*.ppm ./video/%s.%s && rm -rf ./video/ppm/ && echo '[VIDEO]: Video saved to ./video/'", time_stamp, "mpeg");

            // run video conversion in seperate thread (may take a while so lets us continue)
            boost::thread(system,command);  
            
        }

        if ( !save && pangolin::Pushed(record) ){ 
            save = true; 
            time (&start); // get current time
            frame_number = 0; 
            recorded_frames.operator=(frame_number);
            recorded_time.operator=(0);
        }

        // save mode
        if ( save && hdr ){
            video.RecordFramesOneShot(frame_number, img, true, hdr); 
            recorded_frames.operator=(frame_number);
            time (&end);
            recorded_time.operator=(difftime(end,start));
        } 
        else if ( save ){
            video.RecordFrames(frame_number, img, true, false, hdr); 
            recorded_frames.operator=(frame_number);
            time (&end);
            recorded_time.operator=(difftime(end,start));
        } 
        else if ( hdr ){
            video.GrabOneShot(img);
        }
        else{
            video.GrabOneShot(img);
        }
       
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


    /* This file is part of the Pangolin Project.
    * http://github.com/stevenlovegrove/Pangolin
    *
    * Copyright (c) 2011 Steven Lovegrove
    *
    * Permission is hereby granted, free of charge, to any person
    * obtaining a copy of this software and associated documentation
    * files (the "Software"), to deal in the Software without
    * restriction, including without limitation the rights to use,
    * copy, modify, merge, publish, distribute, sublicense, and/or sell
    * copies of the Software, and to permit persons to whom the
    * Software is furnished to do so, subject to the following
    * conditions:
    *
    * The above copyright notice and this permission notice shall be
    * included in all copies or substantial portions of the Software.
    *
    * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    * OTHER DEALINGS IN THE SOFTWARE.
    */

    #ifndef PANGOLIN_FIREWIRE_H
    #define PANGOLIN_FIREWIRE_H

    #include <sys/stat.h>

    #include <pangolin/pangolin.h>
    #include <pangolin/video.h>

    #include <dc1394/dc1394.h>

    #include <ImageMagick/Magick++.h>

    #include <exiv2/exiv2.hpp>

    #ifndef _WIN32
    #include <unistd.h>
    #endif

    namespace pangolin
    {

    std::string Dc1394ColorCodingToString(dc1394color_coding_t coding);

    dc1394color_coding_t Dc1394ColorCodingFromString(std::string coding);

    void Dc1394ModeDetails(dc1394video_mode_t mode, unsigned& w, unsigned& h, std::string& format );

    class FirewireFrame
    {
    friend class FirewireVideo;
    public:
    bool isValid() { return frame; }
    unsigned char* Image() { return frame ? frame->image : 0; }
    unsigned Width() const { return frame ? frame->size[0] : 0; }
    unsigned Height() const { return frame ? frame->size[1] : 0; }

    protected:
    FirewireFrame(dc1394video_frame_t* frame) : frame(frame) {}
    dc1394video_frame_t *frame;
    };

    struct Guid
    {
        Guid(uint64_t guid):guid(guid){}
        uint64_t guid;
    };

    struct MetaData 
    {
        uint32_t flags;
        unsigned int brightness;
        unsigned int auto_exposure;
        unsigned int whitebalance_u_b, whitebalance_v_r;
        uint32_t timestamp, frame_count;
        uint32_t shutterQuant, gain;
        float shutterAbs;

        //TODO: Add strobe, GPIO and ROI functionality if needed.
        uint32_t strobe, gpio, roi;

        void copy_from( MetaData *in ) {
            flags=in->flags;
            brightness=in->brightness;
            auto_exposure=in->auto_exposure;
            whitebalance_u_b=in->whitebalance_u_b;
            whitebalance_v_r=in->whitebalance_v_r;
            timestamp=in->timestamp;
            frame_count=in->frame_count;
            shutterQuant=in->shutterQuant;
            gain=in->gain;
            shutterAbs=in->shutterAbs;
            strobe = in->strobe;
            gpio=in->gpio;
            roi=in->roi;
        }
    };

    typedef enum {
    META_TIMESTAMP = 1,
    META_GAIN = 2,
    META_SHUTTER = 4,
    META_BRIGHTNESS = 8,
    META_EXPOSURE = 16,
    META_WHITE_BALANCE = 32,
    META_FRAME_COUNTER = 64,
    META_STROBE = 128,
    META_GPIO_PIN_STATE = 256,
    META_ROI_POSITION = 512,
    META_ALL = 1023
    } meta_flags;
        
    class FirewireVideo : public VideoInterface
    {
    public:
    const static int MAX_FR = -1;
    const static int EXT_TRIG = -1;

    FirewireVideo(
    unsigned deviceid = 0,
    dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8,
    dc1394framerate_t framerate = DC1394_FRAMERATE_30,
    dc1394speed_t iso_speed = DC1394_ISO_SPEED_400,
    int dma_buffers = 10
    );

    FirewireVideo(
    Guid guid,
    dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8,
    dc1394framerate_t framerate = DC1394_FRAMERATE_30,
    dc1394speed_t iso_speed = DC1394_ISO_SPEED_400,
    int dma_buffers = 10
    );

    FirewireVideo(
      Guid guid,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top,
      dc1394speed_t iso_speed,
      int dma_buffers, bool reset_at_boot=false
    );

    FirewireVideo(
      unsigned deviceid,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top,
      dc1394speed_t iso_speed,
      int dma_buffers, bool reset_at_boot=false
    );

    ~FirewireVideo();

    //! Implement VideoSource::Width()
    unsigned Width() const { return width; }

    //! Implement VideoSource::Height()
    unsigned Height() const { return height; }

    //! Implement VideoSource::SizeBytes()
    size_t SizeBytes() const;

    //! Implement VideoSource::PixFormat()
    std::string PixFormat() const;

    //! Implement VideoSource::Start()
    void Start();

    //! Implement VideoSource::Stop()
    void Stop();

    //! Clear the DMA buffer
    void FlushDMABuffer();
        
    /*-----------------------------------------------------------------------
     * ONE SHOT CONTROL
     *-----------------------------------------------------------------------*/
        
    //! Stop the video stream before using One-shot mode
    void StopForOneShot();

    //! Grab one shot (iso-transmission must be off - call StopForOneShot first)
    bool GrabOneShot(unsigned char* image);
        
    //! Check to see if camera is one-shot capable
    bool CheckOneShotCapable();

    /*-----------------------------------------------------------------------
     * LOOPUP TABLE & META DATA ETC
     *-----------------------------------------------------------------------*/
        
    //! set the meta data flags to be included in image data
    void SetMetaDataFlags(int flags);
    
    //! return the current meta data flags from camera
    uint32_t GetMetaDataFlags();
    
    //! read the meta data from an image according to meta flags
    void ReadMetaData( unsigned char *image, MetaData *metaData );
        
    float ReadShutter( unsigned char *image );
    
    //! create lookup table to convert quantised shutter values to absolute values
    void CreateShutterLookupTable();
            
    /*-----------------------------------------------------------------------
     *  FRAME GRAB
     *-----------------------------------------------------------------------*/

    //! Implement VideoSource::GrabNext()
    bool GrabNext( unsigned char* image, bool wait = true );

    //! Implement VideoSource::GrabNewest()
    bool GrabNewest( unsigned char* image, bool wait = true );

    //! Return object containing reference to image data within
    //! DMA buffer. The FirewireFrame must be returned to
    //! signal that it can be reused with a corresponding PutFrame()
    FirewireFrame GetNext(bool wait = true);

    //! Return object containing reference to newest image data within
    //! DMA buffer discarding old images. The FirewireFrame must be
    //! returned to signal that it can be reused with a corresponding PutFrame()
    FirewireFrame GetNewest(bool wait = true);

    //! Return FirewireFrame object. Data held by FirewireFrame is
    //! invalidated on return.
    void PutFrame(FirewireFrame& frame);
        
    /*-----------------------------------------------------------------------
     *  SHUTTER
     *-----------------------------------------------------------------------*/

    //! set auto shutter value
    void SetAutoShutterTime();

    //! set manual shutter
    void SetManualShutterTime();

    //! set absolute shutter value
    void SetShutterTime(float val);

    //! return quantised shutter value
    void SetShutterTimeQuant(int shutter);  

    //! return absolute shutter value
    float GetShutterTime() const;

    //! return the quantised shutter time
    int GetShutterTimeQuant() const;
    
    //! return max shutter time value
    float GetShutterTimeMax() const;
       
    //! return min shutter time value
    float GetShutterTimeMin() const;
    
    //! return max quantised shutter time value
    int GetShutterTimeQuantMax() const;
    
    //! return min quantised shutter time value
    int GetShutterTimeQuantMin() const;
    
    /*-----------------------------------------------------------------------
     *  EXPOSURE
     *-----------------------------------------------------------------------*/
      
    //! set auto exposure
    void SetAutoExposure();
    
    //! set manual exposure
    void SetManualExposure();
        
    //! set absolute exposure value
    void SetExposure(float val);
    
    //! set quantised exposure
    void SetExposureQuant(int exposure);
     
    //! return absolute exposure value
    float GetExposure() const;
    
    //! return quantised exposure
    int GetExposureQuant() const;

    //! return max exposure time value
    float GetExposureMax() const;
    
    //! return min exposure time value
    float GetExposureMin() const;
    
    //! return max quantised exposure value
    int GetExposureQuantMax() const;
    
    //! return min quantised exposure value
    int GetExposureQuantMin() const;
        
    /*-----------------------------------------------------------------------
     *  BRIGHTNESS
     *-----------------------------------------------------------------------*/   
    
    //! set auto brightness mode
    void SetAutoBrightness();            // doesn't exist for point grey
    
    //! set manual brightness mode
    void SetManualBrightness();      
    
    //! set absolute brightness value 
    void SetBrightness(float val);       
    
    //! set quantised brightness value 
    void SetBrightnessQuant(int val);    // doesn't exist for point grey
    
    //! return absolute brightness value 
    float GetBrightness() const;        
    
    //! return quantised brightness value 
    int GetBrightnessQuant() const;      // doesn't exist for point grey
        
    //! return max brightness value
    float GetBrightnessMax() const;
    
    //! return min brightness value
    float GetBrightnessMin() const;
    
    //! return max quantised brightness value
    int GetBrightnessQuantMax() const;
    
    //! return min quantised brightness value
    int GetBrightnessQuantMin() const;
        
    /*-----------------------------------------------------------------------
     * GAIN
     *-----------------------------------------------------------------------*/

    //! set auto gain 
    void SetAutoGain();
        
    //! set manual gain
    void SetManualGain();

    //! set absolute gain value
    void SetGain(float val);
        
    //! set quantised gain value
    void SetGainQuant(int val);

    //! return absolute gain value
    float GetGain() const;
        
    //! return the quantised gain
    int GetGainQuant() const;
        
    //! return max gain value
    float GetGainMax() const;
    
    //! return min gain value
    float GetGainMin() const;
    
    //! return max quantised gain value
    int GetGainQuantMax() const;
    
    //! return min quantised gain value
    int GetGainQuantMin() const;

   /*-----------------------------------------------------------------------
    *  GAMMA
    *-----------------------------------------------------------------------*/
    
    //! set gamma on 
    void SetGammaOn();
     
    //! set manual off
    void SetGammaOff();
    
    //! Reset gamma to 1.0
    void ResetGamma();
        
    //! set absolute gamma value
    void SetGamma(float val); 
    
    //! set quantised gamma value
    void SetGammaQuant(int val); 
        
    //! return absolute gamma value
    float GetGamma() const;
    
    //! return the quantised gamma
    int GetGammaQuant() const;
        
    //! return max gamma value
    float GetGammaMax() const;
    
    //! return min gamma value
    float GetGammaMin() const;
    
    //! return max quantised gamma value
    int GetGammaQuantMax() const;
    
    //! return min quantised gamma value
    int GetGammaQuantMin() const;
    
    /*-----------------------------------------------------------------------
     *  WHITE BALANCE
     *-----------------------------------------------------------------------*/
      
    //! set the white balance to single shot auto
    void SetSingleAutoWhiteBalance();
        
    //! set the white balance
    void SetWhiteBalance(unsigned int u_b_value, unsigned int v_r_value);

    //! get the white balance
    void GetWhiteBalance(unsigned int *Blue_U_val, unsigned int *Red_V_val);
        
    /*-----------------------------------------------------------------------
     *  SATURATION
     *-----------------------------------------------------------------------*/     

    //! set auto saturation mode
    void SetAutoSaturation();
     
    //! set manual saturation mode
    void SetManualSaturation();
        
    //! set saturation value
    void SetSaturation(float val); 
        
    //! set quantised saturation value
    void SetSaturationQuant(int val); 
    
    //! return saturation value
    float GetSaturation() const; 
     
    //! return quantised saturation value
    int GetSaturationQuant() const;
    
    //! return max saturation value
    float GetSaturationMax() const;
    
    //! return min saturation value
    float GetSaturationMin() const;
    
    //! return max quantised saturation value
    int GetSaturationQuantMax() const;
    
    //! return min quantised saturation value
    int GetSaturationQuantMin() const;
        
    /*-----------------------------------------------------------------------
     *  HUE
     *-----------------------------------------------------------------------*/     
           
    //! set gamma on 
    void SetHueOn();
    
    //! set manual off
    void SetHueOff();
    
    //! Reset gamma to 0
    void ResetHue();
        
    //! set hue value
    void SetHue(float val); 
    
    //! set hue value
    void SetHueQuant(int val); 
    
    //! return hue value
    float GetHue() const; 

    //! return hue value
    int GetHueQuant() const; 
        
    //! return max hue value
    float GetHueMax() const;
    
    //! return min hue value
    float GetHueMin() const;
    
    //! return max quantised hue value
    int GetHueQuantMax() const;
    
    //! return min quantised hue value
    int GetHueQuantMin() const;
        
    /*-----------------------------------------------------------------------
     *  SHARPNESS
     *-----------------------------------------------------------------------*/    
     
    //! set auto sharpness mode
    void SetAutoSharpness();        
    
    //! set manual sharpness mode
    void SetManualSharpness();      

    //! set absolute sharpness value 
    void SetSharpness(float val);       // doesn't exist for point grey
        
    //! set quantised sharpness value 
    void SetSharpnessQuant(int val);
    
    //! return absolute sharpness value 
    float GetSharpness() const;         // doesn't exist for point grey
    
    //! return quantised sharpness value 
    int GetSharpnessQuant() const;
        
    //! return max sharpness value
    float GetSharpnessMax() const;
    
    //! return min sharpness value
    float GetSharpnessMin() const;
    
    //! return max quantised sharpness value
    int GetSharpnessQuantMax() const;
        
    //! return min quantised sharpness value
    int GetSharpnessQuantMin() const;
            
    /*-----------------------------------------------------------------------
     *  FRAMERATE
     *-----------------------------------------------------------------------*/  
       
    
     
    //! set framerate auto
    void SetAutoFramerate();
    
    //! set framerate auto
    void SetManualFramerate();
     
    //! set framerate value
    void SetFramerate(float val);
     
    //! set quantised framerate 
    void SetFramerateQuant(int val);
    
    //! return framerate value
    float GetFramerate() const;
    
    //! return quantised framerate
    int GetFramerateQuant() const;
      
    //! return max framerate value
    float GetFramerateMax() const;
    
    //! return min framerate value
    float GetFramerateMin() const;
    
    //! return max quantised framerate value
    int GetFramerateQuantMax() const;
    
    //! return min quantised framerate value
    int GetFramerateQuantMin() const;
        
        
    /*-----------------------------------------------------------------------
     *  TILT
     *-----------------------------------------------------------------------*/   
    
    /*-----------------------------------------------------------------------
     *  PAN
     *-----------------------------------------------------------------------*/
        
    /*-----------------------------------------------------------------------
     *  TRIGGERS
     *-----------------------------------------------------------------------*/

    //! set the trigger to internal, i.e. determined by video mode
    void SetInternalTrigger();

    //! set the trigger to external
    void SetExternalTrigger(
      dc1394trigger_mode_t mode=DC1394_TRIGGER_MODE_0,
      dc1394trigger_polarity_t polarity=DC1394_TRIGGER_ACTIVE_HIGH,
      dc1394trigger_source_t source=DC1394_TRIGGER_SOURCE_0
    );      
    
    /*-----------------------------------------------------------------------
     *  SAVING
     *-----------------------------------------------------------------------*/
    
    //! Use standard frame grab and save to .ppm (for jpg, set to true)
    bool SaveFrame(     
                        int frame_number,      // current frame number
                        unsigned char* image,  // empty image buffer -- to go in future
                        bool wait,             // defaults = true
                        bool jpg           // true => converts ppm to jpg (default = false)
                   ); 
    
    //! Use one shot frame grab and save to .ppm (for jpg, set to true)
    bool SaveOneShot(     
                        int frame_number,     // current frame number
                        unsigned char* image, // empty image buffer -- to go in future
                        bool jpg              // if true; converts ppm to jpg
                     );
    
    //! Save image file to ppm or jpg
    bool SaveFile(    
                        int frame_number,           // current frame number
                        dc1394video_frame_t *frame, // frame buffer
                        bool jpg = false            // jpg save: defaults to false
                );
        
    /*-----------------------------------------------------------------------
     *  CONVERTING
     *-----------------------------------------------------------------------*/
        
    // HAVEN'T TESTED! PROBABLY DOESN'T WORK
    dc1394video_frame_t* ConvertToRGB(dc1394video_frame_t *original_frame);
           
    /*-----------------------------------------------------------------------
     *  REPORTING
     *-----------------------------------------------------------------------*/
    
    //! Print full camera features and current settings report
    void PrintCameraReport();
    
    //! Get Camera EXIF
    // GetCameraExifData(ExifData **ExifData);
        
    /*-----------------------------------------------------------------------
     *  CONVENIENCE UTILITIES
     *-----------------------------------------------------------------------*/
    
    //! set all to auto mode for point grey flea
    void SetAutoAll();
      
    //! get the best video mode and highest framerate for camera (default colour RGB8)
    void GetBestSettings( dc1394video_mode_t video_mode, 
                          dc1394framerate_t framerate 
                         );
    
    //! get camera feature set
    dc1394featureset_t *GetFeatures() { return &features; }
        
    //! get camera vendor
    std::string GetCameraVendor() { return camera->vendor; }
    
    //! get camera model
    std::string GetCameraModel() { return camera->model; }
     
    //! generate camera response function and save to file
    void GetResponseFunction();
    
    protected:

    void init_camera(
    uint64_t guid, int dma_frames,
    dc1394speed_t iso_speed,
    dc1394video_mode_t video_mode,
    dc1394framerate_t framerate
    );

    void init_format7_camera(
      uint64_t guid, int dma_frames,
      dc1394speed_t iso_speed,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top, bool reset_at_boot
    );

    static int nearest_value(int value, int step, int min, int max);
    static double bus_period_from_iso_speed(dc1394speed_t iso_speed);

    bool running;
    dc1394camera_t *camera;
    unsigned width, height, top, left;
    dc1394featureset_t features;
    dc1394_t * d;
    dc1394camera_list_t * list;
    mutable dc1394error_t err;

    uint32_t meta_data_flags;
    float* shutter_lookup_table;

    };

    }

    #endif // PANGOLIN_FIREWIRE_H

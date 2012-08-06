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

    #include <pangolin/pangolin.h>
    #include <pangolin/video.h>

    #include <dc1394/dc1394.h>

    #include <jpeglib.h>

    #include <boost/thread/thread.hpp>

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
     *  FEATURE CONTROL
     *-----------------------------------------------------------------------*/    

    //!Set all to auto
    void SetAllFeaturesAuto();
     
    //!Set all to manual
    void SetAllFeaturesManual();
        
    //! set feature to auto mode
    void SetFeatureAuto(dc1394feature_t feature);
    
    //! set feature to manual mode
    void SetFeatureManual(dc1394feature_t feature);
    
    //! set feature on
    void SetFeatureOn(dc1394feature_t feature);
    
    //! set feature off
    void SetFeatureOff(dc1394feature_t feature);
           
    //! set feature absolute value
    void SetFeatureValue(dc1394feature_t feature, float value);
    
    //! set feature quantised value
    void SetFeatureQuant(dc1394feature_t feature, int value);
    
    //! check if feature is on or off
    bool GetFeaturePower(dc1394feature_t feature);
        
    //! check current mode (auto [0] or manual [1])
    int GetFeatureMode(dc1394feature_t feature) const;
        
    //! get feature absolute value
    float GetFeatureValue(dc1394feature_t feature) const;
    
    //! get feature quantised value
    int GetFeatureQuant(dc1394feature_t feature) const;
    
    //! get feature max absolute value
    float GetFeatureValueMax(dc1394feature_t feature) const;

    //! get feature min absolute value
    float GetFeatureValueMin(dc1394feature_t feature) const;
    
    //! get feature max quantised value
    int GetFeatureQuantMax(dc1394feature_t feature) const;
    
    //! get feature min quantised value
    int GetFeatureQuantMin(dc1394feature_t feature) const;
    
    //! Reset gamma to 1.0
    void ResetGamma();
    
    //! Reset gamma to 0
    void ResetHue();

    /*-----------------------------------------------------------------------
     *  WHITE BALANCE CONTROLS
     *-----------------------------------------------------------------------*/
      
    //! set the white balance to single shot auto
    void SetSingleAutoWhiteBalance();
           
    //! set the white balance
    void SetWhiteBalance(unsigned int u_b_value, unsigned int v_r_value);

    //! get the white balance
    void GetWhiteBalance(unsigned int *Blue_U_val, unsigned int *Red_V_val);
        
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
     *  RECORDING/SAVING
     *-----------------------------------------------------------------------*/

    //! records multiple frames
    bool RecordFrames(     
                      int frame_number,      // current frame number
                      unsigned char* image,  // empty image buffer -- to go in future
                      bool wait,             // defaults = true
                      bool jpeg = true,      // true = jpeg, false = ppm
                      bool hdr = false       // hdr folder or not
                      ); 
        
    //! records multiple frames using 'One Shot' mode
    bool RecordFramesOneShot(   
                            int frame_number,     // current frame number
                            unsigned char* image, // empty image buffer -- to go
                            bool jpeg = true,     // true = jpeg, false = ppm
                            bool hdr = true // hdr folder or not
                            );
                
    //! saves one frame
    bool CaptureFrame(
                      int frame_number,
                      unsigned char* image,  // empty image buffer -- to go in future
                      bool wait,              // defaults = true
                      bool jpeg = true        //true = jpeg, false = ppm
                     );

    //! saves one frame with 'One Shot' mode
    bool CaptureFrameOneShot(
                          int frame_number,
                          unsigned char* image, // empty image buffer -- to go 
                          bool jpeg = true      // true = jpeg, false = ppm
                      );
        
    //! save image file to ppm or jpeg
    bool SaveFile(    
                    int frame_number,           // current frame number
                    dc1394video_frame_t *frame, // frame buffer
                    const char* path,           // folder name
                    bool jpeg = true                // true = jpeg, false = ppm
                );
    
    //! create ppm from raw frame buffer
    void CreatePPM(dc1394video_frame_t *frame, 
                   const char* filename, 
                   dc1394video_mode_t video_mode);
        
    //! create jpeg from raw frame buffer 
    bool CreateJPEG(dc1394video_frame_t *frame, 
                   const char *filename,  
                   dc1394video_mode_t video_mode);
        
    //! returns video frame in RGB format (e.g. from YUV to RGB)
    dc1394video_frame_t* ConvertToRGB(dc1394video_frame_t *original_frame);
    
    /*-----------------------------------------------------------------------
     *  REPORTING
     *-----------------------------------------------------------------------*/
    
    //! print full camera features and current settings report
    void PrintCameraReport();
    
    /*-----------------------------------------------------------------------
     *  CONVENIENCE UTILITIES
     *-----------------------------------------------------------------------*/
    
    //! get the best video mode and highest framerate for camera (default colour RGB8)
    void GetBestSettings( dc1394video_mode_t video_mode, 
                          dc1394framerate_t framerate 
                         );
    
    //! get camera feature set
    dc1394featureset_t *GetFeatures() { return &features; }
        
    //! get camera vendor
    std::string GetCameraVendor() const { return camera->vendor; }
    
    //! get camera model
    std::string GetCameraModel() const { return camera->model; }
     
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

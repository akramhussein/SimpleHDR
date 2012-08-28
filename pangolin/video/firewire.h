/* This file is part of the Pangolin HDR extension project
 *
 * http://github.com/akramhussein/hdr
 *
 * Copyright (c) 2012 Akram Hussein
 *
 * Original source code by Steven Lovegrove
 *
 * http://github.com/stevenlovegrove/Pangolin
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

    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <inttypes.h>
    #include <sys/stat.h>
    #include <time.h>
    #include <map>

    #include <pangolin/pangolin.h>
    #include <pangolin/video.h>

    #include <dc1394/dc1394.h>

    #include <jpeglib.h>

    #include <boost/thread/thread.hpp>
    #include <boost/property_tree/ptree.hpp>
    #include <boost/property_tree/ini_parser.hpp>

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
        bool abs_on; 

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
    META_ALL = 1023,
    META_ABS = 32678, 
    META_ALL_AND_ABS = 33791,
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
    int dma_buffers = 100
    );

    FirewireVideo(
    Guid guid,
    dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8,
    dc1394framerate_t framerate = DC1394_FRAMERATE_30,
    dc1394speed_t iso_speed = DC1394_ISO_SPEED_400,
    int dma_buffers = 100
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
     * MULTI/ONE SHOT CONTROL
     *-----------------------------------------------------------------------*/
        
    //! check for multi-shot
    bool CheckMultiShotCapable();
        
    //! Turn Multi-shot On
    void SetMultiShotOn(int num_frames);
    
    //! Turn Multi-shot Off
    void SetMultiShotOff();
        
    //! Stop the video stream before using One-shot mode
    void StopForOneShot();

    //! Grab one shot (iso-transmission must be off - call StopForOneShot first)
    bool GrabOneShot(unsigned char* image);
    
    //! grab N frames via multi shot
    void GrabNFramesMulti(unsigned char *image, int n, float shut[]);
    
    //! grab N frames
    void GrabNFrames(unsigned char *image, int n, int shut[]);
        
    //! Check to see if camera is one-shot capable
    bool CheckOneShotCapable();

    /*-----------------------------------------------------------------------
     * REGISTERS - META DATA, HDR & LOOPUP TABLE ETC
     *-----------------------------------------------------------------------*/
        
    //! set the meta data flags to be included in image data
    void SetMetaDataFlags(int flags);
    
    //! return the current meta data flags from camera
    uint32_t GetMetaDataFlags();
    
    //! set HDR register on/off (true/false)
    void SetHDRRegister(bool power);

    //! get HDR register flags
    uint32_t GetHDRFlags();

    //! set HDR shutter flags
    void SetHDRShutterFlags(uint32_t shut0, uint32_t shut1, uint32_t shut2, uint32_t shut3);
    
    //! get HDR shutter flags
    void GetHDRShutterFlags(uint32_t &shut0, uint32_t &shut1, uint32_t &shut2, uint32_t &shut3); 
    
    //! set HDR gain flags
    void SetHDRGainFlags(uint32_t gain0, uint32_t gain1, uint32_t gain2, uint32_t gain3);
    
    //! get HDR Gain flags
    void GetHDRGainFlags(uint32_t &gain0, uint32_t &gain1, uint32_t &gain2, uint32_t &gain3); 
        
    //! read the meta data from an image according to meta flags
    void ReadMetaData( unsigned char *image, MetaData *metaData );
    
    //! return time stamp from image data
    uint32_t ReadTimeStamp( unsigned char *image );
        
    //! read image data and check map/lookup to get absolute shutter value
    float ReadShutter( unsigned char *image );
        
    //! create lookup table to convert quantised shutter values to absolute values
    void CreateShutterLookupTable();
        
    //! create lookup hash maps to convert quantised shutter values to absolute values and vice versa
    void CreateShutterMaps();
    
    //! return quantised shutter value for corresponding absolute value   
    int GetShutterMapQuant(float val);
        
    //! return absolute shutter value for corresponding quantised value
    float GetShutterMapAbs(int val);
    
    /**
     print shutter absolute map <int,float>
     */   
    void PrintShutterMapAbs();
        
    /**
     print shutter quantised map <float,int> 
     */
    void PrintShutterMapQuant();
        
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

    /**
     set all features to auto mode
     @exception dc1394 error
     */
    void SetAllFeaturesAuto();
     
    /**
     set all features to manual mode
     @exception dc1394 error
     */
    void SetAllFeaturesManual();

    /**
     set feature to auto mode
     @param feature
     @exception dc1394 error
     */
    void SetFeatureAuto(dc1394feature_t feature);

    /**
     set feature to manual mode
     @param feature
     @exception dc1394 error
     */
    void SetFeatureManual(dc1394feature_t feature);
    
    /**
     set feature to on
     @param feature
     @exception dc1394 error
     */
    void SetFeatureOn(dc1394feature_t feature);
    
    /**
     set feature to off
     @param feature
     */
    void SetFeatureOff(dc1394feature_t feature);
           
    /**
     set feature absolute value
     @param feature
     @param value
          @exception dc1394 error
     */
    void SetFeatureValue(dc1394feature_t feature, float value);
    
    /**
     set feature quantised value
     @param feature
     @param value
     @exception dc1394 error
    */
    void SetFeatureQuant(dc1394feature_t feature, int value);
    
    /**
     get feature on/off
     @param feature
     @return bool flag (on/off)
     @exception dc1394 error
     */
    bool GetFeaturePower(dc1394feature_t feature);
        
    /**
     get feature enabled mode (auto [0] or manual [1])
     @param feature
     @param mode
     @exception dc1394 error
     */
    int GetFeatureMode(dc1394feature_t feature) const;
        
    /**
     get feature absolute value
     @param feature
     @return absolute value
     @exception dc1394 error
     */
    float GetFeatureValue(dc1394feature_t feature) const;
    
    /**
     get feature quantised value
     @param feature
     @return quantised value
     @exception dc1394 error
     */    
    int GetFeatureQuant(dc1394feature_t feature) const;
    

    /**
     get feature absolute max value
     @param feature
     @return max absolute value
     @exception dc1394 error
     */
    float GetFeatureValueMax(dc1394feature_t feature) const;

    /**
     get feature absolute min value
     @param feature
     @return min absolute value
          @exception dc1394 error
     */
    float GetFeatureValueMin(dc1394feature_t feature) const;
        
    /**
     get feature quantised max value
     @param feature
     @return max quantised value
     @exception dc1394 error
     */
    int GetFeatureQuantMax(dc1394feature_t feature) const;
    
    /**
     get feature quantised min value
     @param feature
     @return min quantised value
     @exception dc1394 error
     */
    int GetFeatureQuantMin(dc1394feature_t feature) const;
    
    /**
     reset brightness to 0
     @exception dc1394 error
     */
    void ResetBrightness();
        
    /**
     reset gamma to 1.0
     @exception dc1394 error
     */
    void ResetGamma();
    
    /**
    reset hue to 0
    @exception dc1394 error
     */
    void ResetHue();

    /*-----------------------------------------------------------------------
     *  WHITE BALANCE CONTROLS
     *-----------------------------------------------------------------------*/
      
    /**
     set white balance to auto mode
     */
    void SetSingleAutoWhiteBalance();
           
    /**
     set white balance blue/U value and red/v value
     @param blue/u value
     @param red/v value
     */
    void SetWhiteBalance(unsigned int u_b_value, unsigned int v_r_value);

    /**
     get white balance blue/U value and red/v value
     @param pass by reference blue/u value 
     @param pass by reference red/v value
     */
    void GetWhiteBalance(unsigned int *Blue_U_val, unsigned int *Red_V_val);
     
    /**
     get white balance blue/U value
     @returns white balance blue/u value
     */
    int GetWhiteBalanceBlueU();
   
    /**
     get white balance red/V value
     @returns white balance red/v value
     */
    int GetWhiteBalanceRedV();
        
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
        
    /**
     record multiple frames
     @param frame number
     @param image buffer
     @param wait or poll for image from dma
     @param jpeg or not
     @param hdr frame or not
     @returns bool flag
     */
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
        
    //! Automatic HDR Frame Capture
    void CaptureHDRFrame(unsigned char* image, int n, uint32_t shutter[]);
        
    //! grab and save indidivdual frame with time stamp 
    void SaveSingleFrame(unsigned char* image);
        
    //! save image file to ppm or jpeg
    bool SaveFile(    
                    int frame_number,           // current frame number
                    dc1394video_frame_t frame,  // frame buffer (copy - threading)
                    const char* path,           // folder name
                    bool jpeg = true            // true = jpeg, false = ppm
                );
    
    //! save normal video
    void SaveVideo();
        
    // saves HDR video 
    void SaveHDRVideo(int frame_number);
        
    //! returns video frame in RGB format (e.g. from YUV to RGB)
    dc1394video_frame_t* ConvertToRGB(dc1394video_frame_t *original_frame);
    
    /*-----------------------------------------------------------------------
     *  REPORTING
     *-----------------------------------------------------------------------*/
    
    //! generate image pixel intensity count in map
    void CreatePixIntensityMap(dc1394video_frame_t frame);
        
    //! accepts dc1394 frame, returns change (multiplier) to shutter time for next frame
    float AEC(dc1394video_frame_t frame, float st, bool under_over); 
        
    //! accepts image buffer, returns change (multiplier) to shutter time for next frame
    float AEC(unsigned char *image, float st, bool under_over);
              
    /*-----------------------------------------------------------------------
     *  CAMERA UTILITIES
     *-----------------------------------------------------------------------*/
    
    //! get camera pointer
    dc1394camera_t *GetCamera() { return camera; }
        
    //! get camera feature set
    dc1394featureset_t *GetFeatures() { return &features; }
    
    //! get video mode
    dc1394video_mode_t *GetVideoMode() { return &video_mode; }
        
    //! get camera vendor
    std::string GetCameraVendor() const { return camera->vendor; }
    
    //! get camera model
    std::string GetCameraModel() const { return camera->model; }
        
    //! print full camera features and current settings report
    void PrintCameraReport();
    
    //! get the best video mode and highest framerate for camera (default colour RGB8)
    void GetBestSettings(dc1394video_mode_t &video_mode, dc1394framerate_t &framerate);
        
    /*-----------------------------------------------------------------------
     *  FILE UTILITIES
     *-----------------------------------------------------------------------*/
        
    //! generate camera response function and save to file
    void GetResponseFunction();
    
    //! check if response function exists
    bool CheckResponseFunction();
        
    //! return number of bytes to skip when scanning frame
    int GetMetaOffset(); 
            
    //! load configuration file
    void LoadConfig();
        
    //! get loaded configuration file attribute value
    std::string GetConfigValue(std::string attribute);  
        
    //! set loaded configuration file attribute value
    void SetConfigValue(std::string attribute, std::string value); 
        
    //! check if config map is loaded
    bool CheckConfigLoaded();
    
    //return time stamped string
    void GetTimeStamp(char* time_stamp);
        
    // return padded frame number
    char* PadNumber(int frame_number);
        
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
    dc1394video_mode_t video_mode;
        
    uint32_t meta_data_flags;
    bool hdr_register; // 1 = on
    
    float* shutter_lookup_table;

    std::map<int,float> shutter_abs_map;
    std::map<float,int> shutter_quant_map; 
        
    std::map<std::string, std::string> config;
        
    };

    }

    #endif // PANGOLIN_FIREWIRE_H

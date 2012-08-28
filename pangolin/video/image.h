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

/** @brief Image functions to create and read PPM and JPEG files natively without using libraries
 
 This header file compartmentalises functionality to create, load and edit PPM and JPEG image files without the use of an external library.
 
 The primary purpose of this is for speed.
 
 @author Hussein, A.
 @date August 2012
 */

#ifndef PANGOLIN_IMAGE_H
#define PANGOLIN_IMAGE_H

#include <pangolin/pangolin.h>
#include <pangolin/video/firewire.h>

#include <dc1394/dc1394.h>
#include <ImageMagick/Magick++.h>
#include <exiv2/exiv2.hpp>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace pangolin
{
    /** 
     create ppm from image buffer 
     @param image buffer
     @param image width
     @param image height
     @param image output filename
     @returns bool value
     */
    bool CreatePPM(unsigned char* image, int width, int height, const char* filename);
    

    /**
     create jpeg from image buffer
     @param image buffer
     @param image width
     @param image height
     @param image output file path
     @returns bool flag
     */
    bool CreateJPEG(unsigned char* image, int width, int height, const char* filename);
    
    /**
     load jpeg in to image buffer
     @param image buffer
     @param image output file path
     @returns bool flag
     */
    bool LoadJPEG(unsigned char* image, const char* filename);

    /**
     copy ppm to jpeg
     @param from file path
     @param to file path
     @exception image magick exception
     */
    void CopyFormatToFormat(const char* from_filename, const char* to_filename);
    
    /**
     write current camera data to exif on jpeg image
     @param video object with specific GUID
     @param to file path
     @exception exiv error
     */
    void WriteExifData(const pangolin::FirewireVideo *video, const std::string& filename);

    /**
     write image metadata to exif on jpeg image
     @param metadata struct read from image
     @param to file path
     @exception exiv error
     */
    void WriteExifDataFromImageMetaData(MetaData *metaData, const std::string& filename);
    
    /**
     copy exif data from one jpeg to another    
     @param from file path
     @param to file path
     @param bool flat to override or not
     @exception exiv error
     */
    void CopyExifData(const std::string& from_filename, const std::string& to_filename,  bool dont_overwrite);
    
    /**
     returns the "average scene luminance" (cd/m^2) from an image file.   
     @param file path
     @return float of "average scene luminance" (cd/m^2) 
     */
    float GetAvgLuminance(const std::string& filename);

    /**
     generates HDRGEN script for pfshdrcalibrate based on image exif data
     @param file path
     @param file
     @param framenumber
     @return bool flag
     @exception exiv error
     */
    bool JpegToHDRGEN(const char* filename, FILE* hdrgen, int frame_number);
    
    /**
     saves image histogram to file
     @param file path
     */
    void SaveImageHistogram(const char* filename);
}

#endif

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

    //! Copy ppm to jpeg
    void CopyFormatToFormat(const char* from_filename, const char* to_filename);
    
    //! write current camera data to exif on jpeg image    
    void WriteExifData(const FirewireVideo* video, const std::string& filename);

    //! copy exif data from one jpeg to another
    void CopyExifData(const std::string& from, const std::string& to,  bool dont_overwrite);

    //! returns the "average scene luminance" (cd/m^2) from an image file.
    float GetAvgLuminance(const std::string& filename);

    //! creates hdrgen script for pfscalibrate based on exif data of image
    bool JpegToHDRGEN(const char* filename, FILE* hdrgen, int frame_number);
    
    //! saves image histogram to file
    void SaveImageHistogram(const char* filename);
}
#endif

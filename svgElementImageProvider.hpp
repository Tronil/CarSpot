/****************************************************************************
**
** Copyright (c) 2012 Troels Nilsson.
** All rights reserved.
** Contact: nilsson.troels@gmail.com
**
** This file is part of the CarSpot project.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 
** Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.
** 
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
** 
** Neither the name of CarSpot nor the names of its contributors may be
** used to endorse or promote products derived from this software without
** specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
** FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/
/* Put licence here */

/* Declarative image provider that will return a specific component/object from an SVG file
   (as opposed to using the whole SVG as an image).

   It needs to be registered in the C++ part of the application by doing this:
   engine->addImageProvider(QLatin1String("svgElement"), new svgElementImageProvider);

   To use the image URL should be in this format:
   "image://svgElement/svgfile/component"
   (assuming it is registered with "svgElement" as the name)
   where svgfile is the SVG filename and component is the component ID.

   It is legal for the filename to contain '/' characters; the last of the '/' characters
   will be interpreted as the separator between filename and element ID.
*/

#ifndef SVGELEMENTIMAGEPROVIDER_HPP
#define SVGELEMENTIMAGEPROVIDER_HPP

#include <QDeclarativeImageProvider>

class svgElementImageProvider : public QDeclarativeImageProvider
{
public:
    svgElementImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requested_size);
};

#endif // svgElementIMAGEPROVIDER_HPP

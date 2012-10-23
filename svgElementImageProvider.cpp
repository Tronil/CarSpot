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

#include "svgElementImageProvider.hpp"
#include <QtSvg/QGraphicsSvgItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

svgElementImageProvider::svgElementImageProvider()
    : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
{
}

QPixmap svgElementImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requested_size)
{
    // element ID is everything after the last '/'
    QString element_id = id.section(QLatin1Char('/'), -1);
    // Filename is everything before that
    QString filename = id.section(QLatin1Char('/'), 0, -2);

    if (id.contains(QLatin1Char('/')) && !element_id.isEmpty() && !filename.isEmpty())
    {
        // Two parts in id string (SVG filename and component ID); all is OK

        QGraphicsSvgItem svg(filename);
        int width;
        int height;
        bool scale;

        svg.setElementId(element_id);
        QRectF bounding_rect = svg.boundingRect();

        if (!requested_size.isEmpty())
        {
            width = requested_size.width();
            height = requested_size.height();
            scale = true;
        }
        else
        {
            // Requested size not valid; use original size given by the bounding rectangle
            width = bounding_rect.width();
            height = bounding_rect.height();
            scale = false;
        }

        QPixmap pixmap(width, height);
        // Pre-fill pixmap with all transparent values
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        QStyleOptionGraphicsItem options;

        // Original size should be returned in size if present
        if (size)
        {
            size->setWidth(bounding_rect.width());
            size->setHeight(bounding_rect.height());
        }

        if (scale)
        {
            painter.scale(width/bounding_rect.width(), height/bounding_rect.height());
        }

        // Fill pixmap with content from SVG file
        svg.paint(&painter, &options);

        // Done painting
        painter.end();

        return pixmap;
    }
    else
    {
        // Something is wrong with the id string; fail by returning the default (empty pixmap)
        return QDeclarativeImageProvider::requestPixmap(id, size, requested_size);
    }

}

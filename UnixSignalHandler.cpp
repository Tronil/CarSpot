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
#include "UnixSignalHandler.hpp"
#include <sys/socket.h>
#include <signal.h>

int UnixSignalHandler::SigIntFD[2];

UnixSignalHandler::UnixSignalHandler(QObject *parent) :
    QObject(parent)
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, SigIntFD) == -1)
    {
        qFatal("Couldn't create SIGINT socketpair");
    }

    m_SNSigInt = new QSocketNotifier(SigIntFD[1], QSocketNotifier::Read, this);
    connect(m_SNSigInt, SIGNAL(activated(int)), this, SLOT(handleSigInt()));

    installSignalHandler();
}

void UnixSignalHandler::installSignalHandler()
{
    struct sigaction sigint;

    sigint.sa_handler = UnixSignalHandler::sigIntSignalHandler;
    sigemptyset(&sigint.sa_mask);
    sigint.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sigint, 0) > 0)
        qFatal("Couldn't install signal handler!");
}

void UnixSignalHandler::sigIntSignalHandler(int unused)
{
    // This is the actual POSIX/Unix signal handler; note that only a few
    // function calls are legal here, so watch it
    Q_UNUSED(unused);

    // Use the socket pair to asynchronously signal the application
    char dummy = 0;
    write(SigIntFD[0], &dummy, sizeof(dummy));
}

void UnixSignalHandler::handleSigInt()
{
    // Read the dummy byte out from the socket
    m_SNSigInt->setEnabled(false);
    char dummy;
    read(SigIntFD[1], &dummy, sizeof(dummy));

    // emit SIGINT, now converted to Qt signal
    emit SigInt();

    m_SNSigInt->setEnabled(true);
}

/***************************************************************************
 *   Copyright (C) 2006-2007 Laurentiu-Gheorghe Crisan                     *
 *   Copyright (C) 2006-2007 Marc Boris Duerner                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction. Specifically, if other files    *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License. This exception does not however invalidate any other         *
 *   reasons why the executable file might be covered by the GNU Library   *
 *   General Public License.                                               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "iodeviceimpl.h"
#include "cxxtools/ioerror.h"
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/select.h>

namespace cxxtools {

const short IODeviceImpl::POLLERR_MASK= POLLERR | POLLHUP | POLLNVAL;
const short IODeviceImpl::POLLIN_MASK= POLLIN;
const short IODeviceImpl::POLLOUT_MASK= POLLOUT;

IODeviceImpl::IODeviceImpl(IODevice& device)
: _device(device)
, _fd(-1)
, _timeout(Selectable::WaitInfinite)
, _pfd(0)
{ }



IODeviceImpl::~IODeviceImpl()
{
    assert(_pfd == 0);
}


void IODeviceImpl::open(const std::string& path, IODevice::OpenMode mode)
{
    int flags = O_RDONLY;

    if( (mode & IODevice::Read ) && (mode & IODevice::Write) )
    {
        flags |= O_RDWR;
    }
    else if(mode & IODevice::Write)
    {
        flags |= O_WRONLY;
    }
    else if(mode & IODevice::Read  )
    {
        flags |= O_RDONLY;
    }

    if(mode & IODevice::Async)
        flags |= O_NONBLOCK;

    if(mode & IODevice::Trunc)
        flags |= O_TRUNC;

    flags |=  O_NOCTTY;

    _fd = ::open( path.c_str(), flags );
    if(_fd == -1)
        throw AccessFailed("open failed", CXXTOOLS_SOURCEINFO);
}


void IODeviceImpl::open(int fd, bool isAsync)
{
    _fd = fd;

    if(isAsync)
    {
        int flags = fcntl(_fd, F_GETFL);
        flags |= O_NONBLOCK ;
        int ret = fcntl(_fd, F_SETFL, O_NONBLOCK);
        if(-1 == ret)
            throw IOError("Could not set fd to non-blocking.", CXXTOOLS_SOURCEINFO);
    }
}


void IODeviceImpl::close()
{
    if(_fd != -1)
    {
        if( ::close(_fd) != 0 )
            throw IOError("Could not close file handle", CXXTOOLS_SOURCEINFO);

        _fd = -1;
        _pfd = 0;
    }
}


size_t IODeviceImpl::beginRead(char* buffer, size_t n, bool& eof)
{
     if(_pfd)
     {
         _pfd->events |= POLLIN;
     }

     return 0;
}


size_t IODeviceImpl::endRead(bool& eof)
{
     if(_pfd)
     {
         _pfd->events &= ~POLLIN;
     }

    return this->read( _device.rbuf(), _device.rbuflen(), eof );
}


size_t IODeviceImpl::read( char* buffer, size_t count, bool& eof )
{
    ssize_t ret = 0;

    while(true)
    {
        ret = ::read( _fd, (void*)buffer, count);
        if(ret > 0)
            break;

        if(ret == 0 || errno == ECONNRESET)
        {
            eof = true;
            return 0;
        }

        if(errno == EINTR)
            continue;

        if(errno != EAGAIN)
            throw IOError("read failed", CXXTOOLS_SOURCEINFO);

        pollfd pfd;
        pfd.fd = this->fd();
        pfd.revents = 0;
        pfd.events = POLLIN;

        bool ret = this->wait(_timeout, pfd);
        if(false == ret)
        {
            throw IOTimeout();
        }
    }

    return ret;
}


size_t IODeviceImpl::beginWrite(const char* buffer, size_t n)
{
     if(_pfd)
     {
         _pfd->events |= POLLOUT;
     }

     return 0;
}


size_t IODeviceImpl::endWrite()
{
     if(_pfd)
     {
         _pfd->events &= ~POLLOUT;
     }

    return this->write( _device.wbuf(), _device.wbuflen() );
}


size_t IODeviceImpl::write( const char* buffer, size_t count )
{
    ssize_t ret = 0;

    while(true)
    {
        ret = ::write(_fd, (const void*)buffer, count);
        if(ret > 0)
            break;

        if(ret == 0 || errno == ECONNRESET || errno == EPIPE)
            return 0;

        if(errno == EINTR)
            continue;

        if(errno != EAGAIN)
            throw IOError("Could not read from file handle", CXXTOOLS_SOURCEINFO);

        pollfd pfd;
        pfd.fd = this->fd();
        pfd.revents = 0;
        pfd.events = POLLOUT;

        bool ret = this->wait(_timeout, pfd);
        if(false == ret)
        {
            throw IOTimeout();
        }
    }

    return ret;
}


void IODeviceImpl::sync() const
{
    int ret = fsync(_fd);
    if(ret != 0)
        throw IOError("Could not sync handle", CXXTOOLS_SOURCEINFO);
}


void IODeviceImpl::attach(SelectorBase& s)
{

}


void IODeviceImpl::detach(SelectorBase& s)
{
    if(_pfd)
        _pfd = 0;
}


bool IODeviceImpl::wait(std::size_t msecs)
{
    pollfd pfd;
    this->initWait(pfd);
    this->wait(msecs);
    return this->checkPollEvent();
}


bool IODeviceImpl::wait(std::size_t umsecs, pollfd& pfd)
{
    int msecs = umsecs;
    if( umsecs > std::numeric_limits<int>::max() )
    {
        umsecs == SelectorBase::WaitInfinite ? -1
                                             : std::numeric_limits<int>::max();
    }

    int ret = -1;
    while( true )
    {
        ret = ::poll(&pfd, 1, msecs);
        if( ret != -1 )
            break;

        if( errno != EINTR )
            throw IOError( "poll failed", CXXTOOLS_SOURCEINFO );
    }

    return ret > 0;
}


void IODeviceImpl::initWait(pollfd& pfd)
{
    pfd.fd = this->fd();
    pfd.revents = 0;
    pfd.events = 0;

    if( _device.rbuf() )
        pfd.events |= POLLIN;
    if( _device.wbuf() )
        pfd.events |= POLLOUT;
}


size_t IODeviceImpl::initializePoll(pollfd* pfd, size_t pollSize)
{
    assert(pfd != 0);
    assert(pollSize >= 1);

    this->initWait(*pfd);
    _pfd = pfd;

	return 1;
}


bool IODeviceImpl::checkPollEvent()
{
    bool avail = false;

    if (_pfd->revents & POLLERR_MASK)
    {
        _device.errorOccured(_device);
        avail = true;
    }

    if( _pfd->revents & POLLOUT_MASK )
    {
        _device.outputReady(_device);
        avail = true;
    }

    if( _pfd->revents & POLLIN_MASK )
    {
        _device.inputReady(_device);
        avail = true;
    }

    return avail;
}

}

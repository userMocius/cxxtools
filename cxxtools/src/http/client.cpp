/*
 * Copyright (C) 2009 by Marc Boris Duerner, Tommi Maekitalo
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public License.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <cxxtools/http/client.h>
#include "clientimpl.h"

namespace cxxtools {

namespace http {

Client::Client()
: _impl(new ClientImpl(this))
{
}


Client::Client(const std::string& server, unsigned short int port)
: _impl(new ClientImpl(this, server, port))
{
}


Client::Client(SelectorBase& selector, const std::string& server, unsigned short int port)
: _impl(new ClientImpl(this, selector, server, port))
{
}

Client::~Client()
{
  delete _impl;
}

void Client::connect(const std::string& server, unsigned short int port)
{
    _impl->connect(server, port);
}

const ReplyHeader& Client::execute(const Request& request, std::size_t timeout)
{
    return _impl->execute(request, timeout);
}

const ReplyHeader& Client::header()
{
    return _impl->header();
}

void Client::readBody(std::string& s)
{
    _impl->readBody(s);
}

std::string Client::get(const std::string& url, std::size_t timeout)
{
    return _impl->get(url, timeout);
}

void Client::beginExecute(const Request& request)
{
    _impl->beginExecute(request);
}

void Client::setSelector(SelectorBase& selector)
{
    _impl->setSelector(selector);
}

void Client::wait(std::size_t msecs)
{
    _impl->wait(msecs);
}

std::istream& Client::in()
{
    return _impl->in();
}

const std::string& Client::server() const
{
    return _impl->server();
}

unsigned short int Client::port() const
{
    return _impl->port();
}

void Client::auth(const std::string& username, const std::string& password)
{
    _impl->auth(username, password);
}

void Client::clearAuth()
{
    _impl->clearAuth();
}

} // namespace http

} // namespace cxxtools

/*
 * Copyright (C) 2011 Tommi Maekitalo
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

#ifndef CXXTOOLS_REMOTECLIENT_H
#define CXXTOOLS_REMOTECLIENT_H

#include <cstddef>

namespace cxxtools
{
    class IComposer;
    class IDecomposer;
    class IRemoteProcedure;

    /// Base class for rpc client.
    class RemoteClient
    {
        public:
            static const std::size_t WaitInfinite = static_cast<std::size_t>(-1);

            virtual ~RemoteClient()
            { }

            virtual void beginCall(IComposer& r, IRemoteProcedure& method, IDecomposer** argv, unsigned argc) = 0;

            virtual void endCall() = 0;

            virtual void call(IComposer& r, IRemoteProcedure& method, IDecomposer** argv, unsigned argc) = 0;

            virtual const IRemoteProcedure* activeProcedure() const = 0;

            virtual void cancel() = 0;

            virtual void wait(std::size_t msecs = WaitInfinite) = 0;

            virtual std::size_t timeout() const = 0;
            virtual void timeout(std::size_t t) = 0;

            virtual std::size_t connectTimeout() const = 0;
            virtual void connectTimeout(std::size_t t) = 0;

    };
}

#endif // CXXTOOLS_REMOTECLIENT_H

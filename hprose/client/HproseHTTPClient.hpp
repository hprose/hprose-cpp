/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.net/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/
/**********************************************************\
 *                                                        *
 * HproseHTTPClient.hpp                                   *
 *                                                        *
 * hprose default http client unit for Cpp.               *
 *                                                        *
 * LastModified: Mar 2, 2014                              *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_CLIENT_HPROSE_HTTPCLIENT_HPP
#define HPROSE_CLIENT_HPROSE_HTTPCLIENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <hprose/client/asio/HproseHTTPClient.hpp>

namespace hprose {
    using asio::HproseHTTPClient;
}

#endif // HPROSE_CLIENT_HPROSE_HTTPCLIENT_HPP

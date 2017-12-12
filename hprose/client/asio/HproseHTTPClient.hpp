/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/
/**********************************************************\
 *                                                        *
 * HproseHTTPClient.hpp                                   *
 *                                                        *
 * hprose asio http client class unit for Cpp.            *
 *                                                        *
 * LastModified: Mar 2, 2014                              *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_CLIENT_ASIO_HPROSE_HTTPCLIENT_HPP
#define HPROSE_CLIENT_ASIO_HPROSE_HTTPCLIENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <hprose/client/HproseClient.hpp>
#include <hprose/client/CookieManager.hpp>

#include <boost/asio.hpp>
#ifndef HPROSE_NO_OPENSSL
#include <boost/asio/ssl.hpp>
#endif
#include <boost/thread/mutex.hpp>

namespace hprose { namespace asio {

using boost::asio::ip::tcp;

class HproseHTTPClient : public HproseClient {
public:

    HproseHTTPClient(const std::string & uri = "http://localhost/") {
        UseService(uri);
        SetKeepAlive(true);
        SetHeader("Content-Type", "application/hprose");
        SetUserAgent("Hprose Http Client for Cpp (asio)");
    };

    virtual ~HproseHTTPClient() {
        boost::mutex::scoped_lock lock(mutex);
        while (!pool.empty()) {
            HTTPContext * context = pool.top();
            pool.pop();
            delete context;
        }
    };

public:

    virtual void UseService(const std::string & uri) {
        HproseClient::UseService(uri);
        ParseURI();
    };

    inline void SetHeader(const std::string & key, const std::string & value) {
        headers[key] = value;
    };

    void SetKeepAlive(bool keepAlive, int timeout = 300) {
        if (keepAlive) {
            std::ostringstream stream;
            stream << "keep-alive\r\nKeep-Alive: " << timeout;
            SetHeader("Connection", stream.str());
        } else {
            SetHeader("Connection", "close");
        }
    };

    inline void SetUserAgent(const std::string & userAgent) {
        headers["User-Agent"] = userAgent;
    };

protected:

    virtual void * GetInvokeContext() {
        boost::mutex::scoped_lock lock(mutex);
        if (pool.size() > 0) {
            HTTPContext * ctx = pool.top();
            pool.pop();
            return ctx;
        } else {
            return new HTTPContext(ios);
        }
    };

    virtual void SendData(void * context) {
        if (context) {
            HTTPContext & ctx = *static_cast<HTTPContext *>(context);
            ctx.Post(host, port, path, headers, protocol == "https");
        }
    };

    virtual void EndInvoke(void * context) {
        if (context) {
            boost::mutex::scoped_lock lock(mutex);
            HTTPContext * ctx = static_cast<HTTPContext *>(context);
            pool.push(ctx);
        }
    };

    virtual std::ostream & GetOutputStream(void * context) {
        if (context) {
            return (static_cast<HTTPContext *>(context))->requestStream;
        } else {
            HPROSE_THROW_EXCEPTION("Can''t get output stream.");
        }
    };

    virtual std::istream & GetInputStream(void * context) {
        if (context) {
            return (static_cast<HTTPContext *>(context))->responseStream;
        } else {
            HPROSE_THROW_EXCEPTION("Can''t get input stream.");
        }
    };

private:

    void ParseURI() {
        std::string surl;
        std::string::size_type x, y;
        protocol = "http";
        user = "";
        password = "";
        port = "80";
        x = uri.find("://");
        if (x != std::string::npos) {
            protocol = uri.substr(0, x);
            surl = uri.substr(x + 3);
        } else {
            surl = uri;
        }
        std::transform(protocol.begin(), protocol.end(), protocol.begin(), tolower);
        if (protocol == "https") {
            port = "443";
        }
        x = surl.find('@');
        y = surl.find('/');
        if ((x != std::string::npos) && ((x < y) || (y == std::string::npos))) {
            user = surl.substr(0, x);
            surl.erase(0, x + 1);
            x = user.find(':');
            if (x != std::string::npos) {
                password = user.substr(x + 1);
                user.erase(x);
            }
        }
        x = surl.find('/');
        if (x != std::string::npos) {
            host = surl.substr(0, x);
            surl.erase(0, x + 1);
        } else {
            host = surl;
            surl = "";
        }
        bool ipv6 = host[0] == '[';
        if (ipv6) {
            x = host.find(']');
            if ((x + 1 < host.size()) && (host[x + 1] == ':')) {
                port = host.substr(x + 2);
            }
            host = host.substr(1, x - 1);
        } else {
            x = host.find(':');
            if (x != std::string::npos) {
                port = host.substr(x + 1);
                host.erase(x);
            }
        }
        SetHeader("Host", (ipv6 ? ('[' + host + ']') : host) + ((port == "80") ? std::string() : (':' + port)));
        
        path = '/' + surl;
        
        if (host == "") {
            host = "localhost";
        }
    };

private:

    class HTTPContext {
    public: // structors

        HTTPContext(boost::asio::io_service & ios) 
            : resolver(ios),
            socket(ios),
#ifndef HPROSE_NO_OPENSSL
            sslContext(ios, boost::asio::ssl::context::sslv23),
            sslSocket(ios, sslContext),
#endif
            requestStream(&request),
            responseStream(&response) {
        };

    public:

        void Post(const std::string & host, const std::string & port, const std::string & path, const std::map<std::string, std::string> & headers, bool secure) {
            if (!Connect(
#ifndef HPROSE_NO_OPENSSL
                secure ?
                sslSocket.next_layer() :
#endif
                socket, host, port, secure)) return;
            boost::system::error_code error = boost::asio::error::host_not_found;
            boost::asio::streambuf header;
            std::iostream headerStream(&header);
            headerStream << "POST " << ((path == "/*") ? std::string("*") : path) << " HTTP/1.1\r\n";;
            for (std::map<std::string, std::string>::const_iterator iter = headers.begin(); iter != headers.end(); iter++) {
                headerStream << iter->first << ": " << iter->second << "\r\n";
            }
            std::string cookie = CookieManager::SharedInstance()->GetCookie(aliveHost, path, secure);
            if (!cookie.empty()) {
                headerStream << "Cookie: " << cookie << "\r\n";
            }
            headerStream << "Content-Length: " << request.size() << "\r\n\r\n";
            if (request.size() >= 64 * 1024) {
                Write(header, secure);
                Write(request, secure);
            } else {
                headerStream << &request;
                Write(header, secure);
            }
            if (response.size()) {
                response.consume(response.size());
            }
            std::string s;
            size_t bytes = 0, len = 0;
            bool toclose = false, chunked = false;
            while ((bytes = ReadLine(header, secure)) > 2) {
                headerStream >> s;
                if (_strcmpi(s.c_str(), "Content-Length:") == 0) {
                    headerStream >> len;
                } else if (_strcmpi(s.c_str(), "Connection:") == 0) {
                    headerStream >> s;
                    if (_strcmpi(s.c_str(), "close") == 0) {
                        toclose = true;
                    }
                } else if (_strcmpi(s.c_str(), "Keep-Alive:") == 0) {
                    std::getline(headerStream, s, '=');
                    if (_strcmpi(s.c_str(), " timeout") == 0) {
                        clock_t timeout;
                        headerStream >> timeout;
                        aliveTime = clock() + timeout * CLOCKS_PER_SEC;
                    }
                } else if (_strcmpi(s.c_str(), "Transfer-Encoding:") == 0) {
                    headerStream >> s;
                    if (_strcmpi(s.c_str(), "chunked") == 0) {
                        chunked = true;
                    }
                } else if ((_strcmpi(s.c_str(), "Set-Cookie:") == 0) || (_strcmpi(s.c_str(), "Set-Cookie2:") == 0)) {
                    std::getline(headerStream, s);
                    CookieManager::SharedInstance()->SetCookie(aliveHost, s.substr(1, s.size() - 2));
                    continue;
                }
                headerStream.ignore((std::numeric_limits<int>::max)(), '\n');
            }
            header.consume(2);
            if (chunked) {
                while (true) {
                    size_t chunk_size = 0;
                    ReadLine(header, secure);
                    headerStream >> std::hex >> chunk_size >> std::dec;
                    header.consume(2);
                    if (chunk_size) {
                        bytes = 0;
                        while (true) {
                            bytes += ReadLine(header, secure);
                            if (bytes > chunk_size) {
                                for (size_t i = 0; i < chunk_size; i++) {
                                    responseStream << (char)headerStream.get();
                                };
                                header.consume(2);
                                break;
                            }
                        }
                    } else {
                        header.consume(2);
                        break;
                    }
                }
            } else {
                bool nosize = !len;
                size_t n = std::min<size_t>(len, header.size());
                for (size_t i = 0; i < n; ++i, --len) {
                    responseStream << (char)headerStream.get();
                };
                if (nosize) {
                    len = (std::numeric_limits<int>::max)();
                }
                if (len) {
                    char buf[1024];
                    while (len) {
                        size_t n =
#ifndef HPROSE_NO_OPENSSL
                            secure ?
                            sslSocket.read_some(boost::asio::buffer(buf), error) :
#endif
                            socket.read_some(boost::asio::buffer(buf), error);
                        if (error) break;
                        responseStream.write(buf, n);
                        len -= n;
                    }
                }
            }
            if (toclose) {
                (
#ifndef HPROSE_NO_OPENSSL
                    secure ?
                    sslSocket.next_layer() :
#endif
                    socket).close();
                Clear();
            }
        };

    private:

        inline void Clear() {
            aliveHost.clear();
            alivePort.clear();
            aliveTime = 0;
        };

        bool Connect(tcp::socket & socket, const std::string & host, const std::string & port, bool secure) {
            if (socket.is_open() && (aliveHost == host) && (alivePort == port) && (clock() < aliveTime)) {
                return true;
            }
            tcp::resolver::query query(host, port);
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            tcp::resolver::iterator end;
            boost::system::error_code error = boost::asio::error::host_not_found;
            while (error && endpoint_iterator != end) {
                socket.close();
                socket.connect(*endpoint_iterator++, error);
            }
            if (error) {
                Clear();
                return false;
            } else {
#ifndef HPROSE_NO_OPENSSL
                if (secure) {
                    sslSocket.handshake(boost::asio::ssl::stream_base::client, error);
                    if (error) {
                        socket.close();
                        return false;
                    }
                }
#endif
                aliveHost = host;
                alivePort = port;
                return true;
            }
        };
 
        void Write(boost::asio::streambuf & buf, bool secure) {
#ifndef HPROSE_NO_OPENSSL
            if (secure) {
                boost::asio::write(sslSocket, buf);
            } else {
#endif
                boost::asio::write(socket, buf);
#ifndef HPROSE_NO_OPENSSL
            }
#endif
        };

        size_t ReadLine(boost::asio::streambuf & buf, bool secure) {
            return
#ifndef HPROSE_NO_OPENSSL
                secure ?
                boost::asio::read_until(sslSocket, buf, "\r\n") :
#endif
                boost::asio::read_until(socket, buf, std::string("\r\n"));
        };

    private:

        std::string aliveHost;
        std::string alivePort;
        clock_t aliveTime;

        tcp::resolver resolver;
        tcp::socket socket;

    #ifndef HPROSE_NO_OPENSSL
        boost::asio::ssl::context sslContext;
        boost::asio::ssl::stream<tcp::socket> sslSocket;
    #endif

        boost::asio::streambuf request;
        boost::asio::streambuf response;

    public:

        std::iostream requestStream;
        std::iostream responseStream;

    };

private:

    std::string protocol;
    std::string user;
    std::string password;
    std::string host;
    std::string port;
    std::string path;

    std::map<std::string, std::string> headers;
    std::stack<HTTPContext *> pool;
    boost::mutex mutex;
    boost::asio::io_service ios;

}; // class HproseHTTPClient

} } // namespaces

#endif

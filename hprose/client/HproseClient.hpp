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
 * HproseClient.hpp                                       *
 *                                                        *
 * hprose client class for Cpp.                           *
 *                                                        *
 * LastModified: Jul 7, 2015                              *
 * Author: Chen fei <cf@hprose.com>                       *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_CLIENT_HPROSE_CLIENT_HPP
#define HPROSE_CLIENT_HPROSE_CLIENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <hprose/io.hpp>
#include <boost/thread.hpp>

namespace hprose {

namespace HproseTags {

const char ResultTags[4] = { TagResult, TagArgument, TagError, TagEnd }; // Todo: Remove

} // namespace HproseTags

class HproseClient {
protected: // structors

    HproseClient() {
    }

    HproseClient(const std::string & uri) {
        UseService(uri);
    }

    virtual ~HproseClient() {
    }

protected:

    virtual void * GetInvokeContext() = 0;

    virtual void SendData(void * context) = 0;

    virtual void EndInvoke(void * context) = 0;

    virtual std::ostream & GetOutputStream(void * context) = 0;

    virtual std::istream & GetInputStream(void * context) = 0;

public:

    virtual void UseService(const std::string & uri) {
        this->uri = uri;
    }

    template<typename ReturnType>
    inline ReturnType Invoke(const std::string & name) {
        std::vector<Any> args;
        return Invoke<ReturnType>(name, args);
    }

    template<typename ReturnType>
    inline void Invoke(ReturnType & ret, const std::string & name) {
        std::vector<Any> args;
        Invoke(ret, name, args);
    }

#ifndef BOOST_NO_INITIALIZER_LISTS
    template<typename ReturnType, typename ArgType>
    inline ReturnType Invoke(const std::string & name, const std::initializer_list<ArgType> & args) {
        return Invoke<ReturnType>(name, args, false);
    }

    template<typename ReturnType, typename ArgType>
    inline void Invoke(ReturnType & ret, const std::string & name, const std::initializer_list<ArgType> & args) {
        Invoke(ret, name, args, false);
    }
#endif

    template<typename ReturnType, typename ArgsType>
    inline ReturnType Invoke(const std::string & name, ArgsType & args, bool ref = false) {
        ReturnType ret = ReturnType();
        Invoke(ret, name, args, ref);
        return ret;
    }

    template<typename ReturnType, typename ArgsType>
    void Invoke(ReturnType & ret, const std::string & name, ArgsType & args, bool ref = false) {
        std::string error;
        void * context = 0;
        context = GetInvokeContext();
        try {
            DoOutput(name, args, ref, GetOutputStream(context));
            SendData(context);
            DoInput(ret, args, error, GetInputStream(context));
        }
        catch (...) {
        }
        EndInvoke(context);
        if (!error.empty()) {
            HPROSE_THROW_EXCEPTION(error);
        }
    }

    template<typename ReturnType, typename Functor>
    inline void AsyncInvoke(const std::string & name, Functor func) {
        static std::vector<Any> args;
        AsyncInvoke<ReturnType>(name, args, func, false);
    }

    template<typename ReturnType, typename ArgsType, typename Functor>
    inline void AsyncInvoke(const std::string & name, const ArgsType & args, Functor func, bool ref = false) {
        boost::thread thread(Async<ReturnType, ArgsType, Functor>(*this, name, args, func, ref));
    }
    
    template<typename ReturnType, typename ArgsType, typename Functor, size_t ArraySize>
    inline void AsyncInvoke(const std::string & name, const ArgsType (&args)[ArraySize], Functor func, bool ref = false) {
        std::vector<ArgsType> newArgs(ArraySize);
        for (int i = 0; i < ArraySize; ++i) {
            newArgs[i] = args[i];
        }
        AsyncInvoke<ReturnType>(name, newArgs, func, ref);
    }

private:

    template<typename ReturnType, typename ArgsType, typename Functor>
    class Async {
    public:

        Async(HproseClient & client, const std::string & name, ArgsType args, Functor func, bool ref)
            : client(client), name(name), args(args), func(func), ref(ref) {
        }

    public:

        inline void operator()() {
            ReturnType ret = ReturnType();
            client.Invoke(ret, name, args, ref);
            func(ret, args);
        }

    private:

        HproseClient & client;
        std::string name;
        ArgsType args;
        Functor func;
        bool ref;

    }; // class Async

private:

    template<typename ReturnType, typename ArgsType>
    void DoInput(ReturnType & ret, ArgsType & args, std::string & error, std::istream & stream) {
        HproseReader reader(stream);
        while (true) {
            switch (reader.CheckTags(HproseTags::ResultTags)) {
                case HproseTags::TagResult:
                    ret = reader.Unserialize<ReturnType>();
                    break;
                case HproseTags::TagArgument:
                    //args = reader.ReadList<ArgsType>();
                    break;
                case HproseTags::TagError:
                    error = reader.ReadString();
                    return;
                case HproseTags::TagEnd:
                    return;
            }
        }
    }

    template<typename ArgsType>
    void DoOutput(const std::string & name, ArgsType & args, bool ref, std::ostream & stream) {
        HproseWriter writer(stream);
        stream << HproseTags::TagCall;
        writer.WriteString(name, false);
        if (!args.empty()) {
            writer.WriteList(args, false);
            if (ref) {
                writer.WriteBool(true);
            }
        }

        stream << HproseTags::TagEnd;
    }

    template<typename ArgsType, size_t ArraySize>
    void DoOutput(const std::string & name, ArgsType (&args)[ArraySize], bool ref, std::ostream & stream) {
        HproseWriter writer(stream);
        stream << HproseTags::TagCall;
        writer.WriteString(name, false);
        if (!args.empty()) {
            writer.WriteList(args, false);
            if (ref) {
                writer.WriteBool(true);
            }
        }

        stream << HproseTags::TagEnd;
    }

protected:

    std::string uri;

}; // class HproseClient

} // namespace hprose

#endif // HPROSE_CLIENT_HPROSE_CLIENT_HPP

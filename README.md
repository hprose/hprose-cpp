# Hprose for C++

[![Join the chat at https://gitter.im/hprose/hprose-cpp](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/hprose/hprose-cpp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

*Hprose* is a High Performance Remote Object Service Engine.

It is a modern, lightweight, cross-language, cross-platform, object-oriented, high performance, remote dynamic communication middleware. It is not only easy to use, but powerful. You just need a little time to learn, then you can use it to easily construct cross language cross platform distributed application system.

*Hprose* supports many programming languages, for example:

* AAuto Quicker
* ActionScript
* ASP
* C++
* Dart
* Delphi/Free Pascal
* dotNET(C#, Visual Basic...)
* Golang
* Java
* JavaScript
* Node.js
* Objective-C
* Perl
* PHP
* Python
* Ruby
* ...

Through *Hprose*, You can conveniently and efficiently intercommunicate between those programming languages.

This project is the implementation of Hprose for C++.


Tips:
1.cpp版依赖Boost，需要额外编译

2.boost编译，安装
  官方 windows/unix-variants 安装文档
  - C:\boost_1_62_0\more\getting_started\windows.html
  - C:\boost_1_62_0\more\getting_started\unix-variants.html
  
3.需要加上宏，

* #define HPROSE_CHARSET_UTF8  //字符用UTF8编码
* #define HPROSE_NO_OPENSSL    //取消OpenSSL依赖（这样不用ASIO不用依赖OpenSSL）,  当然将来server端想用OpenSSL的话，可以自己编译进去


================
另外发展问题，将来cpp版本不会再打算维护，

将来社区重心会放在[Hprose-cpp1x](https://github.com/hprose/hprose-cpp1x)

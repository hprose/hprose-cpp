#include <iostream>
#include "hprose/ext/CTime.hpp"
#include "hprose/client.hpp"

using namespace std;
using namespace hprose;

HproseHTTPClient client("http://www.hprose.com/example/");

boost::shared_mutex print_mutex;

void println(string msg) {
    boost::unique_lock<boost::shared_mutex> lock(print_mutex);
    cout << msg << endl;
}

void callback(string result, vector<string> args) {
    println(result);
}

struct handler {
    void operator()(string result, vector<string> args) {
        println(result);
    }
};

void hello1() {
    string args[] = {"world"};
    cout << client.Invoke<string>("hello", args) << endl;
}

void hello2() {
    string args[] = {"asynchronous world 2"};
    client.AsyncInvoke<string>("hello", args, callback);
}

void hello3() {
    vector<string> args;
    args.push_back("asynchronous world 3");
    client.AsyncInvoke<string>("hello", args, handler());
}

int main() {
    cout << "hello1 start ............" << endl;
    hello1();
    cout << "hello2 start 5 times ............" << endl;
    for (int i = 0; i < 5; ++i) {
        hello2();
    }
    cout << "hello3 start 5 times ............" << endl;
    for (int i = 0; i < 5; ++i) {
        hello3();
    }
    int i;
    cin >> i;
    return 0;
}

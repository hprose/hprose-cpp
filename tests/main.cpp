#include <iostream>
#include "hprose/ext/CTime.hpp"
#include "hprose/client.hpp"

using namespace std;
using namespace hprose;

HproseHTTPClient client1("http://www.hprose.com/example/");

void callback(string result, string (&args)[1]) {
    cout << result << endl;
}

struct handler {
    void operator()(string result, string (&args)[1]) {
        cout << result << endl;
    }
};

void hello1() {
    string args[] = {"world"};
    cout << client1.Invoke<string>("hello", args) << endl;
}

void hello2() {
    string args[] = {"asynchronous world 2"};
    client1.AsyncInvoke<string>("hello", args, callback);
}

void hello3() {
    string args[] = {"asynchronous world 3"};
    client1.AsyncInvoke<string>("hello", args, handler());
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

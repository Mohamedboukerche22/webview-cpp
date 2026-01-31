#include "lib/webviewer.hpp"
using namespace SimpleWeb;

// simple example
int main() {
    auto browser = BrowserWindow(1024, 768, "My Browser");
    
    browser.loadHTML(R"(
        <html>
        <head><title>Hello</title></head>
        <body style="background:#2c3e50;color:white;text-align:center;padding-top:100px">
            <h1>Hello from SimpleWeb!</h1>
            <p>This is an HTML page loaded from C++</p>
        </body>
        </html>
    )");
    
    browser.run();
    return 0;
}

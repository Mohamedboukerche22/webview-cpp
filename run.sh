g++ -std=c++17 "code.cpp" -o "web" `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1`     -O2 -Wall -Wextra && ./web

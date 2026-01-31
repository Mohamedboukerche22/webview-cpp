#ifndef SIMPLE_WEBKIT_HPP
#define SIMPLE_WEBKIT_HPP

//#include<bits/stdc++.h>
// not the best choise in c++ big programs lol
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include <string>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <map>

namespace SimpleWeb {

class BrowserWindow {
private:
    GtkWindow* window;
    WebKitWebView* webView;
    int width, height;
    std::string title;
    bool running;
    
    std::function<void(BrowserWindow*)> onLoadStart;
    std::function<void(BrowserWindow*)> onLoadFinish;
    std::function<void(BrowserWindow*, const std::string&)> onTitleChange;

public:
    BrowserWindow(int w = 800, int h = 600, std::string t = "Simple Browser") 
        : width(w), height(h), title(t), running(false), window(nullptr), webView(nullptr) {
        
        if (!gtk_init_check(nullptr, nullptr)) {
            std::cerr << "Failed to initialize GTK\n";
            return;
        }
        
        window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
        gtk_window_set_title(window, title.c_str());
        gtk_window_set_default_size(window, width, height);
        
        webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
        gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(webView));
        
        g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer data) {
            auto* self = static_cast<BrowserWindow*>(data);
            self->running = false;
            gtk_main_quit();
        }), this);
        
        g_signal_connect(webView, "load-changed", G_CALLBACK(+[](WebKitWebView* view, WebKitLoadEvent event, gpointer data) {
            auto* self = static_cast<BrowserWindow*>(data);
            if (event == WEBKIT_LOAD_STARTED && self->onLoadStart) {
                self->onLoadStart(self);
            } else if (event == WEBKIT_LOAD_FINISHED && self->onLoadFinish) {
                self->onLoadFinish(self);
            }
        }), this);
        
        g_signal_connect(webView, "notify::title", G_CALLBACK(+[](GObject* obj, GParamSpec*, gpointer data) {
            auto* self = static_cast<BrowserWindow*>(data);
            if (self->onTitleChange) {
                const char* newTitle = webkit_web_view_get_title(WEBKIT_WEB_VIEW(obj));
                if (newTitle) {
                    self->onTitleChange(self, std::string(newTitle));
                }
            }
        }), this);
        
        gtk_widget_show_all(GTK_WIDGET(window));
    }
    
    ~BrowserWindow() {
        if (window) {
            gtk_widget_destroy(GTK_WIDGET(window));
        }
    }
    
    void loadHTML(const std::string& html) {
        if (webView) {
            webkit_web_view_load_html(webView, html.c_str(), nullptr);
        }
    }
    
    void loadURL(const std::string& url) {
        if (webView) {
            webkit_web_view_load_uri(webView, url.c_str());
        }
    }
    
    void loadFile(const std::string& filepath) {
        if (webView) {
            std::string uri;
            if (filepath.find("://") != std::string::npos) {
                uri = filepath;
            } else {
                std::filesystem::path path = std::filesystem::absolute(filepath);
                uri = "file://" + path.string();
            }
            webkit_web_view_load_uri(webView, uri.c_str());
        }
    }
    
    void run() {
        running = true;
        gtk_main();
    }
    
    void close() {
        if (window) {
            running = false;
            gtk_main_quit();
        }
    }
    
    void setFullscreen(bool full) {
        if (full) {
            gtk_window_fullscreen(window);
        } else {
            gtk_window_unfullscreen(window);
        }
    }
    
    void setSize(int w, int h) {
        width = w;
        height = h;
        gtk_window_resize(window, w, h);
    }
    
    void setTitle(const std::string& t) {
        title = t;
        gtk_window_set_title(window, t.c_str());
    }
    
    void executeJS(const std::string& script) {
        if (webView) {
            webkit_web_view_run_javascript(webView, script.c_str(), nullptr, nullptr, nullptr);
        }
    }
    
    void onLoadStarted(std::function<void(BrowserWindow*)> callback) { onLoadStart = callback; }
    void onLoadFinished(std::function<void(BrowserWindow*)> callback) { onLoadFinish = callback; }
    void onTitleChanged(std::function<void(BrowserWindow*, const std::string&)> callback) { onTitleChange = callback; }
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    std::string getTitle() const { return title; }
    bool isRunning() const { return running; }
};

class SimpleBrowser {
private:
    static std::map<std::string, BrowserWindow*> windows;
    
public:
    static BrowserWindow* create(int width = 800, int height = 600, const std::string& title = "Browser") {
        auto* window = new BrowserWindow(width, height, title);
        windows[title] = window;
        return window;
    }
    
    static void closeAll() {
        for (auto& pair : windows) {
            pair.second->close();
            delete pair.second;
        }
        windows.clear();
    }
    
    static void showHTML(BrowserWindow* window, const std::string& content) {
        if (window) window->loadHTML(content);
    }
    
    static void showFile(BrowserWindow* window, const std::string& filename) {
        if (window) {
            std::filesystem::path current = std::filesystem::current_path();
            std::filesystem::path filepath = current / filename;
            if (std::filesystem::exists(filepath)) {
                window->loadFile(filepath.string());
            } else {
                std::cerr << "File not found: " << filepath << "\n";
            }
        }
    }
    
    static void showFolder(BrowserWindow* window, const std::string& foldername) {
        if (window) {
            std::filesystem::path current = std::filesystem::current_path();
            std::filesystem::path folderpath = current / foldername;
            
            if (!std::filesystem::exists(folderpath)) {
                std::cerr << "Folder not found: " << folderpath << "\n";
                return;
            }
            
            std::filesystem::path indexPath = folderpath / "index.html";
            if (std::filesystem::exists(indexPath)) {
                window->loadFile(indexPath.string());
                return;
            }
            
            for (const auto& entry : std::filesystem::directory_iterator(folderpath)) {
                if (entry.path().extension() == ".html" || entry.path().extension() == ".htm") {
                    window->loadFile(entry.path().string());
                    return;
                }
            }
            
            std::cerr << "No HTML files found in: " << folderpath << "\n";
        }
    }
    
    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

std::map<std::string, BrowserWindow*> SimpleBrowser::windows;

} // namespace SimpleWeb

#endif // SIMPLE_WEBKIT_HPP

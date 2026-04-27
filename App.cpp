#include "App.h"

App::App() : server_(io_.get_executor()){}

void App::run()  {
    server_.start();
    io_.run();
}

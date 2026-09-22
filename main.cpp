#include "App.h"

namespace CLI { int Run(int argc, char* argv[]); }

int main(int argc, char *argv[]) {
    if (argc == 1) {
        App app { 800, 600 };
        while (app.Run());
    } else {
        CLI::Run(argc, argv);
    }
}
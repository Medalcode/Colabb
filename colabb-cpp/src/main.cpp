#include "ui/main_window.hpp"
#include "colabb/version.hpp"
#include <gtk/gtk.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);
    
    std::cout << "Colabb Terminal v" << COLABB_VERSION << " (C++ Edition)" << std::endl;
    
    // Create and show main window
    colabb::ui::MainWindow window;
    window.show();
    
    // Run GTK main loop
    gtk_main();
    
    return 0;
}

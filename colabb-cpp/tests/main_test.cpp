#include <gtest/gtest.h>
#include <gtk/gtk.h>

int main(int argc, char **argv) {
    // Initialize GTK for tests that might need it (even if we try to avoid UI)
    gtk_init(&argc, &argv);
    
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

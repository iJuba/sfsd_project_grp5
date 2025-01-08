#include <gtk/gtk.h>
#include "student.h"

int main(int argc, char *argv[]) {
  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *btn_ajouter;
  GtkWidget *btn_rechercher;
  GtkWidget *btn_modifier;
  GtkWidget *btn_supprimer;
  GtkWidget *btn_extraire;
  GtkWidget *btn_reorganiser;
  GtkWidget *btn_quitter;
  GtkWidget *btn_reset;

  FichierTOF tdf;
  gtk_init(&argc, &argv);

  // Create main window
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Gestion des Étudiants");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // Create a grid layout
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);

  // Create buttons
  btn_ajouter = gtk_button_new_with_label("Ajouter Étudiant");
  btn_reset = gtk_button_new_with_label("reset bin");
  btn_rechercher = gtk_button_new_with_label("Rechercher Étudiant");
  btn_modifier = gtk_button_new_with_label("Modifier Étudiant");
  btn_supprimer = gtk_button_new_with_label("Supprimer Étudiant");
  btn_extraire = gtk_button_new_with_label("Extraire Étudiants");
  btn_reorganiser = gtk_button_new_with_label("Réorganiser");
  btn_quitter = gtk_button_new_with_label("Quitter");

  // Attach buttons to the grid
  gtk_grid_attach(GTK_GRID(grid), btn_ajouter, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_rechercher, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_modifier, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_supprimer, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_extraire, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_reorganiser, 0, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_quitter, 1, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_reset, 2, 5, 1, 1);

 g_signal_connect(btn_ajouter, "clicked", G_CALLBACK(ajouter_etudiant), NULL); 
 g_signal_connect(btn_rechercher, "clicked", G_CALLBACK(find), NULL);
 g_signal_connect(btn_modifier, "clicked", G_CALLBACK(modifier_etudiant), NULL);
 g_signal_connect(btn_supprimer, "clicked", G_CALLBACK(suppression_logique), NULL);
 g_signal_connect(btn_extraire, "clicked", G_CALLBACK(extractByClass), NULL);
 g_signal_connect(btn_reorganiser, "clicked", G_CALLBACK(Reorganisation), NULL);
  g_signal_connect(btn_quitter, "clicked", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(btn_reset, "clicked", G_CALLBACK(reset), NULL);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}



/*
#include <gtk/gtk.h>
#include "student.h"

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *btn_ajouter;
    GtkWidget *btn_rechercher;
    GtkWidget *btn_supprimer;
    GtkWidget *btn_reorganiser;
    GtkWidget *btn_quitter;

    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gestion des Étudiants");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Create buttons
    btn_ajouter = gtk_button_new_with_label("Ajouter Étudiant");
    btn_rechercher = gtk_button_new_with_label("Rechercher Étudiant");
    btn_supprimer = gtk_button_new_with_label("Supprimer Étudiant");
    btn_reorganiser = gtk_button_new_with_label("Réorganiser");
    btn_quitter = gtk_button_new_with_label("Quitter");

    // Attach buttons to the grid
    gtk_grid_attach(GTK_GRID(grid), btn_ajouter, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_rechercher, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_supprimer, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_reorganiser, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), btn_quitter, 1, 3, 1, 1);

    // Connect button signals
    g_signal_connect(btn_ajouter, "clicked", G_CALLBACK(ajouter_etudiant), NULL);
    g_signal_connect(btn_rechercher, "clicked", G_CALLBACK(find), NULL);
    //g_signal_connect(btn_supprimer, "clicked", G_CALLBACK(supprimer_etudiant_logique), NULL);
    g_signal_connect(btn_reorganiser, "clicked", G_CALLBACK(Reorganisation), NULL);
    g_signal_connect(btn_quitter, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    // Show all widgets
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
*/

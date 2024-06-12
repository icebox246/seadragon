#include <gtk/gtk.h>

static gboolean key_pressed(GtkEventControllerKey *self, guint keyval,
                            guint keycode, GdkModifierType state,
                            gpointer user_data) {
    GApplication *app = G_APPLICATION(user_data);
    if (keyval == 'q') {
        g_application_quit(app);
        return true;
    }
    return false;
}

static void file_clicked(GtkButton *self, gpointer user_data) {
    GApplication *app = G_APPLICATION(user_data);
    g_application_quit(app);
}

static void open_file(GApplication *self, GFile **files, gint n_files,
                      gchar *hint, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(GTK_APPLICATION(self));
    gtk_window_set_title(GTK_WINDOW(window), "dragon");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 0);
    gtk_window_set_resizable(GTK_WINDOW(window), false);

    {
        GtkEventController *key_event_controller =
            gtk_event_controller_key_new();
        g_signal_connect(key_event_controller, "key-pressed",
                         G_CALLBACK(key_pressed), self);
        gtk_widget_add_controller(GTK_WIDGET(window),
                                  GTK_EVENT_CONTROLLER(key_event_controller));
    }

    GtkWidget *file_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    for (gint i = 0; i < n_files; i++) {
        gchar *file_name = g_file_get_basename(G_FILE(files[i]));
        if (!g_file_query_exists(G_FILE(files[i]), NULL)) {
            g_print("File %s does not exist!\n", file_name);
            exit(1);
        }
        gchar *file_uri = g_file_get_uri(G_FILE(files[i]));
        gboolean file_type_uncertain;
        gchar *file_type =
            g_content_type_guess(file_uri, NULL, 0, &file_type_uncertain);
        GIcon *file_icon = g_content_type_get_icon(file_type);

        GBytes *file_uri_bytes = g_bytes_new(file_uri, strlen(file_uri));
        GdkContentProvider *provider =
            gdk_content_provider_new_for_bytes("text/uri-list", file_uri_bytes);

        GtkDragSource *drag_source = gtk_drag_source_new();
        gtk_drag_source_set_actions(drag_source, GDK_ACTION_COPY);
        gtk_drag_source_set_content(drag_source, provider);

        GtkWidget *draggable = gtk_button_new();
        {
            GtkWidget *inner_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
            {
                GtkWidget *image = gtk_image_new_from_gicon(file_icon);
                gtk_box_append(GTK_BOX(inner_box), GTK_WIDGET(image));
            }
            {
                GtkWidget *label = gtk_label_new(file_name);
                gtk_label_set_ellipsize(GTK_LABEL(label),
                                        PANGO_ELLIPSIZE_MIDDLE);
                gtk_box_append(GTK_BOX(inner_box), GTK_WIDGET(label));
            }
            gtk_button_set_child(GTK_BUTTON(draggable), GTK_WIDGET(inner_box));
            gtk_button_set_has_frame(GTK_BUTTON(draggable), false);
        }
        gtk_widget_add_controller(GTK_WIDGET(draggable),
                                  GTK_EVENT_CONTROLLER(drag_source));
        g_signal_connect(GTK_BUTTON(draggable), "clicked",
                         G_CALLBACK(file_clicked), self);
        gtk_box_append(GTK_BOX(file_box), GTK_WIDGET(draggable));
    }
    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(file_box));
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    if (argc < 2) {
        g_print("Usage: dragon FILENAME\n");
        exit(1);
    }

    app = gtk_application_new("icebox.seadragon", G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "open", G_CALLBACK(open_file), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

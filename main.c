#include "scheduler.h"
#include <math.h>
#include <cairo.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

/* ==================== LOAD CUSTOM CSS ==================== */
static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();

    // Find the directory of the running executable
    char exe_path[512];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        char *dir = dirname(exe_path);

        char css_path[600];
        snprintf(css_path, sizeof(css_path), "%s/style.css", dir);

        gtk_css_provider_load_from_path(provider, css_path, NULL);
    } else {
        gtk_css_provider_load_from_path(provider, "style.css", NULL);
    }

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

/* ==================== GLOBALS FOR ANIMATION ==================== */
static int anim_time = 0;
static gboolean anim_running = FALSE;

/* ==================== DRAW GANTT (ANIMATED) ==================== */
static gboolean on_draw_gantt(GtkWidget *widget, cairo_t *cr, gpointer data) {
    App *app = (App*)data;
    GtkAllocation a;
    gtk_widget_get_allocation(widget, &a);
    int W = a.width, H = a.height;

    // Background
    cairo_set_source_rgb(cr, 0.07, 0.05, 0.1);
    cairo_paint(cr);

    if (app->chart.count == 0) return FALSE;

    int tmax = 0;
    for (int i = 0; i < app->chart.count; i++) {
        if (app->chart.slices[i].end > tmax)
            tmax = app->chart.slices[i].end;
    }

    const int L = 40, R = 10, T = 20, B = 45;
    double usableW = W - L - R;
    double usableH = H - T - B;
    double barH = usableH * 0.6;
    double y = T + (usableH - barH) / 2.0;

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);

    for (int i = 0; i < app->chart.count; i++) {
        double start = app->chart.slices[i].start;
        double end = app->chart.slices[i].end;

        if (anim_running && anim_time < start) continue;
        double visibleEnd = end;
        if (anim_running && anim_time < end) visibleEnd = anim_time;

        double x1 = L + (start * usableW) / tmax;
        double x2 = L + (visibleEnd * usableW) / tmax;
        double w  = x2 - x1;
        if (w <= 0) continue;

        int pid = app->chart.slices[i].pid;
        // vivid color per pid (HSV-ish)
        double hue = fmod(pid * 47.0, 360.0);
        double s = 0.9, v = 0.95;
        double c = v * s;
        double hh = hue / 60.0;
        double x = c * (1 - fabs(fmod(hh, 2) - 1));
        double r,g,b;
        if      (hh < 1) {r=c; g=x; b=0;}
        else if (hh < 2) {r=x; g=c; b=0;}
        else if (hh < 3) {r=0; g=c; b=x;}
        else if (hh < 4) {r=0; g=x; b=c;}
        else if (hh < 5) {r=x; g=0; b=c;}
        else             {r=c; g=0; b=x;}
        double m = v - c; r+=m; g+=m; b+=m;

        // bar
        cairo_set_source_rgb(cr, r,g,b);
        cairo_rectangle(cr, x1, y, w, barH);
        cairo_fill(cr);

        // border
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_rectangle(cr, x1, y, w, barH);
        cairo_stroke(cr);

        // label
        char label[16]; g_snprintf(label, sizeof(label), "P%d", pid);
        cairo_move_to(cr, x1 + 5, y + barH/2);
        cairo_show_text(cr, label);
    }

    // axis
    cairo_set_source_rgb(cr, 1,1,1);
    cairo_move_to(cr, L, y+barH+5);
    cairo_line_to(cr, L+usableW, y+barH+5);
    cairo_stroke(cr);

    // ticks
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10);
    int ticks = tmax < 20 ? tmax : 20;
    if (ticks <= 0) ticks = 1;

    for (int i=0; i<=ticks; i++) {
        double tt = (i / (double)ticks) * tmax;
        double x = L + (tt * usableW) / tmax;
        double alpha = 1.0;

        if (anim_running) {
            double fade_start = tt - 2;
            if (anim_time < fade_start) alpha = 0.0;
            else if (anim_time < tt) alpha = (anim_time - fade_start)/2.0;
            if (alpha > 1.0) alpha = 1.0;
        }

        cairo_set_source_rgba(cr, 1,1,1, alpha);
        cairo_move_to(cr, x, y+barH+5);
        cairo_line_to(cr, x, y+barH+10);
        cairo_stroke(cr);

        char tbuf[16]; g_snprintf(tbuf, sizeof(tbuf), "%d", (int)round(tt));
        cairo_move_to(cr, x-5, y+barH+25);
        cairo_show_text(cr, tbuf);
    }

    cairo_move_to(cr, 5, y+barH/2);
    cairo_show_text(cr, "Gantt Chart");
    return FALSE;
}

/* ==================== ANIMATION TIMER ==================== */
static gboolean update_animation(gpointer data) {
    App *app = (App*)data;
    anim_time += 1;
    gtk_widget_queue_draw(app->drawing);

    int tmax = 0;
    for (int i = 0; i < app->chart.count; i++) {
        if (app->chart.slices[i].end > tmax)
            tmax = app->chart.slices[i].end;
    }

    if (anim_time > tmax) {
        anim_running = FALSE;
        gtk_style_context_remove_class(gtk_widget_get_style_context(app->btn_run), "running");
        return FALSE;
    }
    return TRUE;
}

/* ==================== RESULTS TABLE ==================== */
static void results_fill(App *app) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app->tree_results)));
    gtk_list_store_clear(store);

    double sumW=0, sumT=0;
    for (int i=0; i<app->nprocs; i++) {
        GtkTreeIter it;
        gtk_list_store_append(store, &it);
        gtk_list_store_set(store, &it,
                           0, app->procs[i].pid,
                           1, app->procs[i].arrival,
                           2, app->procs[i].burst,
                           3, app->procs[i].priority,
                           4, app->procs[i].waiting,
                           5, app->procs[i].turnaround,
                           -1);
        sumW += app->procs[i].waiting;
        sumT += app->procs[i].turnaround;
    }

    char buf[128];
    g_snprintf(buf, sizeof(buf),
               "Average Waiting: %.2f    Average Turnaround: %.2f",
               sumW/app->nprocs, sumT/app->nprocs);
    gtk_label_set_text(GTK_LABEL(app->lbl_avg), buf);
}

/* ==================== ABOUT DIALOG (PRO) ==================== */
static void on_about_clicked(GtkButton *btn, gpointer user_data) {
    GtkWidget *about = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), "CPU Scheduling Simulator");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), "1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about),
                                  "A colorful CPU scheduling visualizer built in C using GTK3 and Cairo.\n\n"
                                  "Developed by: Parth Borikar\n"
                                  "Course: Operating Systems Fundamentals (OSF)\n"
                                  "Submitted to: Dr. Nilima Zade (Dept. of Computer Science)\n"
                                  "Department of Electronics & Telecommunication, 2025");
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(about), GTK_LICENSE_MIT_X11);
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about), "https://github.com/ParthBorax");
    // If your icon is installed (e.g., in theme), this shows it:
    gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about), "cpu-scheduler");
    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_destroy(about);
}

/* ==================== RUN BUTTON ==================== */
static void on_run_clicked(GtkButton *btn, gpointer user_data) {
    App *app = (App*)user_data;
    int alg = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_alg));
    int quantum = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(app->spin_quantum));

    app->nprocs = 0;
    for (int i=0; i<MAXP; i++) {
        if (gtk_toggle_button_get_active(app->tog_en[i])) {
            app->procs[app->nprocs].pid      = i+1;
            app->procs[app->nprocs].arrival  = gtk_adjustment_get_value(app->adj_arr[i]);
            app->procs[app->nprocs].burst    = gtk_adjustment_get_value(app->adj_burst[i]);
            app->procs[app->nprocs].priority = gtk_adjustment_get_value(app->adj_prio[i]);
            app->nprocs++;
        }
    }

    run_scheduler(app, (Algorithm)alg, quantum);
    results_fill(app);

    anim_time = 0;
    anim_running = TRUE;
    gtk_style_context_add_class(gtk_widget_get_style_context(app->btn_run), "running");
    gtk_widget_queue_draw(app->drawing);
    g_timeout_add(60, update_animation, app);
}

/* ==================== SPIN CREATOR ==================== */
static GtkWidget* make_spin(App *app, GtkAdjustment **out_adj, int min, int max, int def) {
    GtkAdjustment *adj = gtk_adjustment_new(def, min, max, 1, 5, 0);
    GtkWidget *spin = gtk_spin_button_new(adj, 1, 0);
    *out_adj = adj;
    return spin;
}

/* ==================== BUILD GUI ==================== */
static void build_ui(App *app) {
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "🌈 CPU Scheduling Simulator");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 980, 640);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(app->window), root);

    GtkWidget *header = gtk_label_new("CPU Scheduling Simulator");
    gtk_widget_set_halign(header, GTK_ALIGN_CENTER);
    gtk_widget_set_name(header, "header_label");
    gtk_box_pack_start(GTK_BOX(root), header, FALSE, FALSE, 4);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(root), top, FALSE, FALSE, 0);

    app->combo_alg = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_alg), "FCFS");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_alg), "SJF (Non-preemptive)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_alg), "Priority (Non-preemptive)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_alg), "Round Robin");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_alg), "SJF (Preemptive)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_alg), "Priority (Preemptive)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_alg), 0);

    GtkWidget *lbl_alg = gtk_label_new("Algorithm:");
    gtk_box_pack_start(GTK_BOX(top), lbl_alg, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), app->combo_alg, FALSE, FALSE, 0);

    GtkWidget *lbl_q = gtk_label_new("Quantum:");
    app->spin_quantum = gtk_spin_button_new_with_range(1, 50, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->spin_quantum), 4);
    gtk_box_pack_start(GTK_BOX(top), lbl_q, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), app->spin_quantum, FALSE, FALSE, 0);

    app->btn_run = gtk_button_new_with_label("▶ Run Simulation");
    gtk_box_pack_end(GTK_BOX(top), app->btn_run, FALSE, FALSE, 0);
    g_signal_connect(app->btn_run, "clicked", G_CALLBACK(on_run_clicked), app);

    // About button
    GtkWidget *btn_about = gtk_button_new_with_label("ℹ️ About");
    gtk_box_pack_end(GTK_BOX(top), btn_about, FALSE, FALSE, 0);
    g_signal_connect(btn_about, "clicked", G_CALLBACK(on_about_clicked), NULL);

    // Inputs grid
    app->grid_inputs = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(app->grid_inputs), 4);
    gtk_grid_set_column_spacing(GTK_GRID(app->grid_inputs), 8);
    gtk_box_pack_start(GTK_BOX(root), app->grid_inputs, FALSE, FALSE, 0);

    const char *hdr[] = {"Use", "PID", "Arrival", "Burst", "Priority"};
    for (int c=0; c<5; c++) {
        GtkWidget *l = gtk_label_new(hdr[c]);
        gtk_grid_attach(GTK_GRID(app->grid_inputs), l, c, 0, 1, 1);
    }

    for (int i=0; i<MAXP; i++) {
        app->tog_en[i] = GTK_TOGGLE_BUTTON(gtk_check_button_new());
        if (i < 4) gtk_toggle_button_set_active(app->tog_en[i], TRUE);

        GtkWidget *lpid = gtk_label_new(NULL);
        char pidtxt[8]; g_snprintf(pidtxt, sizeof(pidtxt), "P%d", i+1);
        gtk_label_set_text(GTK_LABEL(lpid), pidtxt);

        GtkWidget *s_arr = make_spin(app, &app->adj_arr[i], 0, 100, i);
        GtkWidget *s_bur = make_spin(app, &app->adj_burst[i], 1, 50, 3);
        GtkWidget *s_pri = make_spin(app, &app->adj_prio[i], 1, 10, 2);

        gtk_grid_attach(GTK_GRID(app->grid_inputs), GTK_WIDGET(app->tog_en[i]), 0, i+1, 1, 1);
        gtk_grid_attach(GTK_GRID(app->grid_inputs), lpid, 1, i+1, 1, 1);
        gtk_grid_attach(GTK_GRID(app->grid_inputs), s_arr, 2, i+1, 1, 1);
        gtk_grid_attach(GTK_GRID(app->grid_inputs), s_bur, 3, i+1, 1, 1);
        gtk_grid_attach(GTK_GRID(app->grid_inputs), s_pri, 4, i+1, 1, 1);
    }

    // Results + Gantt
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(root), paned, TRUE, TRUE, 0);

    app->tree_results = gtk_tree_view_new();
    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT,G_TYPE_INT,G_TYPE_INT,G_TYPE_INT,G_TYPE_INT,G_TYPE_INT);
    gtk_tree_view_set_model(GTK_TREE_VIEW(app->tree_results), GTK_TREE_MODEL(store));
    g_object_unref(store);

    const char *cols[] = {"PID","Arrival","Burst","Priority","Waiting","Turnaround"};
    for (int i=0; i<6; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col =
        gtk_tree_view_column_new_with_attributes(cols[i], r, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(app->tree_results), col);
    }

    GtkWidget *scr1 = gtk_scrolled_window_new(NULL,NULL);
    gtk_container_add(GTK_CONTAINER(scr1), app->tree_results);
    gtk_widget_set_size_request(scr1, -1, 220);

    app->lbl_avg = gtk_label_new("Average Waiting: -    Average Turnaround: -");

    GtkWidget *box_res = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start(GTK_BOX(box_res), scr1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box_res), app->lbl_avg, FALSE, FALSE, 0);

    gtk_paned_pack1(GTK_PANED(paned), box_res, TRUE, FALSE);

    app->drawing = gtk_drawing_area_new();
    gtk_widget_set_size_request(app->drawing, -1, 260);
    g_signal_connect(app->drawing, "draw", G_CALLBACK(on_draw_gantt), app);

    GtkWidget *frame = gtk_frame_new("Gantt Chart");
    gtk_container_add(GTK_CONTAINER(frame), app->drawing);
    gtk_paned_pack2(GTK_PANED(paned), frame, TRUE, FALSE);

    gtk_widget_show_all(app->window);
}

/* ==================== MAIN ==================== */
int main(int argc, char **argv) {
    gtk_init(&argc, &argv);
    load_css();
    App app = {0};
    build_ui(&app);
    gtk_main();
    return 0;
}

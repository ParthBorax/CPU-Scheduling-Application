#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <gtk/gtk.h>

#define MAXP 8

/* ==================== PROCESS STRUCT ==================== */
typedef struct {
    int pid;
    int arrival;
    int burst;
    int priority;
    // outputs
    int start_time;
    int finish_time;
    int waiting;
    int turnaround;
} Process;

/* ==================== GANTT CHART STRUCTS ==================== */
typedef struct {
    int pid;
    int start;
    int end;
} GanttSlice;

typedef struct {
    GanttSlice slices[256];
    int count;
} GanttChart;

/* ==================== ALGORITHMS ENUM ==================== */
typedef enum {
    ALG_FCFS = 0,              // First Come First Served
    ALG_SJF,                   // Shortest Job First (Non-preemptive)
    ALG_PRIORITY,              // Priority Scheduling (Non-preemptive)
    ALG_RR,                    // Round Robin
    ALG_SJF_PREEMPTIVE,        // Shortest Remaining Time First (Preemptive)
    ALG_PRIORITY_PREEMPTIVE    // Priority (Preemptive)
} Algorithm;

/* ==================== MAIN APP STRUCT ==================== */
typedef struct {
    GtkWidget *window;
    GtkWidget *grid_inputs;     // Process input grid
    GtkWidget *combo_alg;       // Algorithm selector
    GtkWidget *spin_quantum;    // Quantum spinner (for RR)
    GtkWidget *btn_run;         // Run button
    GtkWidget *tree_results;    // Results table
    GtkWidget *lbl_avg;         // Average display
    GtkWidget *drawing;         // Gantt chart area

    GtkAdjustment *adj_arr[MAXP];
    GtkAdjustment *adj_burst[MAXP];
    GtkAdjustment *adj_prio[MAXP];
    GtkToggleButton *tog_en[MAXP];

    Process procs[MAXP];
    int nprocs;
    GanttChart chart;
} App;

/* ==================== FUNCTION DECLARATIONS ==================== */

// Central scheduler controller
void run_scheduler(App *app, Algorithm alg, int quantum);

/* Core algorithms */
void schedule_fcfs(Process p[], int n, GanttChart *chart);
void schedule_sjf_np(Process p[], int n, GanttChart *chart);
void schedule_priority_np(Process p[], int n, GanttChart *chart);
void schedule_rr(Process p[], int n, int quantum, GanttChart *chart);

/* NEW — preemptive versions */
void schedule_sjf_preemptive(Process p[], int n, GanttChart *chart);
void schedule_priority_preemptive(Process p[], int n, GanttChart *chart);

#endif


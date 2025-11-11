#include "scheduler.h"
#include <limits.h>
#include <string.h>

/* ==================== UTILITY: SORT BY ARRIVAL ==================== */
static void sort_by_arrival(Process p[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (p[j].arrival > p[j + 1].arrival) {
                Process temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }
}

/* ==================== FCFS ==================== */
void schedule_fcfs(Process p[], int n, GanttChart *chart) {
    sort_by_arrival(p, n);
    chart->count = 0;
    int time = 0;

    for (int i = 0; i < n; i++) {
        if (time < p[i].arrival)
            time = p[i].arrival;

        p[i].start_time = time;
        p[i].finish_time = time + p[i].burst;
        p[i].turnaround = p[i].finish_time - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;

        chart->slices[chart->count].pid = p[i].pid;
        chart->slices[chart->count].start = p[i].start_time;
        chart->slices[chart->count].end = p[i].finish_time;
        chart->count++;

        time += p[i].burst;
    }
}

/* ==================== SJF (NON-PREEMPTIVE) ==================== */
void schedule_sjf_np(Process p[], int n, GanttChart *chart) {
    sort_by_arrival(p, n);
    chart->count = 0;

    int completed = 0, time = 0, is_completed[MAXP] = {0};

    while (completed < n) {
        int idx = -1;
        int min_burst = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && !is_completed[i]) {
                if (p[i].burst < min_burst) {
                    min_burst = p[i].burst;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        p[idx].start_time = time;
        p[idx].finish_time = time + p[idx].burst;
        p[idx].turnaround = p[idx].finish_time - p[idx].arrival;
        p[idx].waiting = p[idx].turnaround - p[idx].burst;
        is_completed[idx] = 1;
        completed++;

        chart->slices[chart->count].pid = p[idx].pid;
        chart->slices[chart->count].start = p[idx].start_time;
        chart->slices[chart->count].end = p[idx].finish_time;
        chart->count++;

        time = p[idx].finish_time;
    }
}

/* ==================== PRIORITY (NON-PREEMPTIVE) ==================== */
void schedule_priority_np(Process p[], int n, GanttChart *chart) {
    sort_by_arrival(p, n);
    chart->count = 0;

    int completed = 0, time = 0, done[MAXP] = {0};

    while (completed < n) {
        int idx = -1, best_pri = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && !done[i]) {
                if (p[i].priority < best_pri) {
                    best_pri = p[i].priority;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        p[idx].start_time = time;
        p[idx].finish_time = time + p[idx].burst;
        p[idx].turnaround = p[idx].finish_time - p[idx].arrival;
        p[idx].waiting = p[idx].turnaround - p[idx].burst;
        done[idx] = 1;
        completed++;

        chart->slices[chart->count].pid = p[idx].pid;
        chart->slices[chart->count].start = p[idx].start_time;
        chart->slices[chart->count].end = p[idx].finish_time;
        chart->count++;

        time = p[idx].finish_time;
    }
}

/* ==================== ROUND ROBIN ==================== */
void schedule_rr(Process p[], int n, int quantum, GanttChart *chart) {
    sort_by_arrival(p, n);
    chart->count = 0;

    int remaining[MAXP];
    for (int i = 0; i < n; i++)
        remaining[i] = p[i].burst;

    int time = 0, completed = 0;
    int queue[256], front = 0, rear = 0, in_queue[MAXP] = {0};

    int next_arrival = 0;
    queue[rear++] = 0;
    in_queue[0] = 1;

    while (completed < n) {
        if (front == rear) {
            if (next_arrival < n && time < p[next_arrival].arrival)
                time = p[next_arrival].arrival;
            queue[rear++] = next_arrival++;
            continue;
        }

        int i = queue[front++];
        in_queue[i] = 0;

        int start = time;
        int exec_time = (remaining[i] > quantum) ? quantum : remaining[i];
        time += exec_time;
        remaining[i] -= exec_time;

        chart->slices[chart->count].pid = p[i].pid;
        chart->slices[chart->count].start = start;
        chart->slices[chart->count].end = time;
        chart->count++;

        while (next_arrival < n && p[next_arrival].arrival <= time) {
            if (!in_queue[next_arrival]) {
                queue[rear++] = next_arrival;
                in_queue[next_arrival] = 1;
            }
            next_arrival++;
        }

        if (remaining[i] > 0) {
            queue[rear++] = i;
            in_queue[i] = 1;
        } else {
            completed++;
            p[i].finish_time = time;
            p[i].turnaround = p[i].finish_time - p[i].arrival;
            p[i].waiting = p[i].turnaround - p[i].burst;
        }
    }
}

/* ==================== SJF (PREEMPTIVE) / SRTF ==================== */
void schedule_sjf_preemptive(Process p[], int n, GanttChart *chart) {
    sort_by_arrival(p, n);
    chart->count = 0;

    int remaining[MAXP];
    for (int i = 0; i < n; i++) remaining[i] = p[i].burst;

    int time = 0, completed = 0;
    int current = -1;
    while (completed < n) {
        int idx = -1, min_rem = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && remaining[i] > 0) {
                if (remaining[i] < min_rem) {
                    min_rem = remaining[i];
                    idx = i;
                }
            }
        }

        if (idx == -1) { time++; continue; }

        if (current != idx) {
            if (chart->count > 0)
                chart->slices[chart->count - 1].end = time;
            chart->slices[chart->count].pid = p[idx].pid;
            chart->slices[chart->count].start = time;
            chart->count++;
            current = idx;
        }

        remaining[idx]--;
        time++;

        if (remaining[idx] == 0) {
            completed++;
            p[idx].finish_time = time;
            p[idx].turnaround = p[idx].finish_time - p[idx].arrival;
            p[idx].waiting = p[idx].turnaround - p[idx].burst;
        }
    }
    chart->slices[chart->count - 1].end = time;
}

/* ==================== PRIORITY (PREEMPTIVE) ==================== */
void schedule_priority_preemptive(Process p[], int n, GanttChart *chart) {
    sort_by_arrival(p, n);
    chart->count = 0;

    int remaining[MAXP];
    for (int i = 0; i < n; i++) remaining[i] = p[i].burst;

    int time = 0, completed = 0;
    int current = -1;

    while (completed < n) {
        int idx = -1, best_pri = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && remaining[i] > 0) {
                if (p[i].priority < best_pri) {
                    best_pri = p[i].priority;
                    idx = i;
                }
            }
        }

        if (idx == -1) { time++; continue; }

        if (current != idx) {
            if (chart->count > 0)
                chart->slices[chart->count - 1].end = time;
            chart->slices[chart->count].pid = p[idx].pid;
            chart->slices[chart->count].start = time;
            chart->count++;
            current = idx;
        }

        remaining[idx]--;
        time++;

        if (remaining[idx] == 0) {
            completed++;
            p[idx].finish_time = time;
            p[idx].turnaround = p[idx].finish_time - p[idx].arrival;
            p[idx].waiting = p[idx].turnaround - p[idx].burst;
        }
    }
    chart->slices[chart->count - 1].end = time;
}

/* ==================== SCHEDULER CONTROLLER ==================== */
void run_scheduler(App *app, Algorithm alg, int quantum) {
    Process *p = app->procs;
    int n = app->nprocs;
    GanttChart *chart = &app->chart;

    switch (alg) {
        case ALG_FCFS:
            schedule_fcfs(p, n, chart);
            break;
        case ALG_SJF:
            schedule_sjf_np(p, n, chart);
            break;
        case ALG_PRIORITY:
            schedule_priority_np(p, n, chart);
            break;
        case ALG_RR:
            schedule_rr(p, n, quantum, chart);
            break;
        case ALG_SJF_PREEMPTIVE:
            schedule_sjf_preemptive(p, n, chart);
            break;
        case ALG_PRIORITY_PREEMPTIVE:
            schedule_priority_preemptive(p, n, chart);
            break;
        default:
            g_print("Unknown algorithm selected!\n");
            break;
    }
}


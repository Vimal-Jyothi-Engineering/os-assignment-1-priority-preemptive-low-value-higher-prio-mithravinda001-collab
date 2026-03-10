#include <stdio.h>
#include <string.h>

#define MAX_PROCESSES 10
#define MAX_GANTT     10000

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
} Process;

/* ── Gantt chart (merges consecutive identical slots) ───────────────────── */
void print_gantt(int gantt_pid[], int gantt_time[], int gantt_len) {
    /* merge consecutive identical pid blocks */
    int mpid[MAX_GANTT];
    int mtime[MAX_GANTT + 1];
    int mlen = 0;

    if (gantt_len == 0) return;

    mpid[0]  = gantt_pid[0];
    mtime[0] = gantt_time[0];

    for (int i = 1; i < gantt_len; i++) {
        if (gantt_pid[i] != mpid[mlen]) {
            mlen++;
            mpid[mlen]  = gantt_pid[i];
            mtime[mlen] = gantt_time[i];
        }
    }
    mtime[mlen + 1] = gantt_time[gantt_len];
    mlen++;   /* mlen is now the count of merged blocks */

    printf("\nGantt Chart:\n");

    /* top border */
    printf(" ");
    for (int i = 0; i < mlen; i++) printf("--------");
    printf("\n|");

    /* labels */
    for (int i = 0; i < mlen; i++) {
        if (mpid[i] == -1) printf("  IDLE  |");
        else               printf("   P%-2d  |", mpid[i]);
    }

    /* bottom border */
    printf("\n ");
    for (int i = 0; i < mlen; i++) printf("--------");

    /* time markers */
    printf("\n%-3d", mtime[0]);
    for (int i = 0; i < mlen; i++) printf("       %-3d", mtime[i + 1]);
    printf("\n");
}

/* ── Scheduler ───────────────────────────────────────────────────────────── */
void preemptive_priority(Process proc[], int n) {
    int current_time = 0;
    int completed    = 0;

    static int gantt_pid [MAX_GANTT];
    static int gantt_time[MAX_GANTT + 1];
    int gantt_len = 0;

    for (int i = 0; i < n; i++)
        proc[i].remaining_time = proc[i].burst_time;

    /* start clock at the earliest arrival */
    current_time = proc[0].arrival_time;
    for (int i = 1; i < n; i++)
        if (proc[i].arrival_time < current_time)
            current_time = proc[i].arrival_time;

    while (completed < n) {

        /* pick highest-priority (lowest value) ready process */
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time &&
                proc[i].remaining_time > 0) {
                if (sel == -1 ||
                    proc[i].priority < proc[sel].priority ||
                    (proc[i].priority == proc[sel].priority &&
                     proc[i].arrival_time < proc[sel].arrival_time)) {
                    sel = i;
                }
            }
        }

        if (sel == -1) {
            /* CPU idle — jump to next arrival */
            gantt_pid[gantt_len]  = -1;
            gantt_time[gantt_len] = current_time;
            gantt_len++;

            int next = -1;
            for (int i = 0; i < n; i++)
                if (proc[i].remaining_time > 0 &&
                    (next == -1 || proc[i].arrival_time < next))
                    next = proc[i].arrival_time;

            current_time          = next;
            gantt_time[gantt_len] = current_time;
            continue;
        }

        /* next event: either a higher-priority arrival or selected finishes */
        int next_event = current_time + proc[sel].remaining_time;
        for (int i = 0; i < n; i++) {
            if (i == sel || proc[i].remaining_time <= 0) continue;
            if (proc[i].arrival_time > current_time &&
                proc[i].arrival_time < next_event)
                next_event = proc[i].arrival_time;
        }

        /* record Gantt slot */
        gantt_pid[gantt_len]  = proc[sel].pid;
        gantt_time[gantt_len] = current_time;
        gantt_len++;

        int run = next_event - current_time;
        proc[sel].remaining_time -= run;
        current_time              = next_event;
        gantt_time[gantt_len]     = current_time;

        if (proc[sel].remaining_time == 0) {
            proc[sel].completion_time  = current_time;
            proc[sel].turnaround_time  = current_time - proc[sel].arrival_time;
            proc[sel].waiting_time     = proc[sel].turnaround_time
                                         - proc[sel].burst_time;
            completed++;
        }
    }

    /* ── print ───────────────────────────────────────────────────────────── */
    print_gantt(gantt_pid, gantt_time, gantt_len);

    printf("\n%-6s %-10s %-10s %-10s %-12s %-12s %-12s\n",
           "PID", "Arrival", "Burst", "Priority",
           "Completion", "Turnaround", "Waiting");
    printf("%-6s %-10s %-10s %-10s %-12s %-12s %-12s\n",
           "---", "-------", "-----", "--------",
           "----------", "----------", "-------");

    float total_tat = 0, total_wt = 0;
    for (int i = 0; i < n; i++) {
        printf("P%-5d %-10d %-10d %-10d %-12d %-12d %-12d\n",
               proc[i].pid,
               proc[i].arrival_time,
               proc[i].burst_time,
               proc[i].priority,
               proc[i].completion_time,
               proc[i].turnaround_time,
               proc[i].waiting_time);
        total_tat += proc[i].turnaround_time;
        total_wt  += proc[i].waiting_time;
    }

    printf("\nAverage Turnaround Time : %.2f\n", total_tat / n);
    printf("Average Waiting Time    : %.2f\n",   total_wt  / n);
}

/* ── main ────────────────────────────────────────────────────────────────── */
int main() {
    int n;
    Process proc[MAX_PROCESSES];

    printf("=== Preemptive Priority CPU Scheduling ===\n");
    printf("    (lower priority number = higher priority)\n\n");

    printf("Enter number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", &n);

    printf("\nEnter process details (format: P<id> <arrival> <burst> <priority>):\n");
    for (int i = 0; i < n; i++) {
        int pid_num;
        /* accept both "P1 0 7 3" and plain "0 7 3" */
        char peek;
        scanf(" %c", &peek);
        if (peek == 'P' || peek == 'p') {
            scanf("%d %d %d %d",
                  &pid_num,
                  &proc[i].arrival_time,
                  &proc[i].burst_time,
                  &proc[i].priority);
            proc[i].pid = pid_num;
        } else {
            /* peek was a digit — push it back via ungetc not available cleanly,
               so just read the rest of the three numbers */
            int first = peek - '0';
            int rest;
            /* read remaining digits of first number */
            int arrival = first;
            char c;
            while (scanf("%c", &c) == 1 && c >= '0' && c <= '9')
                arrival = arrival * 10 + (c - '0');
            /* now c is the delimiter after arrival */
            scanf("%d %d", &proc[i].burst_time, &proc[i].priority);
            proc[i].arrival_time = arrival;
            proc[i].pid = i + 1;
        }
    }

    /* sort by arrival time (insertion sort) */
    for (int i = 1; i < n; i++) {
        Process key = proc[i];
        int j = i - 1;
        while (j >= 0 && proc[j].arrival_time > key.arrival_time) {
            proc[j + 1] = proc[j];
            j--;
        }
        proc[j + 1] = key;
    }

    preemptive_priority(proc, n);
    return 0;
}

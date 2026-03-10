#include <stdio.h>
#include <string.h>

#define MAX_PROCESSES 10
#define MAX_GANTT     1000

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;          // lower value = higher priority
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int first_response;    // -1 = not yet started
    int started;
} Process;

/* ── Gantt chart ─────────────────────────────────────────────────────────── */
void print_gantt(int gantt_pid[], int gantt_time[], int gantt_len) {
    // Merge consecutive identical slots for a compact chart
    int mpid[MAX_GANTT], mtime[MAX_GANTT + 1];
    int mlen = 0;

    mpid[0]  = gantt_pid[0];
    mtime[0] = gantt_time[0];
    for (int i = 1; i < gantt_len; i++) {
        if (gantt_pid[i] == mpid[mlen]) {
            // extend current block
        } else {
            mtime[++mlen] = gantt_time[i];
            mpid[mlen]    = gantt_pid[i];
        }
    }
    mtime[++mlen] = gantt_time[gantt_len];   // final timestamp
    // mlen is now the number of merged blocks

    printf("\nGantt Chart:\n");

    // Top border
    printf(" ");
    for (int i = 0; i < mlen; i++) printf("--------");
    printf("\n|");

    // Process labels
    for (int i = 0; i < mlen; i++) {
        if (mpid[i] == -1) printf("  IDLE  |");
        else               printf("   P%-2d  |", mpid[i]);
    }

    // Bottom border
    printf("\n ");
    for (int i = 0; i < mlen; i++) printf("--------");

    // Time markers
    printf("\n%-3d", mtime[0]);
    for (int i = 0; i < mlen; i++) printf("       %-3d", mtime[i + 1]);
    printf("\n");
}

/* ── Scheduler ───────────────────────────────────────────────────────────── */
void preemptive_priority(Process proc[], int n) {
    int current_time = 0;
    int completed    = 0;

    int gantt_pid[MAX_GANTT];
    int gantt_time[MAX_GANTT + 1];
    int gantt_len = 0;

    // Initialise
    for (int i = 0; i < n; i++) {
        proc[i].remaining_time = proc[i].burst_time;
        proc[i].first_response = -1;
        proc[i].started        = 0;
    }

    // Find earliest arrival so we can start the clock correctly
    int start = proc[0].arrival_time;
    for (int i = 1; i < n; i++)
        if (proc[i].arrival_time < start)
            start = proc[i].arrival_time;
    current_time = start;

    while (completed < n) {

        // Pick the highest-priority (lowest priority value) process
        // that has arrived and still has work left
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
            // CPU idle — jump to next arrival
            gantt_pid[gantt_len]    = -1;
            gantt_time[gantt_len]   = current_time;
            gantt_len++;

            int next = -1;
            for (int i = 0; i < n; i++)
                if (proc[i].remaining_time > 0 &&
                    (next == -1 || proc[i].arrival_time < next))
                    next = proc[i].arrival_time;

            current_time = next;
            gantt_time[gantt_len] = current_time;
            continue;
        }

        // Record first response
        if (proc[sel].first_response == -1)
            proc[sel].first_response = current_time;

        // Find the earliest future event that could cause a preemption:
        //   • a new process arriving
        //   • the selected process finishing
        int next_event = current_time + proc[sel].remaining_time; // finish time

        for (int i = 0; i < n; i++) {
            if (proc[i].remaining_time > 0 && i != sel &&
                proc[i].arrival_time > current_time &&
                proc[i].arrival_time < next_event) {
                next_event = proc[i].arrival_time;
            }
        }

        // Record Gantt slot
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
            proc[sel].waiting_time     = proc[sel].turnaround_time - proc[sel].burst_time;
            completed++;
        }
    }

    // ── Gantt chart ────────────────────────────────────────────────────────
    print_gantt(gantt_pid, gantt_time, gantt_len);

    // ── Results table ──────────────────────────────────────────────────────
    printf("\n%-6s %-10s %-10s %-10s %-12s %-12s %-12s\n",
           "PID", "Arrival", "Burst", "Priority",
           "Completion", "Turnaround", "Waiting");
    printf("%-6s %-10s %-10s %-10s %-12s %-12s %-12s\n",
           "---", "-------", "-----", "--------",
           "----------", "----------", "-------");

    float total_tat = 0, total_wt = 0;
    for (int i = 0; i < n; i++) {
        printf("%-6d %-10d %-10d %-10d %-12d %-12d %-12d\n",
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

/* ── main ─────────────────────────────────────────────────────────────────── */
int main() {
    int n;
    Process proc[MAX_PROCESSES];

    printf("=== Preemptive Priority CPU Scheduling ===\n");
    printf("    (lower priority number = higher priority)\n\n");

    printf("Enter number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", &n);

    printf("\nEnter process details:\n");
    for (int i = 0; i < n; i++) {
        proc[i].pid = i + 1;
        printf("\n  P%d\n", i + 1);
        printf("    Arrival Time : "); scanf("%d", &proc[i].arrival_time);
        printf("    Burst Time   : "); scanf("%d", &proc[i].burst_time);
        printf("    Priority     : "); scanf("%d", &proc[i].priority);
    }

    // Sort by arrival time (insertion sort — keeps original pid labels)
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
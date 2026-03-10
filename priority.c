#include <stdio.h>

#define MAX  10
#define GMAX 100000

typedef struct {
    int pid, at, bt, pri, rt, ct, tat, wt;
} Proc;

int main() {
    int n;
    scanf("%d", &n);

    Proc p[MAX];
    for (int i = 0; i < n; i++) {
        char label[16];
        scanf("%s %d %d %d", label, &p[i].at, &p[i].bt, &p[i].pri);
        p[i].pid = 0;
        for (int k = 0; label[k]; k++)
            if (label[k] >= '0' && label[k] <= '9')
                p[i].pid = p[i].pid * 10 + (label[k] - '0');
        p[i].rt = p[i].bt;
        p[i].ct = p[i].tat = p[i].wt = 0;
    }

    /* insertion-sort by arrival time */
    for (int i = 1; i < n; i++) {
        Proc key = p[i]; int j = i - 1;
        while (j >= 0 && p[j].at > key.at) { p[j+1] = p[j]; j--; }
        p[j+1] = key;
    }

    /* preemptive priority scheduling */
    static int gpid[GMAX], gt[GMAX+1];
    int glen = 0, done = 0, t = 0;

    t = p[0].at;
    for (int i = 1; i < n; i++) if (p[i].at < t) t = p[i].at;

    while (done < n) {
        int sel = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= t && p[i].rt > 0) {
                if (sel < 0 || p[i].pri < p[sel].pri ||
                    (p[i].pri == p[sel].pri && p[i].at < p[sel].at))
                    sel = i;
            }
        }

        if (sel < 0) {
            gpid[glen] = -1; gt[glen] = t; glen++;
            int nxt = -1;
            for (int i = 0; i < n; i++)
                if (p[i].rt > 0 && (nxt < 0 || p[i].at < nxt)) nxt = p[i].at;
            t = nxt; gt[glen] = t;
            continue;
        }

        int nxt = t + p[sel].rt;
        for (int i = 0; i < n; i++) {
            if (i == sel || p[i].rt <= 0) continue;
            if (p[i].at > t && p[i].at < nxt) nxt = p[i].at;
        }

        gpid[glen] = p[sel].pid; gt[glen] = t; glen++;
        p[sel].rt -= (nxt - t);
        t = nxt; gt[glen] = t;

        if (p[sel].rt == 0) {
            p[sel].ct  = t;
            p[sel].tat = t - p[sel].at;
            p[sel].wt  = p[sel].tat - p[sel].bt;
            done++;
        }
    }

    /* Gantt chart (merged) */
    int mpid[GMAX], mt[GMAX+1]; int mlen = 0;
    mpid[0] = gpid[0]; mt[0] = gt[0];
    for (int i = 1; i < glen; i++) {
        if (gpid[i] != mpid[mlen]) {
            mlen++; mpid[mlen] = gpid[i]; mt[mlen] = gt[i];
        }
    }
    mt[mlen+1] = gt[glen];
    mlen++;

    printf("Gantt Chart:\n");
    printf(" "); for (int i=0;i<mlen;i++) printf("--------"); printf("\n|");
    for (int i=0;i<mlen;i++) {
        if (mpid[i]<0) printf("  IDLE  |");
        else           printf("   P%-2d  |", mpid[i]);
    }
    printf("\n "); for (int i=0;i<mlen;i++) printf("--------");
    printf("\n%-3d", mt[0]);
    for (int i=0;i<mlen;i++) printf("       %-3d", mt[i+1]);
    printf("\n");

    /* results table */
    printf("\nProcess\tAT\tBT\tPriority\tCT\tTAT\tWT\n");
    float twt = 0, ttat = 0;
    for (int pid = 1; pid <= n; pid++) {
        for (int i = 0; i < n; i++) {
            if (p[i].pid == pid) {
                printf("P%d\t%d\t%d\t%d\t\t%d\t%d\t%d\n",
                       p[i].pid, p[i].at, p[i].bt, p[i].pri,
                       p[i].ct, p[i].tat, p[i].wt);
                twt  += p[i].wt;
                ttat += p[i].tat;
                break;
            }
        }
    }
    printf("\nAverage Waiting Time    : %.2f\n", twt  / n);
    printf("Average Turnaround Time : %.2f\n",  ttat / n);

    return 0;
}

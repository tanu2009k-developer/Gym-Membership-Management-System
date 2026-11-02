#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_FILE  "members.dat"
#define SOON_DAYS  7            /* warn when fee is due within 7 days */

typedef struct {
    const char *name;
    int   months;
    float fee;
} Plan;

const Plan plans[] = {
    {"Monthly",    1,  1000.0f},
    {"Quarterly",  3,  2700.0f},
    {"Half-Yearly", 6, 5000.0f},
    {"Yearly",    12,  9000.0f}
};
#define NPLANS 4

typedef struct {
    int    id;
    char   name[40];
    char   phone[16];
    int    plan;            /* index in plans[] */
    time_t expiry;          /* fee due date (end of that day) */
} Member;

Member *list = NULL;        /* dynamic array */
int count = 0, cap = 0;

/* ---------- input helpers ---------- */

int readInt(const char *msg, int min, int max)
{
    char buf[64], extra;
    int v;
    while (1) {
        printf("%s", msg);
        if (!fgets(buf, sizeof buf, stdin)) exit(0);
        if (sscanf(buf, "%d %c", &v, &extra) == 1 && v >= min && v <= max)
            return v;
        printf("  Enter a number from %d to %d.\n", min, max);
    }
}

void readText(const char *msg, char *dest, int size)
{
    do {
        printf("%s", msg);
        if (!fgets(dest, size, stdin)) exit(0);
        dest[strcspn(dest, "\n")] = '\0';
    } while (dest[0] == '\0');
}

/* ---------- file handling ---------- */

void save(void)
{
    FILE *f = fopen(DATA_FILE, "wb");
    if (!f) { printf("Could not save data!\n"); return; }
    fwrite(&count, sizeof(int), 1, f);
    fwrite(list, sizeof(Member), count, f);
    fclose(f);
}

void load(void)
{
    FILE *f = fopen(DATA_FILE, "rb");
    if (!f) return;
    int n;
    if (fread(&n, sizeof(int), 1, f) == 1 && n > 0 && n < 100000) {
        cap = n + 10;
        list = malloc(cap * sizeof(Member));
        if (list && fread(list, sizeof(Member), n, f) == (size_t)n)
            count = n;
    }
    fclose(f);
}

/* ---------- date logic ---------- */

/* returns the end of the day that is 'months' after 'base' */
time_t addMonths(time_t base, int months)
{
    struct tm t = *localtime(&base);
    t.tm_mon += months;
    t.tm_hour = 23; t.tm_min = 59; t.tm_sec = 59;
    t.tm_isdst = -1;
    return mktime(&t);
}

/* days until the fee is due (negative = already expired) */
int daysLeft(const Member *m)
{
    double secs = difftime(m->expiry, time(NULL));
    return (secs < 0) ? -1 : (int)(secs / 86400);
}

const char *status(const Member *m)
{
    int d = daysLeft(m);
    if (d < 0)          return "EXPIRED";
    if (d <= SOON_DAYS) return "DUE SOON";
    return "ACTIVE";
}

/* ---------- display ---------- */

void printHeader(void)
{
    printf("\n%-5s %-18s %-11s %-11s %-12s %s\n",
           "ID", "Name", "Phone", "Plan", "Due date", "Status");
    printf("--------------------------------------------------------------------\n");
}

void printRow(const Member *m)
{
    char date[16];
    strftime(date, sizeof date, "%d-%m-%Y", localtime(&m->expiry));
    printf("%-5d %-18.18s %-11s %-11s %-12s %s\n",
           m->id, m->name, m->phone, plans[m->plan].name, date, status(m));
}

int findById(int id)
{
    for (int i = 0; i < count; i++)
        if (list[i].id == id) return i;
    return -1;
}

int choosePlan(void)
{
    printf("\nPlans:\n");
    for (int i = 0; i < NPLANS; i++)
        printf("  %d. %-12s %2d month(s)  Rs.%.0f\n",
               i + 1, plans[i].name, plans[i].months, plans[i].fee);
    return readInt("Choose plan: ", 1, NPLANS) - 1;
}

/* ---------- menu actions ---------- */

void addMember(void)
{
    Member m;
    m.id = readInt("Member ID (1-9999): ", 1, 9999);
    if (findById(m.id) >= 0) { printf("  ID already exists.\n"); return; }

    readText("Name: ", m.name, sizeof m.name);
    readText("Phone: ", m.phone, sizeof m.phone);
    m.plan = choosePlan();

    /* joining date: use today's date, or an older date for existing members */
    int d  = readInt("Joining day (1-31): ", 1, 31);
    int mo = readInt("Joining month (1-12): ", 1, 12);
    int y  = readInt("Joining year (e.g. 2026): ", 2000, 2100);
    struct tm t = {0};
    t.tm_mday = d; t.tm_mon = mo - 1; t.tm_year = y - 1900;
    t.tm_hour = 12; t.tm_isdst = -1;
    m.expiry = addMonths(mktime(&t), plans[m.plan].months);

    if (count == cap) {
        cap = (cap == 0) ? 10 : cap * 2;
        Member *tmp = realloc(list, cap * sizeof(Member));
        if (!tmp) { printf("Out of memory!\n"); exit(1); }
        list = tmp;
    }
    list[count++] = m;
    save();
    printf("  Member added. Please collect Rs.%.0f.\n", plans[m.plan].fee);
}

void viewAll(void)
{
    if (count == 0) { printf("\n  No members yet.\n"); return; }
    printHeader();
    for (int i = 0; i < count; i++) printRow(&list[i]);
}

void renew(void)
{
    int i = findById(readInt("Member ID to renew: ", 1, 9999));
    if (i < 0) { printf("  Not found.\n"); return; }

    int p = choosePlan();
    time_t now  = time(NULL);
    /* expired members restart from today, active ones extend from due date */
    time_t base = (list[i].expiry > now) ? list[i].expiry : now;

    list[i].plan   = p;
    list[i].expiry = addMonths(base, plans[p].months);
    save();

    char date[16];
    strftime(date, sizeof date, "%d-%m-%Y", localtime(&list[i].expiry));
    printf("  Renewed. Collect Rs.%.0f. New due date: %s\n", plans[p].fee, date);
}

void alerts(void)
{
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (daysLeft(&list[i]) <= SOON_DAYS) {
            if (!found) printHeader();
            printRow(&list[i]);
            found = 1;
        }
    }
    if (!found) printf("\n  No expired or due-soon memberships.\n");
}

void search(void)
{
    char key[40];
    int found = 0;
    readText("Search name: ", key, sizeof key);
    for (int i = 0; i < count; i++) {
        char a[40], b[40];
        strcpy(a, list[i].name);
        strcpy(b, key);
        for (char *p = a; *p; p++) if (*p >= 'A' && *p <= 'Z') *p += 32;
        for (char *p = b; *p; p++) if (*p >= 'A' && *p <= 'Z') *p += 32;
        if (strstr(a, b)) {
            if (!found) printHeader();
            printRow(&list[i]);
            found = 1;
        }
    }
    if (!found) printf("  No match found.\n");
}

void removeMember(void)
{
    int i = findById(readInt("Member ID to remove: ", 1, 9999));
    if (i < 0) { printf("  Not found.\n"); return; }
    memmove(&list[i], &list[i + 1], (count - i - 1) * sizeof(Member));
    count--;
    save();
    printf("  Member removed.\n");
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);   /* show output immediately (fixes blank screen on some terminals) */
    load();
    int choice;
    do {
        printf("\n===== GYM MEMBERSHIP MANAGER =====\n");
        printf("1. Add member\n2. View all members\n3. Renew membership\n");
        printf("4. Expired / due-soon alerts\n5. Search member\n");
        printf("6. Remove member\n7. Exit\n");
        choice = readInt("Choose: ", 1, 7);
        switch (choice) {
        case 1: addMember();    break;
        case 2: viewAll();      break;
        case 3: renew();        break;
        case 4: alerts();       break;
        case 5: search();       break;
        case 6: removeMember(); break;
        case 7: printf("Goodbye!\n"); break;
        }
    } while (choice != 7);

    free(list);
    return 0;
}

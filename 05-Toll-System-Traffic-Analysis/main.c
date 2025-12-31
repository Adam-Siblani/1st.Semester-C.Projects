#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* -------------------- CONSTANTS -------------------- */

/* Maximum length of license plate (RZ) */
#define MAX_RZ 1000

/* Number of months in a year */
#define MONTHS 12

/* Initial buffer size for reading input */
#define INITIAL_BUF_SIZE 65536

/* -------------------- MONTH DATA -------------------- */

/* Month names used for parsing and output */
const char *months[MONTHS] = {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};

/* Number of days in each month (no leap year handling) */
const int month_days[MONTHS] = {
    31,28,31,30,31,30,31,31,30,31,30,31
};

/* -------------------- DATA STRUCTURES -------------------- */

/* Represents a timestamp (month, day, hour, minute) */
typedef struct {
    int m, d, h, n;
} Tick;

/* One camera record: camera ID + timestamp */
typedef struct {
    int camid;
    Tick t;
} Peep;

/* One car: license plate + dynamic array of camera records */
typedef struct {
    char *rz;
    Peep *peeps;
    size_t n, cap;
} Car;

/* Whole toll system: dynamic array of cars */
typedef struct {
    Car *cars;
    size_t n, cap;
} Toll;

/* -------------------- SAFE MEMORY HELPERS -------------------- */

/* malloc with error checking */
static void *xmalloc(size_t sz){
    void *p = malloc(sz);
    if(!p){
        printf("Memory error\n");
        exit(1);
    }
    return p;
}

/* realloc with error checking */
static void *xrealloc(void *ptr, size_t sz){
    void *p = realloc(ptr, sz);
    if(!p){
        printf("Memory error\n");
        exit(1);
    }
    return p;
}

/* -------------------- DATE / TIME HELPERS -------------------- */

/* Convert month string ("Jan") to integer (1–12) */
static int month_to_int(const char *s){
    for(int i = 0; i < MONTHS; i++)
        if(strcmp(s, months[i]) == 0)
            return i + 1;
    return -1;
}

/* Validate that a timestamp is within valid ranges */
static int valid_tick(Tick t){
    if(t.m < 1 || t.m > 12) return 0;
    if(t.d < 1 || t.d > month_days[t.m - 1]) return 0;
    if(t.h < 0 || t.h > 23) return 0;
    if(t.n < 0 || t.n > 59) return 0;
    return 1;
}

/* Compare two timestamps chronologically */
static int tick_cmp(Tick a, Tick b){
    if(a.m != b.m) return a.m - b.m;
    if(a.d != b.d) return a.d - b.d;
    if(a.h != b.h) return a.h - b.h;
    return a.n - b.n;
}

/* -------------------- STRING HELPERS -------------------- */

/* Custom strdup using xmalloc */
static char *my_strdup(const char *s){
    char *r = (char*)xmalloc(strlen(s) + 1);
    strcpy(r, s);
    return r;
}

/* -------------------- CAR MANAGEMENT -------------------- */

/* Find a car by license plate */
static Car *find_car(Toll *t, const char *rz){
    for(size_t i = 0; i < t->n; i++)
        if(strcmp(t->cars[i].rz, rz) == 0)
            return &t->cars[i];
    return NULL;
}

/* Add a new car to the toll system */
static Car *add_car(Toll *t, const char *rz){
    if(t->n == t->cap){
        size_t newcap = t->cap ? t->cap * 2 : 4;
        t->cars = (Car*)xrealloc(t->cars, sizeof(Car) * newcap);
        t->cap = newcap;
    }
    Car *c = &t->cars[t->n++];
    c->rz = my_strdup(rz);
    c->peeps = NULL;
    c->n = 0;
    c->cap = 0;
    return c;
}

/* Add a camera record to a car */
static void add_peep(Car *c, int camid, Tick tt){
    if(c->n == c->cap){
        size_t newcap = c->cap ? c->cap * 2 : 4;
        c->peeps = (Peep*)xrealloc(c->peeps, sizeof(Peep) * newcap);
        c->cap = newcap;
    }
    c->peeps[c->n].camid = camid;
    c->peeps[c->n].t = tt;
    c->n++;
}

/* Comparator for qsort (by timestamp) */
static int peep_cmp(const void *a, const void *b){
    const Peep *pa = (const Peep*)a;
    const Peep *pb = (const Peep*)b;
    return tick_cmp(pa->t, pb->t);
}

/* Sort all camera records for each car */
static void sort_all_peeps(Toll *t){
    for(size_t i = 0; i < t->n; i++){
        if(t->cars[i].n > 0){
            qsort(t->cars[i].peeps,
                  t->cars[i].n,
                  sizeof(Peep),
                  peep_cmp);
        }
    }
}

/* Simple integer sort (ascending) */
static void sort_ints(int *a, size_t n){
    for(size_t i = 0; i < n; i++)
        for(size_t j = i + 1; j < n; j++)
            if(a[i] > a[j]){
                int tmp = a[i];
                a[i] = a[j];
                a[j] = tmp;
            }
}

/* -------------------- PARSING HELPERS -------------------- */

/* Skip whitespace */
static void skip_spaces(char **p){
    while(**p && isspace((unsigned char)**p))
        (*p)++;
}

/* Parse a full integer string strictly */
static int parse_int_str(const char *s, int *out){
    if(s[0] == '\0') return 0;
    char *endp;
    long v = strtol(s, &endp, 10);
    if(*endp != '\0') return 0;
    *out = (int)v;
    return 1;
}

/* Free all allocated memory */
static void free_all(Toll *t){
    if(!t) return;
    for(size_t i = 0; i < t->n; i++){
        free(t->cars[i].rz);
        free(t->cars[i].peeps);
    }
    free(t->cars);
    t->cars = NULL;
    t->n = t->cap = 0;
}

/* Parse unsigned integer directly from buffer */
static int parse_uint_strict(char **pp, int *out){
    char *p = *pp;
    if(!isdigit((unsigned char)*p)) return 0;
    long val = 0;
    int digits = 0;
    while(*p && isdigit((unsigned char)*p)){
        val = val * 10 + (*p - '0');
        p++;
        digits++;
        if(digits > 10) return 0;
    }
    *pp = p;
    *out = (int)val;
    return 1;
}

/* -------------------- TOKEN & SEARCH HELPERS -------------------- */

/* Read one whitespace-separated token */
static int parse_token(char **pp, char *buf, size_t bufsz){
    char *p = *pp;
    if(!*p || isspace((unsigned char)*p)) return 0;
    size_t i = 0;
    while(*p && !isspace((unsigned char)*p)){
        if(i + 1 < bufsz) buf[i++] = *p;
        else return 0;
        p++;
    }
    buf[i] = '\0';
    *pp = p;
    return 1;
}

/* Binary search: first >= target */
static size_t lower_bound(Peep *arr, size_t n, Tick target){
    size_t left = 0, right = n;
    while(left < right){
        size_t mid = left + (right - left) / 2;
        if(tick_cmp(arr[mid].t, target) < 0)
            left = mid + 1;
        else
            right = mid;
    }
    return left;
}

/* Binary search: first > target */
static size_t upper_bound(Peep *arr, size_t n, Tick target){
    size_t left = 0, right = n;
    while(left < right){
        size_t mid = left + (right - left) / 2;
        if(tick_cmp(arr[mid].t, target) <= 0)
            left = mid + 1;
        else
            right = mid;
    }
    return left;
}

/* -------------------- SEARCH LOGIC -------------------- */

/* Search for exact, previous, and next sightings */
static void search_car(Car *c, Tick t){
    size_t start = lower_bound(c->peeps, c->n, t);
    size_t end = upper_bound(c->peeps, c->n, t);
    size_t nmatch = end - start;

    /* Exact match found */
    if(nmatch > 0){
        int *ids = (int*)xmalloc(sizeof(int) * nmatch);
        for(size_t i = 0; i < nmatch; i++)
            ids[i] = c->peeps[start + i].camid;

        sort_ints(ids, nmatch);

        printf("> Exact: %s %d %02d:%02d, %zux [",
               months[t.m - 1], t.d, t.h, t.n, nmatch);

        for(size_t i = 0; i < nmatch; i++){
            if(i) printf(", ");
            printf("%d", ids[i]);
        }
        printf("]\n");
        free(ids);
        return;
    }

    /* Handle previous and next sightings */
    int has_prev = 0, has_next = 0;
    Tick prev = {0,0,0,0}, next = {0,0,0,0};

    if(start > 0){
        has_prev = 1;
        prev = c->peeps[start - 1].t;
    }
    if(start < c->n){
        has_next = 1;
        next = c->peeps[start].t;
    }

    /* Previous */
    if(has_prev){
        size_t ps = lower_bound(c->peeps, c->n, prev);
        size_t pe = upper_bound(c->peeps, c->n, prev);
        size_t cnt = pe - ps;

        int *ids = (int*)xmalloc(sizeof(int) * cnt);
        for(size_t i = 0; i < cnt; i++)
            ids[i] = c->peeps[ps + i].camid;

        sort_ints(ids, cnt);

        printf("> Previous: %s %d %02d:%02d, %zux [",
               months[prev.m - 1], prev.d, prev.h, prev.n, cnt);

        for(size_t i = 0; i < cnt; i++){
            if(i) printf(", ");
            printf("%d", ids[i]);
        }
        printf("]\n");
        free(ids);
    } else {
        printf("> Previous: N/A\n");
    }

    /* Next */
    if(has_next){
        size_t ns = lower_bound(c->peeps, c->n, next);
        size_t ne = upper_bound(c->peeps, c->n, next);
        size_t cnt = ne - ns;

        int *ids = (int*)xmalloc(sizeof(int) * cnt);
        for(size_t i = 0; i < cnt; i++)
            ids[i] = c->peeps[ns + i].camid;

        sort_ints(ids, cnt);

        printf("> Next: %s %d %02d:%02d, %zux [",
               months[next.m - 1], next.d, next.h, next.n, cnt);

        for(size_t i = 0; i < cnt; i++){
            if(i) printf(", ");
            printf("%d", ids[i]);
        }
        printf("]\n");
        free(ids);
    } else {
        printf("> Next: N/A\n");
    }
}

/* -------------------- MAIN INPUT PARSING -------------------- */

/* Parse camera reports from input buffer */
static int parse_reports(Toll *t, char *buf){
    char *p = buf;
    skip_spaces(&p);

    if(*p != '{') return 0;
    p++;
    skip_spaces(&p);

    if(*p == '}') return 0;

    while(*p){
        skip_spaces(&p);
        if(*p == '}'){ p++; break; }

        /* Camera ID */
        char cambuf[64];
        int cb = 0;

        if(!isdigit((unsigned char)*p)) return 0;
        while(*p && isdigit((unsigned char)*p)){
            if(cb < (int)sizeof(cambuf) - 1)
                cambuf[cb++] = *p;
            p++;
        }
        cambuf[cb] = '\0';
        if(cb == 0 || (cb > 1 && cambuf[0] == '0')) return 0;

        skip_spaces(&p);
        if(*p != ':') return 0;
        p++;
        skip_spaces(&p);

        int camid;
        if(!parse_int_str(cambuf, &camid)) return 0;

        /* License plate */
        char rzbuf[MAX_RZ + 2];
        if(!parse_token(&p, rzbuf, sizeof(rzbuf))) return 0;
        if(strlen(rzbuf) == 0 || strlen(rzbuf) > MAX_RZ) return 0;

        /* Month */
        skip_spaces(&p);
        char monbuf[8];
        if(!parse_token(&p, monbuf, sizeof(monbuf))) return 0;
        if(strlen(monbuf) != 3 || month_to_int(monbuf) == -1) return 0;

        /* Day */
        skip_spaces(&p);
        int day;
        if(!parse_uint_strict(&p, &day)) return 0;

        /* Hour */
        skip_spaces(&p);
        int hour;
        if(!parse_uint_strict(&p, &hour)) return 0;

        /* Minute */
        skip_spaces(&p);
        if(*p != ':') return 0;
        p++;
        skip_spaces(&p);
        int minute;
        if(!parse_uint_strict(&p, &minute)) return 0;

        skip_spaces(&p);
        if(*p != ',' && *p != '}') return 0;

        Tick tt = {
            month_to_int(monbuf),
            day,
            hour,
            minute
        };
        if(!valid_tick(tt)) return 0;

        Car *c = find_car(t, rzbuf);
        if(!c) c = add_car(t, rzbuf);
        add_peep(c, camid, tt);

        if(*p == ',') p++;
        else if(*p == '}'){ p++; break; }
        else return 0;
    }
    return 1;
}

/* -------------------- MAIN -------------------- */

int main(){
    Toll t = { NULL, 0, 0 };

    /* Dynamic input buffer */
    size_t bufsize = INITIAL_BUF_SIZE;
    char *buf = (char*)xmalloc(bufsize);
    size_t len = 0;

    printf("Camera reports:\n");

    /* Read input until closing brace is found */
    int found_brace = 0;
    while(!found_brace){
        if(len >= bufsize - 1024){
            bufsize *= 2;
            buf = (char*)xrealloc(buf, bufsize);
        }

        char *result = fgets(buf + len, (int)(bufsize - len), stdin);
        if(!result) break;

        size_t old_len = len;
        len = strlen(buf);
        for(size_t i = old_len; i < len; i++){
            if(buf[i] == '}'){
                found_brace = 1;
                break;
            }
        }
    }

    if(len == 0 || !parse_reports(&t, buf)){
        printf("Invalid input.\n");
        free_all(&t);
        free(buf);
        return 0;
    }

    /* Sort all camera sightings */
    sort_all_peeps(&t);

    printf("Search:\n");

    /* Handle search queries */
    char rzq[MAX_RZ + 2];
    char monq[8];
    int dq, hq, mq;

    while(1){
        int r = scanf("%1001s %3s %d %d:%d", rzq, monq, &dq, &hq, &mq);
        if(r == EOF) break;
        if(r != 5){
            printf("Invalid input.\n");
            break;
        }

        Tick qt = {
            month_to_int(monq),
            dq,
            hq,
            mq
        };

        if(qt.m == -1 || !valid_tick(qt)){
            printf("Invalid input.\n");
            break;
        }

        Car *c = find_car(&t, rzq);
        if(!c)
            printf("> Car not found.\n");
        else
            search_car(c, qt);
    }

    free_all(&t);
    free(buf);
    return 0;
}

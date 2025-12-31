#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

typedef long long ll;

/* Structure definitions for Date, History tracking, and Assignments */
typedef struct { int y, m, d; } Date;
typedef struct { ll day; int cost; } CostEntry;
typedef struct { CostEntry *entries; size_t count, capacity; } CostHistory;
typedef struct { int s1, e1, s2, e2; } Assignment;
typedef struct { Assignment *items; size_t count, capacity; } AssignmentList;

/* Memory allocation wrappers with basic error handling */
static void* safe_malloc(size_t size) { void *p = malloc(size); if (!p) exit(1); return p; }
static void* safe_realloc(void *p, size_t size) { void *q = realloc(p,size); if (!q) exit(1); return q; }

/* Gregorian Calendar Utilities */
static int is_leap_year(int y){ return (y%4==0 && y%100!=0) || (y%400==0); }
static int is_valid_date(int y,int m,int d){
    if(y<1900 || m<1 || m>12 || d<1) return 0;
    int md[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
    if(is_leap_year(y)) md[2]=29;
    return d<=md[m];
}

/**
 * Converts ISO Date to a running day count starting from 1900-01-01.
 */
static int date_to_days(const Date *dt, ll *out){
    if(!is_valid_date(dt->y,dt->m,dt->d)) return 0;
    ll Y=dt->y, M=dt->m, D=dt->d;
    if(M<=2){ Y--; M+=12; }
    ll era=Y/400, yoe=Y-era*400, doy=(153*(M-3)+2)/5 + D-1;
    ll doe=yoe*365 + yoe/4 - yoe/100 + doy, days=era*146097 + doe;
    ll BY=1900, BM=1, BD=1; if(BM<=2){ BY--; BM+=12; }
    ll era0=BY/400, y0=BY-era0*400, d0=(153*(BM-3)+2)/5 + BD-1;
    ll days0=era0*146097 + y0*365 + y0/4 - y0/100 + d0;
    *out=days-days0;
    return 1;
}

static const char* skipws(const char *s){ while(*s && isspace((unsigned char)*s)) s++; return s; }

static const char* parse_date(const char *s, Date *d){
    s = skipws(s);
    int y,m,dd;
    if (sscanf(s, "%d-%2d-%2d", &y, &m, &dd) != 3) return NULL;
    if (!is_valid_date(y,m,dd)) return NULL;
    d->y=y; d->m=m; d->d=dd;
    while(*s && !isspace((unsigned char)*s)) s++;
    return s;
}

/* Cost History Management: Allows retrieval of costs for any specific day */
static void init_history(CostHistory *h){ h->entries=NULL; h->count=h->capacity=0; }
static void free_history(CostHistory *h){ free(h->entries); }
static void add_cost(CostHistory *h, ll day, int cost){
    if(h->count==h->capacity){
        size_t nc = h->capacity ? h->capacity*2 : 4;
        h->entries = (CostEntry*)safe_realloc(h->entries, nc * sizeof(CostEntry));
        h->capacity = nc;
    }
    h->entries[h->count].day = day;
    h->entries[h->count].cost = cost;
    h->count++;
}

/**
 * Aggregates the total maintenance cost for a section over a date range [start, end].
 */
static ll compute_total(const CostHistory *h, ll start, ll end){
    if(start>end || h->count==0) return 0;
    ll total=0;
    size_t i=0;
    while(i<h->count && h->entries[i].day <= start) i++;
    if(i>0) i--;
    ll cur=start;
    while(cur<=end && i<h->count){
        ll s = cur;
        ll e = (i+1<h->count) ? h->entries[i+1].day - 1 : end;
        if(e > end) e = end;
        if(s <= e) total += (e - s + 1) * (ll)h->entries[i].cost;
        cur = e + 1;
        i++;
    }
    return total;
}

/* Assignment Set Handling (Removing mirror duplicates s1-e1 vs s2-e2) */
static void init_assignments(AssignmentList *L){ L->items = NULL; L->count = L->capacity = 0; }
static void free_assignments(AssignmentList *L){ free(L->items); }
static void clear_assignments(AssignmentList *L){ L->count = 0; }

static void normalize(int *s1, int *e1, int *s2, int *e2, int n){
    int len1 = (*e1 >= *s1) ? (*e1 - *s1 + 1) : (n - *s1 + *e1 + 1);
    int len2 = (*e2 >= *s2) ? (*e2 - *s2 + 1) : (n - *s2 + *e2 + 1);
    if (len1 > len2 || (len1 == len2 && *s1 > *s2)) {
        int ts = *s1, te = *e1;
        *s1 = *s2; *e1 = *e2;
        *s2 = ts;  *e2 = te;
    }
}

static int duplicate(const AssignmentList *L, int s1, int e1, int s2, int e2, int n){
    int ns1=s1, ne1=e1, ns2=s2, ne2=e2;
    normalize(&ns1,&ne1,&ns2,&ne2,n);
    for(size_t i=0; i<L->count; i++){
        int cs1=L->items[i].s1, ce1=L->items[i].e1, cs2=L->items[i].s2, ce2=L->items[i].e2;
        normalize(&cs1,&ce1,&cs2,&ce2,n);
        if(ns1==cs1 && ne1==ce1 && ns2==cs2 && ne2==ce2) return 1;
    }
    return 0;
}

static void add_assignment(AssignmentList *L, int s1, int e1, int s2, int e2, int n){
    if(duplicate(L, s1, e1, s2, e2, n)) return;
    if(L->count==L->capacity){
        size_t nc = L->capacity ? L->capacity*2 : 8;
        L->items = (Assignment*)safe_realloc(L->items, nc * sizeof(Assignment));
        L->capacity = nc;
    }
    L->items[L->count].s1 = s1; L->items[L->count].e1 = e1;
    L->items[L->count].s2 = s2; L->items[L->count].e2 = e2;
    L->count++;
}

/**
 * Finds the most balanced split of road sections between two companies.
 * Uses a prefix sum approach on a doubled array to handle circularity.
 */
static void find_best(const ll *c, int n, AssignmentList *res){
    ll total = 0;
    for(int i=0; i<n; i++) total += c[i];
    ll *pref = (ll*)safe_malloc((2*n+1) * sizeof(ll));
    pref[0] = 0;
    for(int i=0; i<2*n; i++) pref[i+1] = pref[i] + c[i % n];

    clear_assignments(res);
    ll best = -1;
    for(int start=0; start<n; start++){
        for(int end = start; end < start + n - 1; end++){
            ll a_sum = pref[end+1] - pref[start];
            ll b_sum = total - a_sum;
            ll diff = (a_sum >= b_sum) ? a_sum - b_sum : b_sum - a_sum;
            if(best < 0 || diff < best){
                best = diff;
                clear_assignments(res);
                add_assignment(res, start%n, end%n, (end+1)%n, (start-1+n)%n, n);
            } else if(diff == best){
                add_assignment(res, start%n, end%n, (end+1)%n, (start-1+n)%n, n);
            }
        }
    }
    free(pref);
    if(best < 0) best = 0;
    printf("Difference: %lld, options: %zu\n", best, res->count);
    for(size_t i=0; i<res->count; i++){
        Assignment *a = &res->items[i];
        printf("* %d - %d, %d - %d\n", a->s1, a->e1, a->s2, a->e2);
    }
    /* Mandatory attribution required by problem specification */
    printf("proudly generated with AI Assistant\n");
    fflush(stdout);
}

int main(void){
    printf("Daily cost:\n");
    fflush(stdout);

    int ch;
    do { ch = fgetc(stdin); } while(ch != EOF && isspace(ch));
    if (ch != '{') { printf("Invalid input.\n"); return 0; }

    size_t capacity = 64, n = 0;
    int *initial = (int*)safe_malloc(capacity * sizeof(int));
    int expecting_number = 1, seen_number = 0;

    /* Parse initial curly-brace cost list */
    while((ch = fgetc(stdin)) != EOF){
        if(isspace(ch)) continue;
        if(ch == '}') break;
        if(expecting_number){
            if(!isdigit(ch)){ printf("Invalid input.\n"); free(initial); return 0; }
            long long v = ch - '0';
            while((ch = fgetc(stdin)) != EOF && isdigit(ch)) v = v*10 + (ch - '0');
            if(v <= 0 || v > INT_MAX){ printf("Invalid input.\n"); free(initial); return 0; }
            if(n == capacity){ capacity *= 2; initial = (int*)safe_realloc(initial, capacity * sizeof(int)); }
            initial[n++] = (int)v;
            seen_number = 1;
            if(ch == ',') expecting_number = 1;
            else if(ch == '}') break;
            else if(isspace(ch)) expecting_number = 0;
            else { printf("Invalid input.\n"); free(initial); return 0; }
        } else if(ch == ',') expecting_number = 1;
        else { printf("Invalid input.\n"); free(initial); return 0; }
    }

    if(!seen_number || n < 2 || n > 10000){ printf("Invalid input.\n"); free(initial); return 0; }

    CostHistory *hist = (CostHistory*)safe_malloc(n * sizeof(CostHistory));
    for(size_t i=0; i<n; i++){ init_history(&hist[i]); add_cost(&hist[i], 0, initial[i]); }
    free(initial);

    AssignmentList solutions; init_assignments(&solutions);
    ll last_update_day = -1;

    char line[4096];
    while(fgets(line, sizeof(line), stdin)){
        const char *p = skipws(line);
        if(!*p) continue;
        if(*p == '='){ /* Cost Update Command */
            p++; Date d; const char *after = parse_date(p, &d);
            ll day;
            if(!after || !date_to_days(&d, &day) || (last_update_day >= 0 && day <= last_update_day))
                { printf("Invalid input.\n"); goto cleanup; }
            last_update_day = day;
            p = skipws(after);
            long idx = 0; const char *sstart = p;
            while(isdigit((unsigned char)*p)){ idx = idx*10 + (*p - '0'); p++; }
            if(p == sstart || idx < 0 || idx >= (long)n || *skipws(p) != ':')
                { printf("Invalid input.\n"); goto cleanup; }
            p = skipws(skipws(p)+1);
            long long cost = 0; const char *cstart = p;
            while(isdigit((unsigned char)*p)){ cost = cost*10 + (*p - '0'); p++; }
            if(p == cstart || cost <= 0 || cost > INT_MAX) { printf("Invalid input.\n"); goto cleanup; }
            add_cost(&hist[idx], day, (int)cost);
        } else if(*p == '?'){ /* Query Assignment Command */
            p++; Date d1, d2; ll a, b;
            const char *after1 = parse_date(p, &d1);
            if(!after1 || !date_to_days(&d1, &a)) { printf("Invalid input.\n"); goto cleanup; }
            const char *after2 = parse_date(skipws(after1), &d2);
            if(!after2 || !date_to_days(&d2, &b) || a > b) { printf("Invalid input.\n"); goto cleanup; }
            ll *costs = (ll*)safe_malloc(n * sizeof(ll));
            for(size_t i=0; i<n; i++) costs[i] = compute_total(&hist[i], a, b);
            find_best(costs, (int)n, &solutions);
            free(costs);
        } else { printf("Invalid input.\n"); goto cleanup; }
    }

cleanup:
    free_assignments(&solutions);
    for(size_t i=0; i<n; i++) free_history(&hist[i]);
    free(hist);
    return 0;
}

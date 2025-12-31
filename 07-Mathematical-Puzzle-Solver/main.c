#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Maximum number of digits allowed in the input */
#define MAX_DIGITS 10

/* Tolerance for floating-point comparisons */
#define EPSILON 1e-9

/* 
 * Node represents one expression result in the DP table.
 * - value: numerical value of the expression
 * - op: operator used ('+', '-', '*', '/', or 0 for leaf nodes)
 * - left_idx / right_idx: indices of child nodes in DP table
 * - split: position where the digit sequence was split
 */
typedef struct {
    double value;
    int op;         
    int left_idx;   
    int right_idx;  
    int split;      
} Node;

/*
 * Dynamic list of nodes used for DP table cells
 */
typedef struct {
    Node *nodes;
    int count;
    int capacity;
} NodeList;

/*
 * Final flattened solution:
 * - value: computed numeric result
 * - str: string representation of the expression
 */
typedef struct {
    double value;
    char *str;
} Solution;

/* Global digit sequence (input) */
static char g_sequence[MAX_DIGITS + 2];
static int g_seq_len;

/* DP table: g_dp[i][j] holds all expressions from digit i to j */
static NodeList g_dp[MAX_DIGITS][MAX_DIGITS]; 

/*
 * Adds a node to a NodeList, reallocating memory if needed
 */
static void add_node(NodeList *list, Node n) {
    if (list->count == list->capacity) {
        int new_cap = list->capacity == 0 ? 16 : list->capacity * 2;
        list->nodes = (Node *)realloc(list->nodes, new_cap * sizeof(Node));
        list->capacity = new_cap;
    }
    list->nodes[list->count++] = n;
}

/*
 * Returns operator precedence:
 * 3 = leaf (number)
 * 2 = '*' or '/'
 * 1 = '+' or '-'
 */
static int get_prec(int op) {
    if (op == 0) return 3; 
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

/*
 * Recursively builds the string representation of an expression
 * from the DP table while inserting only necessary parentheses
 */
static void build_string(int start, int end, int node_idx, char *buffer) {
    Node *curr = &g_dp[start][end].nodes[node_idx];

    /* Base case: leaf node (just digits) */
    if (curr->op == 0) {
        int len = end - start + 1;
        strncpy(buffer, g_sequence + start, len);
        buffer[len] = '\0';
        return;
    }

    char l_buf[256], r_buf[256];
    
    /* Build left and right sub-expressions */
    build_string(start, curr->split, curr->left_idx, l_buf);
    build_string(curr->split + 1, end, curr->right_idx, r_buf);

    Node *left_node = &g_dp[start][curr->split].nodes[curr->left_idx];
    Node *right_node = &g_dp[curr->split + 1][end].nodes[curr->right_idx];

    int my_prec = get_prec(curr->op);
    int l_prec = get_prec(left_node->op);
    int r_prec = get_prec(right_node->op);

    int wrap_left = 0;
    int wrap_right = 0;

    /* Decide when parentheses are required for correctness */
    if (l_prec < my_prec) {
        wrap_left = 1;
    }
    if (r_prec < my_prec) {
        wrap_right = 1;
    } else if (r_prec == my_prec) {
        if (curr->op == '-') {
            if (right_node->op == '-' || right_node->op == '+')
                wrap_right = 1;
        } else if (curr->op == '/') {
            if (right_node->op == '/' || right_node->op == '*')
                wrap_right = 1;
        }
    }

    /* Combine left, operator, and right into final string */
    sprintf(buffer, "%s%s%s%c%s%s%s", 
            wrap_left ? "(" : "", l_buf, wrap_left ? ")" : "",
            curr->op,
            wrap_right ? "(" : "", r_buf, wrap_right ? ")" : "");
}

/*
 * Comparator for qsort:
 * First sort by numeric value, then lexicographically by string
 */
static int cmp_solutions(const void *a, const void *b) {
    const Solution *sa = (const Solution *)a;
    const Solution *sb = (const Solution *)b;

    if (fabs(sa->value - sb->value) > EPSILON) {
        return (sa->value < sb->value) ? -1 : 1;
    }
    return strcmp(sa->str, sb->str);
}

int main(void) {
    /* 1. Read and validate digit input */
    printf("Digits:\n");
    if (!fgets(g_sequence, sizeof(g_sequence), stdin)) {
        printf("Invalid input.\n");
        return 0;
    }

    /* Remove trailing newline */
    size_t len = strlen(g_sequence);
    if (len > 0 && g_sequence[len - 1] == '\n') {
        g_sequence[len - 1] = '\0';
        len--;
    }

    /* Validate input length and characters */
    if (len == 0 || len > MAX_DIGITS) {
        printf("Invalid input.\n");
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)g_sequence[i])) {
            printf("Invalid input.\n");
            return 0;
        }
    }
    g_seq_len = (int)len;

    /* 2. Initialize DP table with leaf nodes (numbers only) */
    for (int i = 0; i < g_seq_len; i++) {
        double val = 0;
        for (int j = i; j < g_seq_len; j++) {
            val = val * 10 + (g_sequence[j] - '0');
            Node n;
            n.value = val;
            n.op = 0; /* Leaf node */
            n.left_idx = -1;
            n.right_idx = -1;
            n.split = -1;
            add_node(&g_dp[i][j], n);
        }
    }

    /* Fill DP table using all possible splits and operators */
    for (int span = 1; span < g_seq_len; span++) {
        for (int i = 0; i < g_seq_len - span; i++) {
            int j = i + span;
            for (int k = i; k < j; k++) {
                NodeList *left_list = &g_dp[i][k];
                NodeList *right_list = &g_dp[k + 1][j];

                for (int l = 0; l < left_list->count; l++) {
                    for (int r = 0; r < right_list->count; r++) {
                        double lv = left_list->nodes[l].value;
                        double rv = right_list->nodes[r].value;

                        /* Addition */
                        Node n_add;
                        n_add.value = lv + rv;
                        n_add.op = '+';
                        n_add.left_idx = l;
                        n_add.right_idx = r;
                        n_add.split = k;
                        add_node(&g_dp[i][j], n_add);

                        /* Subtraction */
                        Node n_sub;
                        n_sub.value = lv - rv;
                        n_sub.op = '-';
                        n_sub.left_idx = l;
                        n_sub.right_idx = r;
                        n_sub.split = k;
                        add_node(&g_dp[i][j], n_sub);

                        /* Multiplication */
                        Node n_mul;
                        n_mul.value = lv * rv;
                        n_mul.op = '*';
                        n_mul.left_idx = l;
                        n_mul.right_idx = r;
                        n_mul.split = k;
                        add_node(&g_dp[i][j], n_mul);

                        /* Division (avoid division by zero) */
                        if (fabs(rv) > EPSILON) {
                            Node n_div;
                            n_div.value = lv / rv;
                            n_div.op = '/';
                            n_div.left_idx = l;
                            n_div.right_idx = r;
                            n_div.split = k;
                            add_node(&g_dp[i][j], n_div);
                        }
                    }
                }
            }
        }
    }

    /* 3. Convert DP results to strings and deduplicate */
    NodeList *final_nodes = &g_dp[0][g_seq_len - 1];
    Solution *all_sols = (Solution *)malloc(final_nodes->count * sizeof(Solution));
    if (!all_sols && final_nodes->count > 0) return 1;

    char buf[512];
    for (int i = 0; i < final_nodes->count; i++) {
        build_string(0, g_seq_len - 1, i, buf);
        all_sols[i].value = final_nodes->nodes[i].value;
        all_sols[i].str = (char *)malloc(strlen(buf) + 1);
        strcpy(all_sols[i].str, buf);
    }

    /* Sort solutions to group duplicates */
    qsort(all_sols, final_nodes->count, sizeof(Solution), cmp_solutions);

    int unique_count = 0;
    if (final_nodes->count > 0) {
        int write_idx = 0;
        for (int i = 1; i < final_nodes->count; i++) {
            if (fabs(all_sols[i].value - all_sols[write_idx].value) < EPSILON &&
                strcmp(all_sols[i].str, all_sols[write_idx].str) == 0) {
                free(all_sols[i].str);
            } else {
                write_idx++;
                all_sols[write_idx] = all_sols[i];
            }
        }
        unique_count = write_idx + 1;
    }

    /* 4. Process queries */
    printf("Problems:\n");
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;

        char type = *p;
        if (type != '?' && type != '#') {
            printf("Invalid input.\n");
            break;
        }
        p++;

        while (*p && isspace((unsigned char)*p)) p++;

        char *endptr;
        long req_val_l = strtol(p, &endptr, 10);
        if (p == endptr) {
            printf("Invalid input.\n");
            break;
        }

        while (*endptr && isspace((unsigned char)*endptr)) endptr++;
        if (*endptr != '\0') {
            printf("Invalid input.\n");
            break;
        }

        double target = (double)req_val_l;
        int match_start = -1;
        int match_count = 0;

        for (int i = 0; i < unique_count; i++) {
            if (fabs(all_sols[i].value - target) < EPSILON) {
                if (match_start == -1) match_start = i;
                match_count++;
            } else if (match_start != -1) {
                break;
            }
        }

        if (type == '?') {
            for (int i = 0; i < match_count; i++) {
                printf("= %s\n", all_sols[match_start + i].str);
            }
        }
        printf("Total: %d\n", match_count);
    }

    /* Cleanup allocated memory */
    for (int i = 0; i < unique_count; i++) {
        free(all_sols[i].str);
    }
    free(all_sols);

    for (int i = 0; i < g_seq_len; i++) {
        for (int j = 0; j < g_seq_len; j++) {
            free(g_dp[i][j].nodes);
        }
    }

    return 0;
}

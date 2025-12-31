#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* -------------------- CONSTANTS -------------------- */

/* Size of the hash table (large prime for good distribution) */
#define HASH_SIZE 10000019

/* Maximum number of results printed per search */
#define MAX_RESULTS 50

/* Maximum substring length stored in the hash index */
#define MAX_INDEXED_LENGTH 18

/* -------------------- DATA STRUCTURES -------------------- */

/* Represents one DNA specimen */
typedef struct Specimen {
    char *sequence;     /* DNA sequence */
    double priority;    /* Priority value (higher = more important) */
    int position;       /* Insertion order */
} Specimen;

/* Linked list node storing specimen index */
typedef struct Link {
    int specimen_position;
    struct Link *forward;
} Link;

/* One bucket in the hash table */
typedef struct HashBucket {
    char *pattern;           /* DNA substring */
    Link *specimens;         /* Linked list of specimens containing it */
    struct HashBucket *forward; /* Collision chain */
} HashBucket;

/* Dynamic list of specimens */
typedef struct {
    Specimen *entries;
    size_t total;
    size_t limit;
} SpecimenList;

/* Hash index structure */
typedef struct {
    HashBucket **buckets;
} HashIndex;

/* -------------------- ERROR & MEMORY HANDLING -------------------- */

/* Print error and exit */
static void terminate(void) {
    printf("Invalid input.\n");
    exit(0);
}

/* malloc with failure check */
static void* allocate(size_t amount) {
    void *ptr = malloc(amount);
    if (!ptr) exit(1);
    return ptr;
}

/* realloc with failure check */
static void* resize(void *ptr, size_t amount) {
    void *newptr = realloc(ptr, amount);
    if (!newptr) exit(1);
    return newptr;
}

/* -------------------- HASHING -------------------- */

/* djb2 hash function */
static unsigned long compute_hash(const char *str) {
    unsigned long result = 5381;
    unsigned char c;
    while ((c = *str++))
        result = (result << 5) + result + c;
    return result % HASH_SIZE;
}

/* Create and initialize hash index */
static HashIndex* create_index(void) {
    HashIndex *index = (HashIndex*)allocate(sizeof(HashIndex));
    index->buckets = (HashBucket**)allocate(sizeof(HashBucket*) * HASH_SIZE);
    for (int i = 0; i < HASH_SIZE; i++)
        index->buckets[i] = NULL;
    return index;
}

/* Insert a substring → specimen mapping into hash index */
static void insert_into_index(HashIndex *index, const char *pattern, int specimen_position) {
    unsigned long slot = compute_hash(pattern);
    HashBucket *bucket = index->buckets[slot];

    /* Look for existing pattern */
    while (bucket) {
        if (strcmp(bucket->pattern, pattern) == 0) {
            Link *link = (Link*)allocate(sizeof(Link));
            link->specimen_position = specimen_position;
            link->forward = bucket->specimens;
            bucket->specimens = link;
            return;
        }
        bucket = bucket->forward;
    }

    /* Create new bucket */
    bucket = (HashBucket*)allocate(sizeof(HashBucket));
    bucket->pattern = (char*)allocate(strlen(pattern) + 1);
    strcpy(bucket->pattern, pattern);

    Link *link = (Link*)allocate(sizeof(Link));
    link->specimen_position = specimen_position;
    link->forward = NULL;
    bucket->specimens = link;

    bucket->forward = index->buckets[slot];
    index->buckets[slot] = bucket;
}

/* Retrieve specimen list for a given substring */
static Link* retrieve_from_index(HashIndex *index, const char *pattern) {
    unsigned long slot = compute_hash(pattern);
    HashBucket *bucket = index->buckets[slot];

    while (bucket) {
        if (strcmp(bucket->pattern, pattern) == 0)
            return bucket->specimens;
        bucket = bucket->forward;
    }
    return NULL;
}

/* Free entire hash index */
static void destroy_index(HashIndex *index) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashBucket *bucket = index->buckets[i];
        while (bucket) {
            HashBucket *old_bucket = bucket;
            bucket = bucket->forward;

            Link *link = old_bucket->specimens;
            while (link) {
                Link *old_link = link;
                link = link->forward;
                free(old_link);
            }

            free(old_bucket->pattern);
            free(old_bucket);
        }
    }
    free(index->buckets);
    free(index);
}

/* -------------------- DNA VALIDATION -------------------- */

/* Check if character is a valid DNA base */
static int is_valid_base(char base) {
    return base == 'A' || base == 'T' || base == 'C' || base == 'G';
}

/* Validate DNA sequence:
   - only A/T/C/G
   - length > 0
   - length divisible by 3 */
static int verify_sequence(const char *sequence) {
    int length = 0;
    while (sequence[length]) {
        if (!is_valid_base(sequence[length]))
            return 0;
        length++;
    }
    return length > 0 && length % 3 == 0;
}

/* -------------------- INPUT HANDLING -------------------- */

/* Read one full line from stdin */
static char* fetch_line(void) {
    size_t limit = 128;
    size_t length = 0;
    char *buffer = (char*)allocate(limit);

    int ch;
    while ((ch = fgetc(stdin)) != EOF && ch != '\n') {
        if (length + 1 >= limit) {
            limit *= 2;
            buffer = (char*)resize(buffer, limit);
        }
        buffer[length++] = (char)ch;
    }

    buffer[length] = '\0';

    if (length == 0 && ch == EOF) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

/* -------------------- SPECIMEN STORAGE -------------------- */

/* Append specimen to dynamic list */
static void append_specimen(SpecimenList *list, char *sequence, double priority, int position) {
    if (list->total >= list->limit) {
        list->limit = list->limit ? list->limit * 2 : 16;
        list->entries = (Specimen*)resize(list->entries, list->limit * sizeof(Specimen));
    }
    list->entries[list->total].sequence = sequence;
    list->entries[list->total].priority = priority;
    list->entries[list->total].position = position;
    list->total++;
}

/* Sort specimens by priority (desc), then insertion order */
static int compare_specimens(const void *first, const void *second) {
    const Specimen *spec_first = (const Specimen*)first;
    const Specimen *spec_second = (const Specimen*)second;
    if (spec_first->priority > spec_second->priority) return -1;
    if (spec_first->priority < spec_second->priority) return 1;
    return spec_first->position - spec_second->position;
}

/* -------------------- SEARCH HELPERS -------------------- */

/* Check if needle exists at triplet-aligned positions */
static int contains_at_triplet(const char *haystack, const char *needle) {
    int hay_len = strlen(haystack);
    int needle_len = strlen(needle);

    for (int offset = 0; offset + needle_len <= hay_len; offset += 3) {
        if (memcmp(haystack + offset, needle, needle_len) == 0)
            return 1;
    }
    return 0;
}

/* Build hash index for one specimen */
static void build_index_for_specimen(HashIndex *index, const char *sequence, int specimen_position) {
    int length = strlen(sequence);
    int max_len = length < MAX_INDEXED_LENGTH ? length : MAX_INDEXED_LENGTH;

    char *substring = (char*)allocate(MAX_INDEXED_LENGTH + 1);

    for (int substring_length = 3; substring_length <= max_len; substring_length += 3) {
        for (int offset = 0; offset + substring_length <= length; offset += 3) {
            memcpy(substring, sequence + offset, substring_length);
            substring[substring_length] = '\0';
            insert_into_index(index, substring, specimen_position);
        }
    }

    free(substring);
}

/* -------------------- MAIN PROGRAM -------------------- */

int main(void) {
    printf("DNA database:\n");

    SpecimenList specimens = {NULL, 0, 0};
    HashIndex *index = create_index();
    int specimen_counter = 0;

    /* Read database entries */
    while (1) {
        char *input_line = fetch_line();

        if (!input_line || strlen(input_line) == 0) {
            if (input_line) free(input_line);
            break;
        }

        char *separator = strchr(input_line, ':');
        if (!separator) {
            free(input_line);
            terminate();
        }

        *separator = '\0';
        char *priority_text = input_line;
        char *sequence = separator + 1;

        char *end_ptr;
        double priority = strtod(priority_text, &end_ptr);
        if (*end_ptr != '\0' || end_ptr == priority_text || priority <= 0) {
            free(input_line);
            terminate();
        }

        if (!verify_sequence(sequence)) {
            free(input_line);
            terminate();
        }

        char *sequence_copy = (char*)allocate(strlen(sequence) + 1);
        strcpy(sequence_copy, sequence);

        append_specimen(&specimens, sequence_copy, priority, specimen_counter);
        build_index_for_specimen(index, sequence_copy, specimen_counter);

        specimen_counter++;
        free(input_line);
    }

    if (specimen_counter == 0)
        terminate();

    printf("Searches:\n");

    /* Process search queries */
    while (1) {
        char *search_query = fetch_line();
        if (!search_query)
            break;

        if (!verify_sequence(search_query)) {
            free(search_query);
            terminate();
        }

        int query_len = strlen(search_query);
        Link *found_matches = NULL;
        int use_fallback = 0;

        /* Use hash index only for short queries */
        if (query_len <= MAX_INDEXED_LENGTH) {
            found_matches = retrieve_from_index(index, search_query);
        } else {
            use_fallback = 1;
        }

        int *visited = (int*)allocate(specimen_counter * sizeof(int));
        memset(visited, 0, specimen_counter * sizeof(int));

        Specimen *outcomes = (Specimen*)allocate(specimen_counter * sizeof(Specimen));
        int outcome_count = 0;

        if (use_fallback) {
            /* Slow full scan */
            for (int i = 0; i < specimen_counter; i++) {
                if (contains_at_triplet(specimens.entries[i].sequence, search_query)) {
                    outcomes[outcome_count++] = specimens.entries[i];
                }
            }
        } else {
            /* Fast indexed lookup */
            Link *current_link = found_matches;
            while (current_link) {
                int pos = current_link->specimen_position;
                if (!visited[pos]) {
                    visited[pos] = 1;
                    outcomes[outcome_count++] = specimens.entries[pos];
                }
                current_link = current_link->forward;
            }
        }

        qsort(outcomes, outcome_count, sizeof(Specimen), compare_specimens);

        printf("Found: %d\n", outcome_count);

        int display_limit = outcome_count < MAX_RESULTS ? outcome_count : MAX_RESULTS;
        for (int i = 0; i < display_limit; i++) {
            printf("> %s\n", outcomes[i].sequence);
        }

        free(visited);
        free(outcomes);
        free(search_query);
    }

    /* Cleanup */
    for (size_t i = 0; i < specimens.total; i++)
        free(specimens.entries[i].sequence);
    free(specimens.entries);
    destroy_index(index);

    return 0;
}

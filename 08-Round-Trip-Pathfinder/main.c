#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Maximum length for city names */
constexpr size_t CITY_NAME_MAX = 100;

/*
 * Trip node structure (linked list)
 * m_Next   -> pointer to next trip in result list
 * m_Desc   -> textual description of the trip
 * m_Cities -> number of cities visited (edges count)
 * m_Cost   -> total cost of the trip
 */
typedef struct TTrip
{
  struct TTrip * m_Next;
  char         * m_Desc;
  int            m_Cities;
  int            m_Cost;
} TTRIP;

/*
 * Creates a new trip node
 */
TTRIP * makeTrip ( char    desc[],
                   int     cities,
                   int     cost,
                   TTRIP * next )
{
  TTRIP * res = (TTRIP *) malloc ( sizeof ( *res ) );
  res -> m_Next = next;
  res -> m_Desc = desc;
  res -> m_Cities = cities;
  res -> m_Cost = cost;
  return res;
}

/*
 * Frees a linked list of trips including descriptions
 */
void freeTripList ( TTRIP * x )
{
  while ( x )
  {
    TTRIP * tmp = x -> m_Next;
    free ( x -> m_Desc );
    free ( x );
    x = tmp;
  }
}
#endif /* __PROGTEST__ */

#define _INIT_CAPACITY 64
#define _HASH_SEED 5381

/*
 * Adjacency list node (edge)
 * _target_idx -> index of destination city
 * _weight     -> travel cost
 * _next       -> next edge in adjacency list
 */
typedef struct _adj_node_s {
    int _target_idx;
    int _weight;
    struct _adj_node_s* _next;
} _adj_node_t;

/*
 * Vertex (city) representation
 * _name     -> city name
 * _name_len -> length of city name
 * _head     -> adjacency list head
 */
typedef struct _vertex_s {
    char* _name;
    size_t _name_len;
    _adj_node_t* _head;
} _vertex_t;

/*
 * Graph context holding all vertices and hash buckets
 */
typedef struct _graph_ctx_s {
    _vertex_t* _vertices;
    int _v_count;
    int _v_cap;
    
    int* _buckets;
    int _bucket_cnt;
} _graph_ctx_t;

/*
 * Dynamic array for storing result trips
 */
typedef struct _res_arr_s {
    TTRIP** _data;
    int _count;
    int _capacity;
} _res_arr_t;

/*
 * djb2 hash function for city names
 */
static unsigned long _djb2_hash(const char *str) {
    unsigned long hash = _HASH_SEED;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

/*
 * Initializes the graph context
 */
static void _init_graph(_graph_ctx_t* _g) {
    _g->_v_count = 0;
    _g->_v_cap = _INIT_CAPACITY;
    _g->_vertices = (_vertex_t*)malloc(sizeof(_vertex_t) * _g->_v_cap);
    
    _g->_bucket_cnt = 4099;
    _g->_buckets = (int*)malloc(sizeof(int) * _g->_bucket_cnt);
    for(int i = 0; i < _g->_bucket_cnt; i++) _g->_buckets[i] = -1;
}

/*
 * Frees all graph memory
 */
static void _free_graph(_graph_ctx_t* _g) {
    for (int i = 0; i < _g->_v_count; i++) {
        _adj_node_t* _curr = _g->_vertices[i]._head;
        while (_curr) {
            _adj_node_t* _tmp = _curr;
            _curr = _curr->_next;
            free(_tmp);
        }
        free(_g->_vertices[i]._name);
    }
    free(_g->_vertices);
    free(_g->_buckets);
}

/*
 * Returns city ID, creates city if it does not exist
 */
static int _get_city_id(_graph_ctx_t* _g, const char* _name) {
    unsigned long _h = _djb2_hash(_name);
    int _idx = (int)(_h % _g->_bucket_cnt);

    int _candidate = _g->_buckets[_idx];
    if (_candidate != -1 && strcmp(_g->_vertices[_candidate]._name, _name) == 0) {
        return _candidate;
    }
    
    for (int i = 0; i < _g->_v_count; i++) {
        if (strcmp(_g->_vertices[i]._name, _name) == 0) return i;
    }

    if (_g->_v_count == _g->_v_cap) {
        _g->_v_cap *= 2;
        _g->_vertices = (_vertex_t*)realloc(_g->_vertices, sizeof(_vertex_t) * _g->_v_cap);
    }
    
    int _new_id = _g->_v_count++;
    _g->_vertices[_new_id]._name = strdup(_name);
    _g->_vertices[_new_id]._name_len = strlen(_name);
    _g->_vertices[_new_id]._head = NULL;
    
    if (_g->_buckets[_idx] == -1) {
        _g->_buckets[_idx] = _new_id;
    }
    
    return _new_id;
}

/*
 * Adds a directed edge to the graph
 */
static void _add_edge(_graph_ctx_t* _g, int _u, int _v, int _cost) {
    _adj_node_t* _node = (_adj_node_t*)malloc(sizeof(_adj_node_t));
    _node->_target_idx = _v;
    _node->_weight = _cost;
    _node->_next = _g->_vertices[_u]._head;
    _g->_vertices[_u]._head = _node;
}

/*
 * Parses input data and builds the graph
 */
static int _parse_input(_graph_ctx_t* _g, const char* _data) {
    const char* _ptr = _data;
    char _buf1[CITY_NAME_MAX + 1];
    char _buf2[CITY_NAME_MAX + 1];
    int _cost;
    
    while (*_ptr) {
        while (*_ptr && (*_ptr == ' ' || *_ptr == '\n' || *_ptr == '\t')) _ptr++;
        if (!*_ptr) break;

        if (sscanf(_ptr, "%d", &_cost) != 1) break;
        while (*_ptr && (*_ptr >= '0' && *_ptr <= '9')) _ptr++;
        while (*_ptr && (*_ptr == ' ' || *_ptr == '\t')) _ptr++;

        if (*_ptr != ':') break;
        _ptr++;

        while (*_ptr && (*_ptr == ' ' || *_ptr == '\t')) _ptr++;

        int _len = 0;
        while (*_ptr && *_ptr != ' ' && *_ptr != '-' && *_ptr != '\n' && *_ptr != '\t') {
            if (_len < (int)CITY_NAME_MAX) _buf1[_len++] = *_ptr;
            _ptr++;
        }
        _buf1[_len] = '\0';

        while (*_ptr && (*_ptr == ' ' || *_ptr == '\t')) _ptr++;

        if (*_ptr == '-' && *(_ptr+1) == '>') {
            _ptr += 2;
        } else {
            return 0;
        }
        
        while (*_ptr && (*_ptr == ' ' || *_ptr == '\t')) _ptr++;

        _len = 0;
        while (*_ptr && *_ptr != ' ' && *_ptr != '\n' && *_ptr != '\t') {
             if (_len < (int)CITY_NAME_MAX) _buf2[_len++] = *_ptr;
            _ptr++;
        }
        _buf2[_len] = '\0';
        
        int _u = _get_city_id(_g, _buf1);
        int _v = _get_city_id(_g, _buf2);
        _add_edge(_g, _u, _v, _cost);
    }
    return 1;
}

/*
 * Initializes result array
 */
static void _res_init(_res_arr_t* _arr) {
    _arr->_count = 0;
    _arr->_capacity = 16;
    _arr->_data = (TTRIP**)malloc(sizeof(TTRIP*) * _arr->_capacity);
}

/*
 * Adds a trip to result array
 */
static void _res_add(_res_arr_t* _arr, TTRIP* _t) {
    if (_arr->_count == _arr->_capacity) {
        _arr->_capacity *= 2;
        _arr->_data = (TTRIP**)realloc(_arr->_data, sizeof(TTRIP*) * _arr->_capacity);
    }
    _arr->_data[_arr->_count++] = _t;
}

/*
 * Comparator for sorting trips by cost
 */
static int _cmp_trips(const void* _a, const void* _b) {
    TTRIP* _ta = *(TTRIP**)_a;
    TTRIP* _tb = *(TTRIP**)_b;
    if (_ta->m_Cost < _tb->m_Cost) return -1;
    if (_ta->m_Cost > _tb->m_Cost) return 1;
    return 0;
}

/*
 * Depth-first search for finding cycles
 */
static void _dfs_find(_graph_ctx_t* _g, int _curr_id, int _start_id, 
                      int _curr_cost, int _max_cost, 
                      int* _path, int _depth, 
                      char* _visited, _res_arr_t* _results) {
    
    if (_curr_cost > _max_cost) return;

    if (_curr_id == _start_id && _depth > 0) {

        int _total_len = 0;
        for (int i = 0; i < _depth; i++) {
            _total_len += (int)_g->_vertices[_path[i]]._name_len;
            _total_len += 4;
        }
        _total_len += (int)_g->_vertices[_start_id]._name_len;
        _total_len += 1;

        char* _desc_str = (char*)malloc(_total_len);
        char* _wr_ptr = _desc_str;
        
        for (int i = 0; i < _depth; i++) {
            const char* _nm = _g->_vertices[_path[i]]._name;
            int _nl = (int)_g->_vertices[_path[i]]._name_len;
            memcpy(_wr_ptr, _nm, _nl);
            _wr_ptr += _nl;
            memcpy(_wr_ptr, " -> ", 4);
            _wr_ptr += 4;
        }

        const char* _nm = _g->_vertices[_start_id]._name;
        int _nl = (int)_g->_vertices[_start_id]._name_len;
        memcpy(_wr_ptr, _nm, _nl);
        _wr_ptr += _nl;
        *_wr_ptr = '\0';

        TTRIP* _new_trip = makeTrip(_desc_str, _depth, _curr_cost, NULL);
        _res_add(_results, _new_trip);
        return;
    }

    _visited[_curr_id] = 1;
    _path[_depth] = _curr_id;

    _adj_node_t* _e = _g->_vertices[_curr_id]._head;
    while (_e) {
        int _nxt = _e->_target_idx;
        int _new_cost = _curr_cost + _e->_weight;

        if (_new_cost <= _max_cost) {
            if (_nxt == _start_id) {
                if (_depth >= 1) {
                     _dfs_find(_g, _nxt, _start_id, _new_cost, _max_cost, _path, _depth + 1, _visited, _results);
                }
            } else if (!_visited[_nxt]) {
                _dfs_find(_g, _nxt, _start_id, _new_cost, _max_cost, _path, _depth + 1, _visited, _results);
            }
        }
        _e = _e->_next;
    }

    _visited[_curr_id] = 0;
}

/*
 * Main API function
 */
TTRIP * findTrips ( const char data[], const char from[], int costMax )
{
    if (!data || !from || costMax <= 0) return NULL;

    _graph_ctx_t _g_ctx;
    _init_graph(&_g_ctx);

    if (!_parse_input(&_g_ctx, data)) {
        _free_graph(&_g_ctx);
        return NULL;
    }

    int _start_id = -1;
    for (int i = 0; i < _g_ctx._v_count; i++) {
        if (strcmp(_g_ctx._vertices[i]._name, from) == 0) {
            _start_id = i;
            break;
        }
    }

    if (_start_id == -1) {
        _free_graph(&_g_ctx);
        return NULL;
    }

    _res_arr_t _results;
    _res_init(&_results);

    char* _visited = (char*)calloc(_g_ctx._v_count, sizeof(char));
    int* _path = (int*)malloc(sizeof(int) * (_g_ctx._v_count + 100));

    _dfs_find(&_g_ctx, _start_id, _start_id, 0, costMax, _path, 0, _visited, &_results);

    if (_results._count > 0) {
        qsort(_results._data, _results._count, sizeof(TTRIP*), _cmp_trips);
    }

    TTRIP* _head = NULL;
    if (_results._count > 0) {
        _head = _results._data[0];
        for (int i = 0; i < _results._count - 1; i++) {
            _results._data[i]->m_Next = _results._data[i+1];
        }
        _results._data[_results._count - 1]->m_Next = NULL;
    }

    free(_visited);
    free(_path);
    free(_results._data);
    _free_graph(&_g_ctx);

    return _head;
}

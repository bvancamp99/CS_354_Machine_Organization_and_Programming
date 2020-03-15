/* Name: Bryce Van Camp
 * CS login: bvan-camp
 * Section(s): LEC 002
 *
 * csim.c - A cache simulator that can replay traces from Valgrind
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is LRU.
 *
 * Implementation and assumptions:
 *  1. Each load/store can cause at most one cache miss plus a possible
 * eviction.
 *  2. Instruction loads (I) are ignored.
 *  3. Data modify (M) is treated as a load followed by a store to the same
 *  address. Hence, an M operation can result in two cache hits, or a miss and a
 *  hit plus a possible eviction.
 *
 * The function print_summary() is given to print output.
 * Please use this function to print the number of hits, misses and evictions.
 * This is crucial for the driver to evaluate your work.
 */

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/****************************************************************************/
/***** DO NOT MODIFY THESE VARIABLE NAMES ***********************************/

/* Globals set by command line args */
int s = 0;         /* set index bits */
int E = 0;         /* associativity */
int b = 0;         /* block offset bits */
int verbosity = 0; /* print trace if set */
char* trace_file = NULL;

/* Derived from command line args */
int B; /* block size (bytes) B = 2^b */
int S; /* number of sets S = 2^s In C, you can use the left shift operator */

/* Counters used to record cache statistics */
int hit_cnt = 0;
int miss_cnt = 0;
int evict_cnt = 0;
/*****************************************************************************/

// initialize num tag bits
int t = 0;

/* Type: Memory address
 * Use this type whenever dealing with addresses or address masks
 */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
 * Use this type for each line of the cache
 */
typedef struct cache_line {
  char valid;
  mem_addr_t tag;
  struct cache_line* next;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

/* The cache we are simulating */
cache_t cache;

/*
 * init_cache -
 * Allocate data structures to hold info regrading the sets and cache lines
 * use struct "cache_line_t" here
 * Initialize valid and tag field with 0s.
 * use S (= 2^s) and E while allocating the data structures here
 */
void init_cache() {
  // set values for num sets, num bytes/block, and tag bits
  S = 2 << s;
  B = 2 << b;
  t = (sizeof(mem_addr_t) * 8) - s - b;

  // allocate cache with num sets S
  cache = malloc(sizeof(cache_set_t) * S);
  if (cache == NULL) {
    printf("error allocating memory to cache\n");
    exit(1);
  }
  // allocate each set of the cache with num lines E
  for (int i = 0; i < S; i++) {
    // allocate head node of set i and set fields to 0
    cache[i] = malloc(sizeof(cache_line_t));
    if (cache[i] == NULL) {
      printf("error allocating first line of set #%i in the cache", i);
      exit(1);
    }
    cache[i]->valid = 0;
    cache[i]->tag = 0;

    // allocate next nodes of set i and set fields to 0
    cache_line_t* curNode = cache[i];
    for (int j = 1; j < E; j++) {
      curNode->next = malloc(sizeof(cache_line_t));
      if (curNode->next == NULL) {
        printf("error allocating line %i of set #%i in the cache", j, i);
      }
      curNode->next->valid = 0;
      curNode->next->tag = 0;
      curNode = curNode->next;
    }
    // set tail node's next to NULL to avoid undefined behavior
    curNode->next = NULL;
  }
}

/* free_cache - free each piece of memory you allocated using malloc
 * inside init_cache() function
 */
void free_cache() {
  // free each set of the cache first
  for (int i = 0; i < S; i++) {
    cache_line_t* curNode = cache[i];
    // free all but the last node of the linked list
    while (curNode->next != NULL) {
      cache_line_t* nodeToDelete = curNode;
      curNode = curNode->next;
      free(nodeToDelete);
    }
    // free the last node in the list
    free(curNode);
  }
  // free the cache and set to NULL
  free(cache);
  cache = NULL;
}

/*
 * This function brings the node accessed by access_data to the front,
 * which makes it the head of the linked list.
 * param - curSet     this is the current set that was extracted from addr in
 * access_data
 * param - parentNode this is the parent of the node that we will make the head
 * of the list
 */
void bringNodeToFront(mem_addr_t curSet, cache_line_t* parentNode) {
  // set value of node that we will move
  cache_line_t* node = parentNode->next;
  // update parent node's child
  parentNode->next = node->next;
  // make node point to the old head
  node->next = cache[curSet];
  // move node to the front of the list
  cache[curSet] = node;
}

/*
 *   access_data - Access data at memory address addr.
 *   If it is already in cache, increase hit_cnt
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase evict_cnt if a line is evicted.
 *   you will manipulate data structures allocated in init_cache() here
 */
void access_data(mem_addr_t addr) {
  // initialize variables regarding the current set and tag value of param addr
  mem_addr_t curSet = addr << t;
  curSet >>= (t + b);
  mem_addr_t tagID = addr >> (s + b);

  // check if direct-mapped cache
  if (E == 1) {
    // if v-bit is 1, there is an item already in the block
    if (cache[curSet]->valid) {
      // if tags match, it's a cache hit; increment hit_cnt
      if (tagID == cache[curSet]->tag) {
        hit_cnt++;
      } else {  // else there was a conflict miss
        // set tag id and v-bit
        cache[curSet]->tag = tagID;
        cache[curSet]->valid = 1;

        // increment miss_cnt and evict_cnt
        miss_cnt++;
        evict_cnt++;
      }
    } else {  // else v-bit is 0, i.e. no item in the block; this is a cold miss
      // set tag id and v-bit
      cache[curSet]->tag = tagID;
      cache[curSet]->valid = 1;

      // increment miss_cnt
      miss_cnt++;
    }
  } else {  // else not a direct-mapped cache, i.e. lines/set > 1
    // set line ptrs that refer to the parent of the current line, along with
    // the parent's parent
    cache_line_t* parentLine = NULL;
    cache_line_t* parentOfParentLine = NULL;

    // go through curSet until parentLine is the parent of the tail node
    for (parentLine = cache[curSet]; parentLine->next->next != NULL;
         parentLine = parentLine->next) {
      // if tags match and v-bit is 1, increment hit_cnt
      if (tagID == parentLine->tag && parentLine->valid) {
        hit_cnt++;
        // if parentLine isn't the head already, bring it to the front of the
        // list
        if (parentLine != cache[curSet]) {
          bringNodeToFront(curSet, parentOfParentLine);
        }
        return;
      }

      parentOfParentLine = parentLine;
    }

    // check parent of the tail node
    if (tagID == parentLine->tag && parentLine->valid) {
      hit_cnt++;
      // if parentLine isn't the head already, bring it to the front of the list
      if (parentLine != cache[curSet]) {
        bringNodeToFront(curSet, parentOfParentLine);
      }
      return;
    }
    // check tail node
    if (tagID == parentLine->next->tag && parentLine->next->valid) {
      hit_cnt++;
      // bring node to the front of the list
      bringNodeToFront(curSet, parentLine);
      return;
    }

    // increment evict_cnt if tags don't match but v-bit is 1
    if (parentLine->next->valid) {
      evict_cnt++;
    }
    // increment miss_cnt
    miss_cnt++;
    // replace tail node with the new head node
    parentLine->next->tag = tagID;
    parentLine->next->valid = 1;
    // set new head node's next to the old head node
    parentLine->next->next = cache[curSet];
    // have the set point to the new head node
    cache[curSet] = parentLine->next;
    parentLine->next = NULL;
  }
}

/*
 * replay_trace - replays the given trace file against the cache
 * reads the input trace file line by line
 * extracts the type of each memory access : L/S/M
 * YOU MUST TRANSLATE one "L" as a load i.e. 1 memory access
 * YOU MUST TRANSLATE one "S" as a store i.e. 1 memory access
 * YOU MUST TRANSLATE one "M" as a load followed by a store i.e. 2 memory
 * accesses
 */
void replay_trace(char* trace_fn) {
  char buf[1000];
  mem_addr_t addr = 0;
  unsigned int len = 0;
  FILE* trace_fp = fopen(trace_fn, "r");

  if (!trace_fp) {
    fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
    exit(1);
  }

  while (fgets(buf, 1000, trace_fp) != NULL) {
    if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M') {
      sscanf(buf + 3, "%llx,%u", &addr, &len);

      if (verbosity) printf("%c %llx,%u ", buf[1], addr, len);

      // now you have:
      // 1. address accessed in variable - addr
      // 2. type of acccess(S/L/M)  in variable - buf[1]
      // call access_data function here depending on type of access
      access_data(addr);
      if (buf[1] == 'M') {
        access_data(addr);
      }

      if (verbosity) printf("\n");
    }
  }

  fclose(trace_fp);
}

/*
 * print_usage - Print usage info
 */
void print_usage(char* argv[]) {
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n");
  printf("\nExamples:\n");
  printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
  printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
  exit(0);
}

/*
 * print_summary - Summarize the cache simulation statistics. Student cache
 * simulators
 *                must call this function in order to be properly autograded.
 */
void print_summary(int hits, int misses, int evictions) {
  printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
  FILE* output_fp = fopen(".csim_results", "w");
  assert(output_fp);
  fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
  fclose(output_fp);
}

/*
 * main - Main routine
 */
int main(int argc, char* argv[]) {
  char c;

  // Parse the command line arguments: -h, -v, -s, -E, -b, -t
  while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
    switch (c) {
      case 'b':
        b = atoi(optarg);
        break;
      case 'E':
        E = atoi(optarg);
        break;
      case 'h':
        print_usage(argv);
        exit(0);
      case 's':
        s = atoi(optarg);
        break;
      case 't':
        trace_file = optarg;
        break;
      case 'v':
        verbosity = 1;
        break;
      default:
        print_usage(argv);
        exit(1);
    }
  }

  /* Make sure that all required command line args were specified */
  if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
    printf("%s: Missing required command line argument\n", argv[0]);
    print_usage(argv);
    exit(1);
  }

  /* Initialize cache */
  init_cache();

  replay_trace(trace_file);

  /* Free allocated memory */
  free_cache();

  /* Output the hit and miss statistics for the autograder */
  print_summary(hit_cnt, miss_cnt, evict_cnt);
  return 0;
}

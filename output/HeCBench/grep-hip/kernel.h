#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif

// --- from main.cu ---
/*
 * Regular expression implementation.
 * Supports only ( | ) * + ?.  No escapes.
 * Compiles to NFA and then simulates NFA
 * using Thompson's algorithm.
 *
 * See also http://swtch.com/~rsc/regexp/ and
 * Thompson, Ken.  Regular Expression Search Algorithm,
 * Communications of the ACM 11(6) (June 1968), pp. 419-422.
 * 
 * Copyright (c) 2007 Russ Cox.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <hip/hip_runtime.h>





// --- from nfautil.cu ---
#include <hip/hip_runtime.h>

/*
 * Visualize the NFA in stdout
 */
int visited[5000];
int count[5000];
int visited_index = 0;

int nstate;
State matchstate = { Match };  /* matching state */

List l1, l2;
static int listid;

void addstate(List*, State*);

/* Check whether state list contains a match. */

/* Add s to l, following unlabeled arrows. */

/*
 * Step the NFA from the states in clist
 * past the character c,
 * to create next NFA state set nlist.
 */

/* Run NFA to determine whether it matches s. */

/* Check for a string match at all possible start positions */

/* Allocate and initialize State */
  State*
state(int c, State *out, State *out1)
{
  State *s;

  s = (State *) malloc(sizeof *s);
  s->id = ++nstate;
  s->lastlist = 0;
  s->c = c;
  s->out = out;
  s->out1 = out1;

  // device pointer of itself
  // serves no real purpose other than to help transfer the NFA over
  s->dev = NULL;

  s->free = STATE_INIT;
  return s;
}

/* Initialize Frag struct. */
  Frag
frag(State *start, Ptrlist *out)
{
  Frag n = { start, out };
  return n;
}

/* Create singleton list containing just outp. */
  Ptrlist*
list1(State **outp)
{
  Ptrlist *l;

  l = (Ptrlist*)outp;
  l->next = NULL;
  return l;
}

/* Patch the list of states at out to point to start. */

/* Join the two lists l1 and l2, returning the combination. */
  Ptrlist*
append(Ptrlist *l1, Ptrlist *l2)
{
  Ptrlist *oldl1;

  oldl1 = l1;
  while(l1->next)
    l1 = l1->next;
  l1->next = l2;
  return oldl1;
}

/*
 * Convert postfix regular expression to NFA.
 * Return start state.
 */
  State*
post2nfa(char *postfix)
{
  char *p;
  Frag stack[1000], *stackp, e1, e2, e;
  State *s;

  // fprintf(stderr, "postfix: %s\n", postfix);

  if(postfix == NULL)
    return NULL;

#define push(s) *stackp++ = s
#define pop() *--stackp

  stackp = stack;

  e = pop();
  if(stackp != stack)
    return NULL;

  patch(e.out, &matchstate);

  return e.start;
#undef pop
#undef push
}

/*
 * Convert infix regexp re to postfix notation.










// --- from pnfa.cu ---

inline void paddstate(List*, State*, List*);

/*
 * Convert infix regexp re to postfix notation.

/* Compute initial state list */
  inline List*
pstartlist(State *start, List *l)
{
  l->n = 0;

  List addStartState;
  paddstate(l, start, &addStartState);
  return l;
}

/* Check whether state list contains a match. */

/* Add s to l, following unlabeled arrows. */

/*
 * pstep the NFA from the states in clist
 * past the character c,
 * to create next NFA state set nlist.
 */

/* Run NFA to determine whether it matches s. */

/* Check for a string match at all possible start positions */



// --- from putil.cu ---

inline State* pstate(int , State *, State *, State *, int *);
inline Frag pfrag(State *, Ptrlist *);
inline Ptrlist* plist1(State **);
inline void ppatch(Ptrlist *, State *);
inline Ptrlist* pappend(Ptrlist *, Ptrlist *);

/* Initialize frag struct. */
  inline Frag
pfrag(State *start, Ptrlist *out)
{
  Frag n = { start, out };
  return n;
}

/* Create singleton list containing just outp. */
  inline Ptrlist*
plist1(State **outp)
{
  Ptrlist *l;

  l = (Ptrlist*)outp;
  l->next = NULL;
  return l;
}

/* Patch the list of states at out to point to start. */

/* Join the two lists l1 and l2, returning the combination. */
  inline Ptrlist*
pappend(Ptrlist *l1, Ptrlist *l2)
{
  Ptrlist *oldl1;

  oldl1 = l1;
  while(l1->next)
    l1 = l1->next;
  l1->next = l2;
  return oldl1;
}

/*
 * Convert postfix regular expression to NFA.
 * Return start state.
 */

  inline State*
ppost2nfa(char *postfix, State *lstate, int *pnstate, State *pmatchstate)
{
  char *p;
  Frag stack[1000], *stackp, e1, e2, e;
  State *s;

  // fprintf(stderr, "postfix: %s\n", postfix);

  if(postfix == NULL)
    return NULL;

#define push(s) *stackp++ = s
#define pop() *--stackp

  stackp = stack;

  e = pop();
  if(stackp != stack)
    return NULL;

  //ppatch(e.out, &pmatchstate);
  ppatch(e.out, pmatchstate);

  return e.start;
#undef pop
#undef push
}


// --- from regex.cu ---

#define DEREF(arr,i) ((*(arr))[(i)])

/* constructor for SimpleReBuilder */











// --- from cycleTimer.h ---
#ifndef _SYRAH_CYCLE_TIMER_H_
#define _SYRAH_CYCLE_TIMER_H_

#if defined(__APPLE__)
  #if defined(__x86_64__)
    #include <sys/sysctl.h>
  #else
    #include <mach/mach.h>
    #include <mach/mach_time.h>
  #endif // __x86_64__ or not

  #include <stdio.h>  // fprintf
  #include <stdlib.h> // exit

#elif _WIN32
#  include <windows.h>
#  include <time.h>
#else
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <time.h>
#endif

  // This uses the cycle counter of the processor.  Different
  // processors in the system will have different values for this.  If
  // you process moves across processors, then the delta time you
  // measure will likely be incorrect.  This is mostly for fine
  // grained measurements where the process is likely to be on the
  // same processor.  For more global things you should use the
  // Time interface.

  // Also note that if you processors' speeds change (i.e. processors
  // scaling) or if you are in a heterogenous environment, you will
  // likely get spurious results.
  class CycleTimer {
  public:
    typedef unsigned long long SysClock;

    //////////
    // Return the current CPU time, in terms of clock ticks.
    // Time zero is at some arbitrary point in the past.
    static SysClock currentTicks() {
#if defined(__APPLE__) && !defined(__x86_64__)
      return mach_absolute_time();
#elif defined(_WIN32)
      LARGE_INTEGER qwTime;
      QueryPerformanceCounter(&qwTime);
      return qwTime.QuadPart;
#elif defined(__x86_64__)
      unsigned int a, d;
      asm volatile("rdtsc" : "=a" (a), "=d" (d));
      return static_cast<unsigned long long>(a) |
        (static_cast<unsigned long long>(d) << 32);
#elif defined(__ARM_NEON__) && 0 // mrc requires superuser.
      unsigned int val;
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(val));
      return val;
#else
      timespec spec;
      clock_gettime(CLOCK_THREAD_CPUTIME_ID, &spec);
      return CycleTimer::SysClock(static_cast<float>(spec.tv_sec) * 1e9 + static_cast<float>(spec.tv_nsec));
#endif
    }

    //////////
    // Return the current CPU time, in terms of seconds.
    // This is slower than currentTicks().  Time zero is at
    // some arbitrary point in the past.
    static double currentSeconds() {
      return currentTicks() * secondsPerTick();
    }

    //////////
    // Return the conversion from seconds to ticks.
    static double ticksPerSecond() {
      return 1.0/secondsPerTick();
    }

    static const char* tickUnits() {
#if defined(__APPLE__) && !defined(__x86_64__)
      return "ns";
#elif defined(__WIN32__) || defined(__x86_64__)
      return "cycles";
#else
      return "ns"; // clock_gettime
#endif
    }

    //////////
    // Return the conversion from ticks to seconds.
    static double secondsPerTick() {
      static bool initialized = false;
      static double secondsPerTick_val;
      if (initialized) return secondsPerTick_val;
#if defined(__APPLE__)
  #ifdef __x86_64__
      int args[] = {CTL_HW, HW_CPU_FREQ};
      unsigned int Hz;
      size_t len = sizeof(Hz);
      if (sysctl(args, 2, &Hz, &len, NULL, 0) != 0) {
         fprintf(stderr, "Failed to initialize secondsPerTick_val!\n");
         exit(-1);
      }
      secondsPerTick_val = 1.0 / (double) Hz;
  #else
      mach_timebase_info_data_t time_info;
      mach_timebase_info(&time_info);

      // Scales to nanoseconds without 1e-9f
      secondsPerTick_val = (1e-9*static_cast<double>(time_info.numer))/
        static_cast<double>(time_info.denom);
  #endif // x86_64 or not
#elif defined(_WIN32)
      LARGE_INTEGER qwTicksPerSec;
      QueryPerformanceFrequency(&qwTicksPerSec);
      secondsPerTick_val = 1.0/static_cast<double>(qwTicksPerSec.QuadPart);
#else
      FILE *fp = fopen("/proc/cpuinfo","r");
      char input[1024];
      if (!fp) {
         fprintf(stderr, "CycleTimer::resetScale failed: couldn't find /proc/cpuinfo.");
         exit(-1);
      }
      // In case we don't find it, e.g. on the N900
      secondsPerTick_val = 1e-9;
      while (!feof(fp) && fgets(input, 1024, fp)) {
        // NOTE(boulos): Because reading cpuinfo depends on dynamic
        // frequency scaling it's better to read the @ sign first
        float GHz, MHz;
        if (strstr(input, "model name")) {
          char* at_sign = strstr(input, "@");
          if (at_sign) {
            char* after_at = at_sign + 1;
            char* GHz_str = strstr(after_at, "GHz");
            char* MHz_str = strstr(after_at, "MHz");
            if (GHz_str) {
              *GHz_str = '\0';
              if (1 == sscanf(after_at, "%f", &GHz)) {
                //printf("GHz = %f\n", GHz);
                secondsPerTick_val = 1e-9f / GHz;
                break;
              }
            } else if (MHz_str) {
              *MHz_str = '\0';
              if (1 == sscanf(after_at, "%f", &MHz)) {
                //printf("MHz = %f\n", MHz);
                secondsPerTick_val = 1e-6f / GHz;
                break;
              }
            }
          }
        } else if (1 == sscanf(input, "cpu MHz : %f", &MHz)) {
          //printf("MHz = %f\n", MHz);
          secondsPerTick_val = 1e-6f / MHz;
          break;
        }
      }
      fclose(fp);
#endif

      initialized = true;
      return secondsPerTick_val;
    }

    //////////
    // Return the conversion from ticks to milliseconds.
    static double msPerTick() {
      return secondsPerTick() * 1000.0;
    }

  private:
    CycleTimer();
  };

#endif // #ifndef _SYRAH_CYCLE_TIMER_H_


// --- from nfautil.h ---
#ifndef NFAUTIL_H
#define NFAUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>

#define LINE_SIZE 200

#define ANY 0x15
#define CONCATENATE 0x1b
#define ALTERNATE 0x04
#define QUESTION 0x02
#define STAR 0x03
#define PLUS 0x01
#define PAREN_OPEN 0x05
#define PAREN_CLOSE 0x06

typedef struct State State;
struct State
{
  int c;
  int id;
  State *out;
  State *out1;

  State *dev;

  int lastlist;
  unsigned char free;
};

/*
 * Since the out pointers in the list are always 
 * uninitialized, we use the pointers themselves
 * as storage for the Ptrlists.
 */
typedef union Ptrlist Ptrlist;
union Ptrlist
{
  Ptrlist *next;
  State *s;
};

/*
 * A partially built NFA without the matching state filled in.
 * Frag.start points at the start state.
 * Frag.out is a list of places that need to be set to the
 * next state for this fragment.
 */
typedef struct Frag Frag;
struct Frag
{
  State *start;
  Ptrlist *out;
};

typedef struct List List;
struct List
{
  // only handle fixed number of states
  State *s[100];
  int n;
};

/*
 * Represents an NFA state plus zero or one or two arrows exiting.
 * if c == Match, no arrows out; matching state.
 * If c == Split, unlabeled arrows to out and out1 (if != NULL).
 * If c == Any, unlabeled arrows to out (if != NULL).
 * If c < 256, labeled arrow with character c to out.
 */
enum
{
  Match = 256,
  Split = 257,
  Any   = 258
};

void readFile(char *fileName, char ***lines, int *lineIndex);
char* re2post(char *re);
void usage(const char* progname);
void parseCmdLine(int argc, char **argv, int *visualize, int *postfix, int *time, int *simplified, char **fileName, char **regexFile); 
void visualize_nfa_help(State * start);
void visualize_nfa(State * start);
double gettime();

#define STATE_FREED 
#define STATE_INIT 0

#define DEBUG
#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)
#endif

#ifndef DEBUG
#define LOG(...) //comment
#endif

typedef unsigned int u32;

State* state(int c, State *out, State *out1);
Frag frag(State *start, Ptrlist *out);
Ptrlist* list1(State **outp);
void patch(Ptrlist *l, State *s);
Ptrlist* append(Ptrlist *l1, Ptrlist *l2);
State* post2nfa(char *postfix);

List* startlist(State *start, List *l);
int ismatch(List *l);
void addstate(List *l, State *s);
void step(List *clist, int c, List *nlist);
int match(State *start, char *s);
int  anyMatch(State *start, char *s);

#endif


// --- from pnfa.h ---
#ifndef PNFA_H
#define PNFA_H


#define PRINT(time,...) if(!time) printf(__VA_ARGS__)
#define IS_EMPTY(l) (l->n == 0)
#define PUSH(l, state) l->s[l->n++] = state
#define POP(l) l->s[--(l->n)]; 

// host function which calls parallelNFAKernel
void parallelNFA(char *postfix);
// host function which calls parallelMatchingKernel
void pMatch(char * bigLine, u32 * tableOfLineStarts, int numLines, int numRegexs, int time, char *regexLines, u32 *regexTable, char **lines, u32 *hostLineStarts);
 
#define BUFFER_SIZE 8000

#endif


// --- from regex.h ---
#ifndef REGEX_H
#define REGEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
  int i;
  int size;
  char * re;
} SimpleReBuilder;

/* Constructor */
void simpleReBuilder(SimpleReBuilder ** builder, int len);

/* Destructor */
void _simpleReBuilder(SimpleReBuilder * builder);

void regex_error(int i);
char * stringify(char * nonull, int j);
void handle_escape(SimpleReBuilder * builder, char ** complexRe, int *len, int * bi, int * ci);
void putRange(SimpleReBuilder * builder, char start, char end, int * bi);
void handle_range(SimpleReBuilder * builder, char * complexRe, int len, int * bi, int * ci);
SimpleReBuilder * simplifyRe(char ** complexRe, SimpleReBuilder * builder);
void freeNFAStates(State *s);
char * stringifyRegex(const char * oldRegex);

#endif

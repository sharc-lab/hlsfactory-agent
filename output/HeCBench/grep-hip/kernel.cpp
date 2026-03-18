#include "kernel.h"

// --- from pnfa.cu ---
inline void pstep(List*, int, List *);

inline int pstrlen(char *str) {
  int len = 0; 
  while(*str != 0) {
    len ++;
    str += 1;
  }
  return len;
}

  inline int ispmatch(List *l)
{
  int i;

  for(i=0; i<l->n; i++) {
    if(l->s[i]->c == Match)
      return 1;
  }
  return 0;
}

  inline void paddstate(List *l, State *s, List *addStateList)
{  
  addStateList->n = 0;
  PUSH(addStateList, s);
  /* follow unlabeled arrows */
  while(!IS_EMPTY(addStateList)) {  

    s = POP(addStateList);

    // lastlist check is present to ensure that if
    // multiple states point to this state, then only
    //one instance of the state is added to the list
    if(s == NULL);
    else if (s->c == Split) {
      PUSH(addStateList, s->out);
      PUSH(addStateList, s->out1);  
    }
    else {
      l->s[l->n++] = s;
    }
  }
}

  inline void pstep(List *clist, int c, List *nlist)
{
  int i;
  State *s;
  nlist->n = 0;
  for(i=0; i<clist->n; i++){
    s = clist->s[i];

    if(s->c == c || s->c == Any){
      List addStartState;
      paddstate(nlist, s->out, &addStartState);
    }
  }
}

  inline int pmatch(State *start, char *s, List *dl1, List *dl2)
{
  int c;
  List *clist, *nlist, *t;

  clist = pstartlist(start, dl1);
  nlist = dl2;
  for(; *s; s++){
    c = *s & 0xFF;
    pstep(clist, c, nlist);
    t = clist; clist = nlist; nlist = t;  // swap clist, nlist 
  }
  return ispmatch(clist);
}

inline int panypmatch(State *start, char *s, List *dl1, List *dl2) { 
  int c;
  List *clist, *nlist, *t;

  clist = pstartlist(start, dl1);
  nlist = dl2;
  for(; *s; s++){
    c = *s & 0xFF;
    pstep(clist, c, nlist);
    t = clist; clist = nlist; nlist = t;  // swap clist, nlist 
  }
  return ispmatch(clist);
}
extern "C"

void parallelMatch(
  char * bigLine,
  const u32 * tableOfLineStarts, 
  int numLines,
  char *regexLines,
  const u32 *regexTable, 
  unsigned char * devResult,
  State *pmatchstate) 
{
    #pragma HLS INTERFACE m_axi port=bigLine offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=tableOfLineStarts offset=slave bundle=gmem1
    #pragma HLS INTERFACE s_axilite port=numLines
    #pragma HLS INTERFACE m_axi port=regexLines offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=regexTable offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=devResult offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=pmatchstate offset=slave bundle=gmem5
    #pragma HLS INTERFACE s_axilite port=return
    #pragma HLS ARRAY_PARTITION variable=buf complete dim=1
    #pragma HLS ARRAY_PARTITION variable=s complete dim=1

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1


            char buf[BUFFER_SIZE];
            int pnstate;
            State s[100];
            State *st;

            if (_tid_x == 0) {
            pre2post(regexLines + regexTable[0], buf);

            pnstate = 0;
            st = ppost2nfa(buf, s, &pnstate, pmatchstate);
            }

            List d1;
            List d2;

            int i;
            for (i = _bid_x * BLOCK_DIM_X + _tid_x; i < numLines; i += GRID_DIM_X * BLOCK_DIM_X) {

            char * lineSegment = bigLine + tableOfLineStarts[i];
            if (panypmatch(st, lineSegment, &d1, &d2))
            devResult[i] = 1;
            else
            devResult[i] = 0;

            }

        }
    }
}


// --- from putil.cu ---
inline State* ppost2nfa(char *, State *, int *, State *);

/* Allocate and initialize State */
inline State* pstate(int c, State *out, State *out1, 
    State *lstate, int *pnstate)
{
  State *s = lstate + *pnstate; // assign a state

  s->id = *pnstate;
  (*pnstate)++;
  s->lastlist = 0;
  s->c = c;
  s->out = out;
  s->out1 = out1;

  // device pointer of itself
  // serves no real purpose other than to help transfer the NFA over
  s->dev = NULL;

  s->free = 0;
  return s;
}

  inline void ppatch(Ptrlist *l, State *s)
{
  Ptrlist *next;

  for(; l; l=next){
    next = l->next;
    l->s = s;
  }
}

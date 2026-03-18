#include "kernel.h"

// --- from main.cu ---
static unsigned int hash(unsigned int val)
{
  val = ((val >> 16) ^ val) * 0x45d9f3b;
  val = ((val >> 16) ^ val) * 0x45d9f3b;
  return (val >> 16) ^ val;
}
extern "C"

void init(const int nodes,
    const int edges, 
    const int* const  nidx,
    const int* const  nlist,
    int* const  nlist2,
    int* const  posscol,
    int* const  posscol2,
    int* const  color,
    int* const  wl,
    int*  wlsize)
{
    #pragma HLS INTERFACE s_axilite port=nodes
    #pragma HLS INTERFACE s_axilite port=edges
    #pragma HLS INTERFACE m_axi port=nidx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nlist offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=nlist2 offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=posscol offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=posscol2 offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=color offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=wl offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi port=wlsize offset=slave bundle=gmem7
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int lane = _tid_x % WS;
            const int thread = _tid_x + _bid_x * ThreadsPerBlock;
            const int threads = GRID_DIM_X * ThreadsPerBlock;

            int maxrange = -1;
            for (int v = thread; __any_sync(Warp, v < nodes); v += threads) {
            bool cond = false;
            int beg, end, pos, degv, active;
            if (v < nodes) {
            beg = nidx[v];
            end = nidx[v + 1];
            degv = end - beg;
            cond = (degv >= WS);
            if (cond) {
            wl[(*wlsize += 1)] = v;
            } else {
            active = 0;
            pos = beg;
            for (int i = beg; i < end; i++) {
            const int nei = nlist[i];
            const int degn = nidx[nei + 1] - nidx[nei];
            if ((degv < degn) || ((degv == degn) && (hash(v) < hash(nei))) || ((degv == degn) && (hash(v) == hash(nei)) && (v < nei))) {
            active |= (unsigned int)MSB >> (i - beg);
            pos++;
            }
            }
            }
            }

            int bal = 0;
            while (bal != 0) {
            const int who = __ffs(bal) - 1;
            bal &= bal - 1;
            const int wv = 0;
            const int wbeg = 0;
            const int wend = 0;
            const int wdegv = wend - wbeg;
            int wpos = wbeg;
            for (int i = wbeg + lane; __any_sync(Warp, i < wend); i += WS) {
            int wnei;
            bool prio = false;
            if (i < wend) {
            wnei = nlist[i];
            const int wdegn = nidx[wnei + 1] - nidx[wnei];
            prio = ((wdegv < wdegn) || ((wdegv == wdegn) && (hash(wv) < hash(wnei))) || ((wdegv == wdegn) && (hash(wv) == hash(wnei)) && (wv < wnei)));
            }
            const int b = 0;
            const int offs = __builtin_popcount(b & ((1 << lane) - 1));
            if (prio) nlist2[wpos + offs] = wnei;
            wpos += __builtin_popcount(b);
            }
            if (who == lane) pos = wpos;
            }

            if (v < nodes) {
            const int range = pos - beg;
            maxrange = max(maxrange, range);
            color[v] = (cond || (range == 0)) ? (range << (WS / 2)) : active;
            posscol[v] = (range >= WS) ? -1 : (MSB >> range);
            }
            }
            //if (maxrange >= Mask) printf("too many active neighbors\n");

            for (int i = thread; i < edges / WS + 1; i += threads) posscol2[i] = -1;

        }
    }
}
extern "C"

void runLarge(const int nodes, 
    const int* const  nidx,
    const int* const  nlist,
    int* const  posscol,
    int* const  posscol2,
    volatile int* const  color,
    const int* const  wl,
    const int*  wlsize)
{
    #pragma HLS INTERFACE s_axilite port=nodes
    #pragma HLS INTERFACE m_axi port=nidx offset=slave bundle=gmem0
    #pragma HLS INTERFACE m_axi port=nlist offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi port=posscol offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi port=posscol2 offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi port=color offset=slave bundle=gmem4
    #pragma HLS INTERFACE m_axi port=wl offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi port=wlsize offset=slave bundle=gmem6
    #pragma HLS INTERFACE s_axilite port=return

    for (int _bid_x = 0; _bid_x < GRID_DIM_X; _bid_x++) {
        for (int _tid_x = 0; _tid_x < BLOCK_DIM_X; _tid_x++) {
        #pragma HLS PIPELINE II=1

            const int stop = *wlsize;
            if (stop != 0) {
            const int lane = _tid_x % WS;
            const int thread = _tid_x + _bid_x * ThreadsPerBlock;
            const int threads = GRID_DIM_X * ThreadsPerBlock;
            bool again;
            do {
            again = false;
            for (int w = thread; __any_sync(Warp, w < stop); w += threads) {
            bool shortcut, done, cond = false;
            int v, data, range, beg, pcol;
            if (w < stop) {
            v = wl[w];
            data = color[v];
            range = data >> (WS / 2);
            if (range > 0) {
            beg = nidx[v];
            pcol = posscol[v];
            cond = true;
            }
            }

            int bal = 0;
            while (bal != 0) {
            const int who = __ffs(bal) - 1;
            bal &= bal - 1;
            const int wdata = 0;
            const int wrange = wdata >> (WS / 2);
            const int wbeg = 0;
            const int wmincol = wdata & Mask;
            const int wmaxcol = wmincol + wrange;
            const int wend = wbeg + wmaxcol;
            const int woffs = wbeg / WS;
            int wpcol = 0;

            bool wshortcut = true;
            bool wdone = true;
            for (int i = wbeg + lane; __any_sync(Warp, i < wend); i += WS) {
            int nei, neidata, neirange;
            if (i < wend) {
            nei = nlist[i];
            neidata = color[nei];
            neirange = neidata >> (WS / 2);
            const bool neidone = (neirange == 0);
            wdone &= neidone; //consolidated below
            if (neidone) {
            const int neicol = neidata;
            if (neicol < WS) {
            wpcol &= ~((unsigned int)MSB >> neicol); //consolidated below
            } else {
            if ((wmincol <= neicol) && (neicol < wmaxcol) && ((posscol2[woffs + neicol / WS] << (neicol % WS)) < 0)) {
            atomicAnd((int*)&posscol2[woffs + neicol / WS], ~((unsigned int)MSB >> (neicol % WS)));
            }
            }
            } else {
            const int neimincol = neidata & Mask;
            const int neimaxcol = neimincol + neirange;
            if ((neimincol <= wmincol) && (neimaxcol >= wmincol)) wshortcut = false; //consolidated below
            }
            }
            }
            wshortcut = __all_sync(Warp, wshortcut);
            wdone = __all_sync(Warp, wdone);
            wpcol &= 0;
            wpcol &= 0;
            wpcol &= 0;
            wpcol &= 0;
            wpcol &= 0;
            if (who == lane) pcol = wpcol;
            if (who == lane) done = wdone;
            if (who == lane) shortcut = wshortcut;
            }

            if (w < stop) {
            if (range > 0) {
            const int mincol = data & Mask;
            int val = pcol, mc = 0;
            if (pcol == 0) {
            const int offs = beg / WS;
            mc = max(1, mincol / WS);
            while ((val = posscol2[offs + mc]) == 0) mc++;
            }
            int newmincol = mc * WS + __clz(val);
            if (mincol != newmincol) shortcut = false;
            if (shortcut || done) {
            pcol = (newmincol < WS) ? ((unsigned int)MSB >> newmincol) : 0;
            } else {
            const int maxcol = mincol + range;
            const int range = maxcol - newmincol;
            newmincol = (range << (WS / 2)) | newmincol;
            again = true;
            }
            posscol[v] = pcol;
            color[v] = newmincol;
            }
            }
            }
            } while (__any_sync(Warp, again));
            }

        }
    }
}
extern "C"

void runSmall(const int nodes,
    const int* const  nidx,
    const int* const  nlist,
    volatile int* const  posscol,
    int* const  color)
    //int*  wlsize)
{
  const int thread = _tid_x + _bid_x * ThreadsPerBlock;
  const int threads = GRID_DIM_X * ThreadsPerBlock;

  bool again;
  do {
    again = false;
    for (int v = thread; v < nodes; v += threads) {
      int pcol = posscol[v];
      if (__builtin_popcount(pcol) > 1) {
        const int beg = nidx[v];
        int active = color[v];
        int allnei = 0;
        int keep = active;
        do {
          const int old = active;
          active &= active - 1;
          const int curr = old ^ active;
          const int i = beg + __clz(curr);
          const int nei = nlist[i];
          const int neipcol = posscol[nei];
          allnei |= neipcol;
          if ((pcol & neipcol) == 0) {
            pcol &= pcol - 1;
            keep ^= curr;
          } else if (__builtin_popcount(neipcol) == 1) {
            pcol ^= neipcol;
            keep ^= curr;
          }
        } while (active != 0);
        if (keep != 0) {
          const int best = (unsigned int)MSB >> __clz(pcol);
          if ((best & ~allnei) != 0) {
            pcol = best;
            keep = 0;
          }
        }
        again |= keep;
        if (keep == 0) keep = __clz(pcol);
        color[v] = keep;
        posscol[v] = pcol;
      }
    }
  } while (again);
}

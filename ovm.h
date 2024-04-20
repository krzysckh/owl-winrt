#ifndef __OVM_H__
#define __OVM_H__

#include <inttypes.h>

typedef unsigned int uint;
typedef uint8_t byte;
typedef uintptr_t word; /* heap cell */
typedef uint32_t hval; /* heap value */
typedef intptr_t wdiff;

/*** Macros ***/

#define MIN2(a,b)                   ((a)<(b)?(a):(b))
#define mkport(fd)                  make_immediate(fd, TPORT)
#define IPOS                        8 /* offset of immediate payload */
#define SPOS                        16 /* offset of size bits in header immediate values */
#define TPOS                        2  /* offset of type bits in header */
#define V(ob)                       (*(word *)(ob))
#define W                           ((uint)sizeof(word))
#define LDW                         ((W >> 3) + 2) /* poor man's log2(W), valid for 4, 8, 16 */
#define NWORDS                      1024*1024*8    /* static malloc'd heap size if used as a library */
#define FBITS                       24             /* bits in fixnum, on the way to 24 and beyond */
#define FMAX                        ((1<<FBITS)-1) /* maximum fixnum (and most negative fixnum) */
#define MAXOBJ                      0xffff         /* max words in tuple including header */
#define MAXPAYL                     ((MAXOBJ - 1) * W) /* maximum payload in an allocated object */
#define RAWBIT                      2048
#define FPOS                        (SPOS - LDW) /* offset of the fractional part in the header size */
#define payl_len(hdr)               (((hval)hdr >> FPOS) - W - (W - 1))
#define make_immediate(value, type) ((hval)(value) << IPOS | (type) << TPOS | 2)
#define make_header(size, type)     ((hval)(size) << SPOS | (type) << TPOS | 2)
#define BOOL(cval)                  ((cval) ? ITRUE : IFALSE)
#define immval(desc)                ((hval)(desc) >> IPOS)
#define fixnump(desc)               (((desc) & 255) == 2)
#define NR                          98 /* FIXME: should be ~32, see owl/register.scm:/define.n-registers/ */
#define header(x)                   V(x)
#define is_type(x, t)               (((x) & (63 << TPOS | 2)) == ((t) << TPOS | 2))
#define objsize(x)                  ((hval)(x) >> SPOS)
#define immediatep(x)               ((word)(x) & 2)
#define allocp(x)                   (!immediatep(x))
#define rawp(hdr)                   ((hdr) & RAWBIT)
#define NEXT(n)                     ip += n; continue
#define PAIRHDR                     make_header(3, 1)
#define NUMHDR                      make_header(3, 40) /* <- on the way to 40, see type-int+ in defmac.scm */
#define NUMNHDR                     make_header(3, 41)
#define pairp(ob)                   (allocp(ob) && V(ob) == PAIRHDR)
#define cons(a, d)                  mkpair(PAIRHDR, a, d)
#define INULL                       make_immediate(0, 13)
#define IFALSE                      make_immediate(1, 13)
#define ITRUE                       make_immediate(2, 13)
#define IEMPTY                      make_immediate(3, 13) /* empty ff */
#define IEOF                        make_immediate(4, 13)
#define IHALT                       make_immediate(5, 13)
#define TNUM                        0
#define TTUPLE                      2
#define TSTRING                     3
#define TPORT                       12
#define TTHREAD                     31
#define TNUMN                       32
#define TBVEC                       19
#define TBYTECODE                   16
#define TPROC                       17
#define TCLOS                       18
#define F(value)                    make_immediate(value, TNUM)
#define stringp(ob)                 (allocp(ob) && (V(ob) & make_header(0, 63)) == make_header(0, TSTRING))
#define FLAG                        1
#define cont(n)                     V((word)(n) & ~FLAG)
#define flag(n)                     ((word)(n) ^ FLAG)
#define flagged(n)                  ((word)(n) & FLAG)
#define flagged_or_raw(n)           ((word)(n) & (RAWBIT | FLAG))
#define TBIT                        1024
#define teardown_needed(hdr)        ((word)(hdr) & TBIT)
#define A0                          R[*ip]
#define A1                          R[ip[1]]
#define A2                          R[ip[2]]
#define A3                          R[ip[3]]
#define A4                          R[ip[4]]
#define A5                          R[ip[5]]
#define G(ptr, n)                   (((word *)(ptr))[n])
#define TICKS                       10000 /* # of function calls in a thread quantum */
#define allocate(size, to)          (to = fp, fp += size)
#define error(opcode, a, b)         do { R[4] = F(opcode); R[5] = (word)(a); R[6] = (word)(b); goto invoke_mcp; } while (0)
#define assert(exp, val, code)      if (!(exp)) error(code, val, ITRUE)
#define assert_not(exp, val, code)  if (exp) error(code, val, ITRUE)
#define MEMPAD                      (NR + 2) * 8 /* space at end of heap for starting GC */
#define MINGEN                      1024 * 32 /* minimum generation size before doing full GC */
#define INITCELLS                   100000
#define SIGGC                       9  /* signal 9=SIGKILL cannot be caught, so use its place for gc signaling */

uint llen(word *ptr);
word mkpair(word h, word a, word d);
word mkint(uint64_t x);
int64_t cnum(word a);
word onum(int64_t n, uint s);
word mkstring(char *s);
#endif

/* -*- mode: c; c-basic-offset: 3 -*- */
/* Owl Lisp runtime */

#include <stdint.h>

#ifndef NOT_RT
#define RT
#endif

#ifndef RT
static void* heap = 0;
#endif

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define NOGDI
#define NOUSER
#define _INC_MMSYSTEM
typedef struct tagMSG *LPMSG;

#include <winsock2.h>
#include <ws2tcpip.h>

#undef VOID

/* defined like that, so it can be downloaded as 1 file, and also used with extensions */
#ifndef __OVM_H__
#define __OVM_H__

#include <inttypes.h>

typedef unsigned int uint;
typedef uint8_t byte;
typedef uintptr_t word; /* heap cell */
typedef uint32_t hval; /* heap value */
typedef intptr_t wdiff;

/*** Macros ***/

#define car(l)                      G(l, 1)
#define cdr(l)                      G(l, 2)
#define PTR(t)                      mkseq((byte*)&t, sizeof(uintptr_t), TBVEC)
#define cptr(v)                     ((void*)*(uintptr_t*)(v+W))
#define mkstring(s)                 mkseq(s, strlen(s), TSTRING)
#define mkbvec(vp, n)               mkseq(vp, n, TBVEC)
#define MIN2(a,b)                   ((a)<(b)?(a):(b))
#define ABS(a)                      ((a<0)?-(a):(a))
#define mkport(fd)                  make_immediate(fd, TPORT)
#define cstr(s)                     ((char*)s+W) /* word as cstring */
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
#define TRAT                        42
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
word mkrat(int64_t p, int64_t q);
word mkrat_approx(float f);
double cdouble(word);
word mkseq(uint8_t *v, unsigned long n, uint T);
#endif
#ifdef SILENT
#define not_implemented(s, why) (void)0;
#else
#define not_implemented(s, why) fprintf(stderr, "vm-error: %s: not implemented: %s (why?: %s)\n", __func__, s, why)
#endif

#ifdef PRIM_CUSTOM
word prim_custom(int op, word a, word b, word c);
#endif

#ifndef EMULTIHOP
#define EMULTIHOP -1
#endif
#ifndef ENODATA
#define ENODATA -1
#endif
#ifndef ENOLINK
#define ENOLINK -1
#endif
#ifndef ENOSR
#define ENOSR -1
#endif
#ifndef ENOSTR
#define ENOSTR -1
#endif
#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE -1
#endif
#ifndef EOWNERDEAD
#define EOWNERDEAD -1
#endif
#ifndef ETIME
#define ETIME -1
#endif
#ifndef F_DUPFD_CLOEXEC
#define F_DUPFD_CLOEXEC -1
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_EXEC
#define O_EXEC 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_RSYNC
#define O_RSYNC 0
#endif
#ifndef O_DSYNC
#define O_DSYNC 0
#endif
#ifndef O_SEARCH
#define O_SEARCH 0
#endif
#ifndef O_TTY_INIT
#define O_TTY_INIT 0
#endif
#ifdef __APPLE__
#define st_atim st_atimespec
#define st_mtim st_mtimespec
#define st_ctim st_ctimespec
#endif

/* winrt defines */
enum {
   O_DIRECTORY,
   O_NOCTTY,
   O_NONBLOCK,
   O_SYNC,
   FD_CLOEXEC,
   F_DUPFD,
   F_GETFD,
   F_SETFD,
   F_GETFL,
   F_SETFL,
   F_GETOWN,
   F_SETOWN,
   F_GETLK,
   F_SETLK,
   F_SETLKW,
   F_RDLCK,
   F_UNLCK,
   F_WRLCK,
};
/* winrt defines */

int
fcntl(int fd, int cmd, intptr_t arg)
{
   switch (cmd) {
   case F_DUPFD:   not_implemented("F_DUPFD",  ""); break;
   case F_GETFD:   not_implemented("F_GETFD",  ""); break;
   case F_SETFD:   not_implemented("F_SETFD",  ""); break;
   case F_GETFL:   return 0; // me when i lie :3
   case F_SETFL: {
      unsigned long mode = arg & O_NONBLOCK ? 1 : 0;
      return ioctlsocket(fd, FIONBIO, &mode);
   }
   case F_GETOWN:  not_implemented("F_GETOWN", ""); break;
   case F_SETOWN:  not_implemented("F_SETOWN", ""); break;
   case F_GETLK:   not_implemented("F_GETLK",  ""); break;
   case F_SETLK:   not_implemented("F_SETLK",  ""); break;
   case F_SETLKW:  not_implemented("F_SETLKW", ""); break;
   case F_RDLCK:   not_implemented("F_RDLCK",  ""); break;
   case F_UNLCK:   not_implemented("F_UNLCK",  ""); break;
   case F_WRLCK:   not_implemented("F_WRLCK",  ""); break;
   }

   return -1;
}

volatile unsigned int signals; /* caught signals */

void catch_signal(int signal) {
   //fprintf(stderr, "[vm: caught signal %d]\n", signal);
   signals |= (1 << signal);
}

/*** Globals and Prototypes ***/

WSADATA wsa;
int wsaerrno = -1;
extern char **environ;

/* memstart <= genstart <= memend */
word *genstart;
word *memstart;
word *memend;
hval max_heap_mb; /* max heap size in MB */
word state;       /* IFALSE | previous program state across runs */
const byte *hp;
word *fp;
byte *file_heap;
uint64_t nalloc;
size_t maxheap;

/*** Garbage Collector, based on "Efficient Garbage Compaction Algorithm" by Johannes Martin (1982) ***/

void rev(word pos) {
   word val = V(pos);
   word next = cont(val);
   V(pos) = next;
   cont(val) = (pos | FLAG) ^ (val & FLAG);
}

word *chase(word *pos) {
   word val = cont(pos);
   while (allocp(val) && flagged(val)) {
      pos = (word *)val;
      val = cont(pos);
   }
   return pos;
}

void mark(word *pos, word *end) {
   while (pos != end) {
      word val = *pos;
      if (allocp(val) && val >= (word)genstart) {
         if (flagged(val)) {
            pos = (word *)flag(chase((word *)val));
         } else {
            word hdr = header(val);
            rev((word) pos);
            if (!flagged_or_raw(hdr))
               pos = (word *)val + objsize(hdr);
         }
      }
      --pos;
   }
}

word *compact() {
   word *new = genstart;
   word *old = new;
   word *end = memend - 1;
   while (old < end) {
      word val = *old;
      if (flagged(val)) {
         hval h;
         *new = val;
         do { /* unthread */
            rev((word) new);
         } while (flagged(*new));
         h = objsize(*new);
         if (old == new) {
            old += h;
            new += h;
         } else {
            while (--h)
               *++new = *++old;
            old++;
            new++;
         }
      } else {
         /* if (teardown_needed(val))
            printf("gc: would teardown\n"); */
         old += objsize(val);
      }
   }
   return new;
}

void fix_pointers(word *pos, wdiff delta) {
   for (;;) {
      word hdr = *pos;
      hval n = objsize(hdr);
      if (hdr == 0) /* end marker reached. only dragons beyond this point. */
         return;
      if (rawp(hdr)) {
         pos += n; /* no pointers in raw objects */
      } else {
         for (++pos; --n; ++pos) {
            word val = *pos;
            if (allocp(val))
               *pos = val + delta;
         }
      }
   }
}


/* emulate sbrk with malloc'd memory, because sbrk is no longer properly supported */
/* n-cells-wanted → heap-delta (to be added to pointers), updates memstart and memend */
wdiff adjust_heap(wdiff cells) {
   word *old = memstart;
   word nwords = memend - memstart + MEMPAD; /* MEMPAD is after memend */
   word new_words = nwords + (cells > 0xffffff ? 0xffffff : cells); /* limit heap growth speed */
   if (((cells > 0) && (new_words*W < nwords*W)) || ((cells < 0) && (new_words*W > nwords*W)))
      return 0; /* don't try to adjust heap, if the size_t would overflow in realloc */
   memstart = realloc(memstart, new_words*W);
   if (!memstart) {
      catch_signal(SIGGC);
      return 0;
   } else {
      wdiff delta = (word)memstart - (word)old;
      memend = memstart + new_words - MEMPAD; /* leave MEMPAD words alone */
      if (delta)
         fix_pointers(memstart, delta); /* d'oh! we need to O(n) all the pointers... */
      return delta;
   }
}

/* input desired allocation size and (the only) pointer to root object
   return a pointer to the same object after heap compaction, possible heap size change and relocation */
word *gc(int size, word *regs) {
   word *root;
   word *realend = memend;
   wdiff nfree;
   fp = regs + objsize(*regs);
   root = fp+1;
   *root = (word) regs;
   memend = fp;
   nalloc += fp - genstart;
   mark(root, fp);
   fp = compact();
   regs = (word *)*root;
   memend = realend;
   nfree = (word)memend - (word)regs;
   if (genstart == memstart) {
      word heapsize = (word) memend - (word) memstart;
      word nused = heapsize - nfree;
      if (maxheap < nused)
         maxheap = nused;
      if (heapsize / (1024 * 1024) > max_heap_mb)
         catch_signal(SIGGC);
      nfree -= size*W + MEMPAD; /* how much really could be snipped off */
      if (nfree < (heapsize / 3) || nfree < 0) {
         /* increase heap size if less than 33% is free by ~10% of heap size (growth usually implies more growth) */
         regs[objsize(*regs)] = 0; /* use an invalid descriptor to denote end live heap data */
         regs = (word *) ((word)regs + adjust_heap(size*W + nused/10 + 4096));
         nfree = memend - regs;
         if (nfree <= size)
            catch_signal(SIGGC);
      } else if (nfree > (heapsize/2)) {
         /* decrease heap size if more than 50% is free by 10% of the free space */
         wdiff dec = -(nfree / 10);
         wdiff new = nfree - dec;
         if (new > size*W*2 + MEMPAD) {
            regs[objsize(*regs)] = 0; /* as above */
            regs = (word *) ((word)regs + adjust_heap(dec+MEMPAD*W));
            heapsize = (word)memend - (word)memstart;
            nfree = (word)memend - (word)regs;
         }
      }
      genstart = regs; /* always start new generation */
   } else if (nfree < MINGEN || nfree < size*W*2) {
      genstart = memstart; /* start full generation */
      return gc(size, regs);
   } else {
      genstart = regs; /* start new generation */
   }
   return regs;
}

/*** OS Interaction and Helpers ***/



/* list length, no overflow or valid termination checks */
uint llen(word *ptr) {
   uint len = 0;
   while (pairp(ptr)) {
      len++;
      ptr = (word *) ptr[2];
   }
   return len;
}

word mkpair(word h, word a, word d) {
   word *pair;
   allocate(3, pair);
   pair[0] = h;
   pair[1] = a;
   pair[2] = d;
   return (word)pair;
}

/* recursion depth does not exceed two */
word mkint(uint64_t x) {
   return mkpair(NUMHDR, F(x), x > FMAX ? mkint(x >> FBITS) : INULL);
}

/* make a raw object to hold len bytes (compute size, advance fp, clear padding) */
word mkraw(uint type, hval len) {
   word *ob;
   byte *end;
   hval hdr = (W + len + W - 1) << FPOS | RAWBIT | make_header(0, type);
   uint pads = -len % W;
   allocate(objsize(hdr), ob);
   *ob = hdr;
   end = (byte *)ob + W + len;
   while (pads--)
      *end++ = 0; /* clear the padding bytes */
   return (word)ob;
}

word mkseq(uint8_t *v, unsigned long n, uint T) {
   size_t len = memend - fp,
      max = len > MAXOBJ ? MAXPAYL + 1 : (len - 1) * W,
      N = MIN2(max-1, n);

   memcpy((uint8_t*)fp + W, v, N);
   return mkraw(T, N);
}

word mkrat(int64_t p, int64_t q) {
   word s = 2, *ob, po = onum(p, 1), qo = onum(q, 1);

   allocate(s+1, ob);
   *ob = make_header(s+1, TRAT);

   ob[1] = po;
   ob[2] = qo;

   return (word)ob;
}

// not copying the new function yet
word mkrat_approx(float f) {
   int64_t v = 1, m = f < 0;
   f = ABS(f);

   if (f >= (float)INT64_MAX)
      return IFALSE;

   while ((float)(f - (int64_t)f) > 0.0) {
      f *= 10, v *= 10;
      if (v < 0) /* v overflew */
        return IFALSE;
   }

   return mkrat(m ? -f : f, v);
}

double cdouble(word x) {
   if (allocp(x)) {
      if (is_type(*(word *)x,TRAT))
         return (double)cnum(car(x))/(double)cnum(cdr(x));
      else if (header(x) == NUMHDR || header(x) == NUMNHDR)
         return (double)cnum(x);
   } else if (is_type(x,TNUM)||is_type(x,TNUMN))
      return (double)cnum(x);

   return 0.0/0.0;
}


/*** Primops called from VM and generated C-code ***/

/* win32 sockets are not ports, so you cannot write() to them */
#define MAX_SOCKETS (2<<12)
int sockets[MAX_SOCKETS] = {0};

#define issocket(fd) ((fd) < MAX_SOCKETS && sockets[(fd)])

int
w32write(int fd, char* buf, size_t n)
{
   if (issocket(fd))
      return send(fd, buf, n, 0);

   return write(fd, buf, n);
}

int
w32read(int fd, char* buf, size_t n)
{
   if (issocket(fd))
      return recv(fd, buf, n, 0);

   return read(fd, buf, n);
}

int
w32close(int fd)
{
   if (issocket(fd)) {
      sockets[fd] = 0;
      return closesocket(fd);
   }

   return close(fd);
}


hval prim_connect(word *host, word port, word type) {
   int sock;
   byte *ip = (byte *)host + W;
   unsigned long ipfull;
   struct sockaddr_in addr;
   char udp = (immval(type) == 1);
   port = immval(port);

   if ((sock = socket(AF_INET, (udp ? SOCK_DGRAM : SOCK_STREAM), (udp ? IPPROTO_UDP : IPPROTO_TCP))) == INVALID_SOCKET) {
      return IFALSE;
   }

   if (udp)
      return make_immediate(sock, TPORT);

   if (!allocp(host)) /* bad host type */
      return IFALSE;

   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   /* addr.sin_addr.s_addr = (ULONG)host[1]; */

   ipfull = ip[0] << 24 | ip[1] << 16 | ip[2] << 8 | ip[3];
   addr.sin_addr.s_addr = htonl(ipfull);

   if (connect(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
      closesocket(sock);
      return IFALSE;
   }

   sockets[sock] = 1;

   return make_immediate(sock, TPORT);
}

word prim_less(word a, word b) {
   if (immediatep(a))
      return immediatep(b) ? BOOL(a < b) : ITRUE;  /* imm < alloc */
   else
      return immediatep(b) ? IFALSE : BOOL(a < b); /* alloc > imm */
}

word prim_ref(word pword, hval pos) {
   hval hdr;
   if (immediatep(pword))
      return IFALSE;
   hdr = header(pword);
   pos = immval(pos);
   if (rawp(hdr)) { /* raw data is #[hdrbyte{W} b0 .. bn 0{0,W-1}] */
      if (payl_len(hdr) <= pos)
         return IFALSE;
      return F(((byte *)pword)[W + pos]);
   }
   if (!pos || objsize(hdr) <= pos) /* tuples are indexed from 1 (probably later 0-255) */
      return IFALSE;
   return G(pword, pos);
}

int64_t cnum(word a) {
   uint64_t x;
   if (allocp(a)) {
      word *p = (word *)a;
      uint shift = 0;
      x = 0;
      do {
         x |= (uint64_t)immval(p[1]) << shift;
         shift += FBITS;
         p = (word *)p[2];
      } while (shift < 64 && allocp(p));
      return header(a) == NUMNHDR ? -x : x;
   }
   x = immval(a);
   return is_type(a, TNUMN) ? -x : x;
}

word onum(int64_t n, uint s) {
   word h = NUMHDR, t = TNUM;
   if (s && n < 0) {
      h = NUMNHDR;
      t = TNUMN;
      n = -n;
   }
   if (n > FMAX) {
      word p = mkint(n);
      header(p) = h;
      return p;
   }
   return make_immediate(n, t);
}

word prim_set(word wptr, hval pos, word val) {
   word *ob = (word *)wptr;
   hval hdr, p;
   word *new;
   pos = immval(pos);
   if (immediatep(ob))
      return IFALSE;
   hdr = *ob;
   if (rawp(hdr) || (hdr = objsize(hdr)) < pos)
      return IFALSE;
   allocate(hdr, new);
   for (p = 0; p <= hdr; ++p)
      new[p] = (pos == p && p) ? val : ob[p];
   return (word) new;
}

void setdown() {
   not_implemented("setdown", "no termios");
}

word do_poll(word, word, word);

/* system- and io primops */
word prim_sys(word op, word a, word b, word c) {
   switch (immval(op)) {
   case 0: { /* clock_gettime clock_id → nanoseconds */
      LARGE_INTEGER frequency, currentTime;
      QueryPerformanceFrequency(&frequency);
      QueryPerformanceCounter(&currentTime);

      double ns = (double)currentTime.QuadPart / (double)frequency.QuadPart * 1e9;
      return onum(ns, 1);
   }
   case 1: /* open path flags mode → port | #f */
      if (stringp(a)) {
         int fd = _open((const char *)a + W, cnum(b)|_O_BINARY, immval(c));
         if (fd != -1)
            return make_immediate(fd, TPORT);
      }
      return IFALSE;
   case 2:
      return BOOL(w32close(immval(a)) == 0);
   case 3: { /* 3 = sopen port 0=tcp|1=udp -> False | fd  */
      int port = immval(a);
      int type = immval(b);
      int s;
      int opt = 1; /* TRUE */
      char udp = (type == 1);
      struct sockaddr_in myaddr;
      myaddr.sin_family = AF_INET;
      myaddr.sin_port = htons(port);
      myaddr.sin_addr.s_addr = INADDR_ANY;
      s = socket(AF_INET, (udp ? SOCK_DGRAM : SOCK_STREAM), (udp ? IPPROTO_UDP : 0));
      fcntl(s, F_SETFL, O_NONBLOCK);
      if (s < 0)
         return IFALSE;
      if (type != 1) {
         if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) \
             || bind(s, (struct sockaddr *) &myaddr, sizeof(myaddr)) != 0 \
             || listen(s, SOMAXCONN) != 0) {
            close(s);
            return IFALSE;
         }
      } else {
         if (bind(s, (struct sockaddr *) &myaddr, sizeof(myaddr)) != 0) {
            close(s);
            return IFALSE;
         }
      }
      sockets[s] = 1;
      return make_immediate(s, TPORT); }
   case 4: { /* 4 = accept port -> rval=False|(ip . fd) */
      int sock = immval(a);
      struct sockaddr_in addr;
      socklen_t len = sizeof(addr);
      SOCKET fd;
      word ipa;
      fd = accept(sock, (struct sockaddr *)&addr, &len);
      if (fd == INVALID_SOCKET) return IFALSE;
      sockets[fd] = 1;
      ipa = mkraw(TBVEC, 4);
      memcpy((word *)ipa + 1, &addr.sin_addr, 4);
      return cons(ipa, make_immediate(fd, TPORT)); }
   case 5: /* read fd len -> bvec | EOF | #f */
      if (is_type(a, TPORT)) {
         long long int len = memend - fp;
         const size_t max = len > MAXOBJ ? MAXPAYL : (len - 1) * W;
         len = cnum(b);

         len = w32read(immval(a), (void*)(fp + 1), len < max ? len : max);
         if (len < 0) {
            wsaerrno = WSAGetLastError();
            return IFALSE;
         }
         if (len == 0)
            return IEOF;
         return mkraw(TBVEC, len);
      } return IFALSE;
   case 6:
      setdown();
      exit(immval(a)); /* stop the press */
   case 7: /* set memory limit (in mb) */
      max_heap_mb = immval(a);
      return a;
   case 8: { /* return system constants */
      const word sysconst[] = {
         S_IFMT, W, S_IFBLK, S_IFCHR, S_IFIFO, S_IFREG, S_IFDIR, 0,
         0, E2BIG, EACCES, EADDRINUSE, EADDRNOTAVAIL, EAFNOSUPPORT, EAGAIN, EALREADY,
         EBADF, EBADMSG, EBUSY, ECANCELED, ECHILD, ECONNABORTED, ECONNREFUSED, ECONNRESET,
         EDEADLK, EDESTADDRREQ, EDOM, WSAEDQUOT, EEXIST, EFAULT, EFBIG, EHOSTUNREACH,
         EIDRM, EILSEQ, EINPROGRESS, EINTR, EINVAL, EIO, EISCONN, EISDIR,
         ELOOP, EMFILE, EMLINK, EMSGSIZE, EMULTIHOP, ENAMETOOLONG, ENETDOWN, ENETRESET,
         ENETUNREACH, ENFILE, ENOBUFS, ENODATA, ENODEV, ENOENT, ENOEXEC, ENOLCK,
         ENOLINK, ENOMEM, ENOMSG, ENOPROTOOPT, ENOSPC, ENOSR, ENOSTR, ENOSYS,
         ENOTCONN, ENOTDIR, ENOTEMPTY, ENOTRECOVERABLE, ENOTSOCK, ENOTSUP, ENOTTY, ENXIO,
         EOPNOTSUPP, EOVERFLOW, EOWNERDEAD, EPERM, EPIPE, EPROTO, EPROTONOSUPPORT, EPROTOTYPE,
         ERANGE, EROFS, ESPIPE, ESRCH, WSAESTALE, ETIME, ETIMEDOUT, ETXTBSY,
         WSAEWOULDBLOCK, EXDEV, SEEK_SET, SEEK_CUR, SEEK_END, O_EXEC, O_RDONLY, O_RDWR,
         O_SEARCH, O_WRONLY, O_APPEND, O_CLOEXEC, O_CREAT, O_DIRECTORY, O_DSYNC, O_EXCL,
         O_NOCTTY, O_NOFOLLOW, O_NONBLOCK, O_RSYNC, O_SYNC, O_TRUNC, O_TTY_INIT, O_ACCMODE,
         FD_CLOEXEC, F_DUPFD, F_DUPFD_CLOEXEC, F_GETFD, F_SETFD, F_GETFL, F_SETFL, F_GETOWN,
         F_SETOWN, F_GETLK, F_SETLK, F_SETLKW, F_RDLCK, F_UNLCK, F_WRLCK, CLOCK_MONOTONIC,
         CLOCK_PROCESS_CPUTIME_ID, CLOCK_REALTIME, CLOCK_THREAD_CPUTIME_ID
      };
      return onum(sysconst[immval(a) % (sizeof sysconst / W)], 0); }
   case 9: /* return process variables */
      return onum(
                  a == F(0) ? (wsaerrno >= 0 ? wsaerrno : errno) :
                  a == F(1) ? (uintptr_t)environ :
                  a == F(8) ? nalloc + fp - memstart : /* total allocated objects so far */
                  a == F(9) ? maxheap : /* maximum heap size in a major gc */
                  max_heap_mb, 0);
   case 10: { /* receive-udp-packet sock → (ip-bvec . payload-bvec)| #false */
      struct sockaddr_in si_other;
      socklen_t slen = sizeof(si_other);
      word bvec, ipa;
      int recvd;
      recvd = recvfrom(immval(a), (void*)(fp + 1), 65528, 0, (struct sockaddr *)&si_other, &slen);
      if (recvd < 0)
         return IFALSE;
      bvec = mkraw(TBVEC, recvd);
      ipa = mkraw(TBVEC, 4);
      memcpy((word *)ipa + 1, &si_other.sin_addr, 4);
      return cons(ipa, bvec); }
   case 11: /* open-dir path → dirobjptr | #false */
      if (stringp(a)) {
         DIR *dirp = opendir((const char *)a + W);
         if (dirp != NULL)
            return onum((intptr_t)dirp, 1);
      }
      return IFALSE;
   case 12: { /* read-dir dirp → pointer */
      struct dirent *ent;
      errno = 0;
      ent = readdir((DIR *)(intptr_t)cnum(a));
      return onum(ent != NULL ? (uintptr_t)&ent->d_name : 0, 0); }
   case 13: /* close-dir dirp → bool */
      return BOOL(closedir((DIR *)(intptr_t)cnum(a)) == 0);
   case 14: /* strerror errnum → pointer */
      return onum((uintptr_t)strerror(immval(a)), 0);
   case 15: /* fcntl port cmd arg → integer | #f */
      if (is_type(a, TPORT)) {
         int res = fcntl(immval(a), cnum(b), (intptr_t)cnum(c));
         if (res != -1)
            return onum(res, 1);
      }
      return IFALSE;
   case 16: /* getenv key → pointer */
      return onum(stringp(a) ? (uintptr_t)getenv((const char *)a + W) : 0, 0);
   case 17: { /* exec[v] path argl ret */
      char *path = (char *)a + W;
      int nargs = llen((word *)b);
      char **argp, **args = realloc(NULL, (nargs + 1) * sizeof(char *));
      if (args == NULL)
         return IFALSE;
      for (argp = args; nargs--; ++argp) {
         *argp = (char *)G(b, 1) + W;
         b = G(b, 2);
      }
      *argp = NULL;
      execv(path, args); /* may return -1 and set errno */
      free(args);
      return IFALSE; }
   case 18: { /* fork → #f: failed, 0: we're in child process, integer: we're in parent process */
      not_implemented("fork", "thank you winapi");
      return IFALSE;
   }
   case 19: { /* wait <pid> <respair> _ */
      not_implemented("wait", "i'm too lazy sorry");
      return IFALSE;
      /* pid_t pid = a != IFALSE ? cnum(a) : -1; */
      /* int status; */
      /* word *r = (word *)b; */
      /* pid = waitpid(pid, &status, WNOHANG|WUNTRACED); /\* |WCONTINUED *\/ */
      /* if (pid == -1) */
      /*    return IFALSE; /\* error *\/ */
      /* if (pid == 0) */
      /*    return ITRUE; /\* no changes, would block *\/ */
      /* if (WIFEXITED(status)) { */
      /*    r[1] = F(1); */
      /*    r[2] = F(WEXITSTATUS(status)); */
      /* } else if (WIFSIGNALED(status)) { */
      /*    r[1] = F(2); */
      /*    r[2] = F(WTERMSIG(status)); */
      /* } else if (WIFSTOPPED(status)) { */
      /*    r[1] = F(3); */
      /*    r[2] = F(WSTOPSIG(status)); */
      /* /\*} else if (WIFCONTINUED(status)) { */
      /*    r[1] = F(4); */
      /*    r[2] = F(1); *\/ */
      /* } else { */
      /*    r = (word *)IFALSE; */
      /* } */
      /* return (word)r; */
   }
   case 20: /* chdir path → bool */
      return BOOL(stringp(a) && chdir((char *)a + W) == 0);
   case 21: /* kill pid signal → bool */
      not_implemented("kill", "i'm too lazy");
      return IFALSE;
      /* return BOOL(kill(cnum(a), immval(b)) == 0); */
   case 22: /* unlink path → bool */
      return BOOL(stringp(a) && unlink((char *)a + W) == 0);
   case 23: /* rmdir path → bool */
      return BOOL(stringp(a) && rmdir((char *)a + W) == 0);
   case 24: /* mknod path (type . mode) dev → bool */
      if (stringp(a) && pairp(b)) {
         const char *path = (const char *)a + W;
         mkdir(path);
         return ITRUE;
      }
      return IFALSE;
   case 25: /* lseek port offset whence → offset | #f */
      if (is_type(a, TPORT)) {
         off_t o = lseek(immval(a), cnum(b), cnum(c));
         if (o != -1)
            return onum(o, 1);
      }
      return IFALSE;
   case 26:
      not_implemented("termios", "no termios");
      return IFALSE;
   case 27: { /* sendmsg sock (port . ipv4) bvec */
      int sock = immval(a);
      int port;
      struct sockaddr_in peer;
      byte *ip, *data = (byte *)c + W;
      size_t len = payl_len(header(c));
      port = immval(G(b, 1));
      ip = (byte *)G(b, 2) + W;
      peer.sin_family = AF_INET;
      peer.sin_port = htons(port);
      peer.sin_addr.s_addr = htonl(ip[0] << 24 | ip[1] << 16 | ip[2] << 8 | ip[3]);
      return BOOL(sendto(sock, (void*)data, len, 0, (struct sockaddr *)&peer, sizeof(peer)) != -1); }
   case 28: /* setenv <owl-raw-bvec-or-ascii-leaf-string> <owl-raw-bvec-or-ascii-leaf-string-or-#f> */
      if (stringp(a) && (b == IFALSE || stringp(b))) {
         const char *name = (const char *)a + W;
         return ((b != IFALSE ?
                  (SetEnvironmentVariable(name, (const char *)b + W) ? ITRUE : IFALSE)
                  : (SetEnvironmentVariable(name, "") ? ITRUE : IFALSE)));
      } return IFALSE;
   case 29:
      return prim_connect((word *) a, b, c);
   case 30: /* dup2 old-port new-fd → new-port | #f */
      if (is_type(a, TPORT)) {
         int fd = dup2(immval(a), immval(b));
         if (fd != -1)
            return make_immediate(fd, TPORT);
      }
      return IFALSE;
   case 31: { /* pipe → '(read-port . write-port) | #f */
      int fd[2];
      CreatePipe((PHANDLE)&fd[0], (PHANDLE)&fd[1], NULL, 0);
      return cons(make_immediate(fd[0], TPORT), make_immediate(fd[1], TPORT)); }
   case 32: /* rename src dst → bool */
      return BOOL(stringp(a) && stringp(b) && rename((char *)a + W, (char *)b + W) == 0);
   case 33: /* link src dst → bool */
      not_implemented("link", "balls");
      return IFALSE;
      /* return BOOL(stringp(a) && stringp(b) && link((char *)a + W, (char *)b + W) == 0); */
   case 34: /* symlink src dst → bool */
      not_implemented("symlink", "too alpha");
      return IFALSE;
      /* return BOOL(stringp(a) && stringp(b) && symlink((char *)a + W, (char *)b + W) == 0); */
   case 35: /* readlink path → raw-sting | #false */
      /* if (stringp(a)) { */
      /*    size_t len = memend - fp; */
      /*    size_t max = len > MAXOBJ ? MAXPAYL + 1 : (len - 1) * W; */
      /*    /\* the last byte is temporarily used to check, if the string fits *\/ */
      /*    len = readlink((const char *)a + W, (char *)fp + W, max); */
      /*    if (len != (size_t)-1 && len != max) */
      /*       return mkraw(TSTRING, len); */
      /* } */
      not_implemented("readlink", "i'm too lazy");
      return IFALSE;
   case 36: /* getcwd → raw-sting | #false */
      {
         size_t len = memend - fp;
         size_t max = len > MAXOBJ ? MAXPAYL + 1 : (len - 1) * W;
         /* the last byte is temporarily used for the terminating '\0' */
         if (getcwd((char *)fp + W, max) != NULL)
            return mkraw(TSTRING, strnlen((char *)fp + W, max - 1));
      }
      return IFALSE;
   case 37: /* umask mask → mask */
      return F(umask(immval(a)));
   case 38: /* stat fd|path follow → list */
      if (immediatep(a) || stringp(a)) {
         struct stat st;
         int fd;
         if (b != IFALSE)
            fd = _open((char*)a + W, O_BINARY, O_RDONLY);
         else
            fd = immval(a);

         if (fstat(fd, &st) == 0) {
            word lst = INULL;
            lst = cons(onum(0, 1), lst);
            lst = cons(onum(0, 1), lst);
            lst = cons(onum(st.st_ctime, 1), lst);
            lst = cons(onum(st.st_mtime, 1), lst);
            lst = cons(onum(st.st_atime, 1), lst);
            lst = cons(onum(st.st_size, 1), lst);
            lst = cons(onum(st.st_rdev, 0), lst);
            lst = cons(onum(st.st_gid, 0), lst);
            lst = cons(onum(st.st_uid, 0), lst);
            lst = cons(onum(st.st_nlink, 0), lst);
            lst = cons(onum(st.st_mode, 0), lst);
            lst = cons(onum(st.st_ino, 0), lst);
            lst = cons(onum(st.st_dev, 1), lst);
            if (b != IFALSE)
               close(fd);
            return lst;
         }
      }
      return INULL;
   case 39: /* chmod fd|path mode follow → bool */
      /* if ((immediatep(a) || stringp(a)) && fixnump(b)) { */
      /*    mode_t mod = immval(b); */
      /*    int flg = c != IFALSE ? 0 : AT_SYMLINK_NOFOLLOW; */
      /*    if ((allocp(a) ? fchmodat(AT_FDCWD, (char *)a + W, mod, flg) : fchmod(immval(a), mod)) == 0) */
      /*       return ITRUE; */
      /* } */
      not_implemented("chmod", "i'm too lazy sorry");
      return IFALSE;
   case 40: /* chown fd|path (uid . gid) follow → bool */
      /* if ((immediatep(a) || stringp(a)) && pairp(b)) { */
      /*    uid_t uid = cnum(G(b, 1)); */
      /*    gid_t gid = cnum(G(b, 2)); */
      /*    int flg = c != IFALSE ? 0 : AT_SYMLINK_NOFOLLOW; */
      /*    if ((allocp(a) ? fchownat(AT_FDCWD, (char *)a + W, uid, gid, flg) : fchown(immval(a), uid, gid)) == 0) */
      /*       return ITRUE; */
      /* } */
      not_implemented("chown", "i'm too lazy sorry");
      return IFALSE;
   case 41: { /* peek mem nbytes → num */
      const word p = cnum(a);
      return onum(
                  b == F(1) ? *(uint8_t *)p :
                  b == F(2) ? *(uint16_t *)p :
                  b == F(4) ? *(uint32_t *)p :
                  b == F(8) ? *(uint64_t *)p :
                  V(p), 0);
   }
   case 42: /* write fd data len | #f → nbytes | #f */
      if (is_type(a, TPORT) && allocp(b)) {
         long long len, size = payl_len(header(b));
         len = c != IFALSE ? cnum(c) : size;
         if (len <= size) {
            len = w32write(immval(a), (void*)(((const word *)b) + 1), len);
            if (len < 0) {
               wsaerrno = WSAGetLastError();
               return IFALSE;
            }
            if (len != (size_t)-1)
               return onum(len, 0);
         }
      }
      return IFALSE;
   case 43:
      return do_poll(a, b, c);
   case 44:  {
      char *host = (char *) (word *) a + W;
      struct addrinfo *res;
      int nth = immval(b);
      int rv = getaddrinfo(host, NULL, NULL, &res);
      if (rv == 0) {
         word rv = IFALSE;
         while (nth--) {
            if (res)
               res = res->ai_next;
            if (!res)
               return INULL;
         }
         if (res->ai_addr->sa_family == AF_INET6) {
            char *n = (char *) &((struct sockaddr_in6*)res->ai_addr)->sin6_addr;
            rv = mkraw(TBVEC, 6);
            memcpy((word *)rv + 1, n, 6);
         } else if (res->ai_addr->sa_family == AF_INET) {
            char *n = (char *) &((struct sockaddr_in*)res->ai_addr)->sin_addr;
            rv = mkraw(TBVEC, 4);
            memcpy((word *)rv + 1, n, 4);
         }
         freeaddrinfo(res);
         return rv;
      }
      return IFALSE; }
   case 45: { /* getpid _ _ _*/
      return(onum(getpid(), 0)); }
   case 46: { /* catch-signals (4 8 ...) _ _*/
      word *lst = (word *)a;
      while((word)lst != INULL) {
         signal(immval(lst[1]), catch_signal);
         lst = (word *)lst[2];
      }
      return ITRUE;
   }
   case 47:
      return isatty(immval(a)) ? ITRUE : IFALSE;
   default:
#ifdef PRIM_CUSTOM
      return prim_custom(immval(op), a, b, c);
#else
      return IFALSE;
#endif
   }
}

word prim_lraw(word wptr, word type) {
   word *lst = (word *)wptr;
   byte *pos;
   word *ob, raw;
   uint len = 0;
   for (ob = lst; pairp(ob); ob = (word *)ob[2])
      len++;
   if ((word)ob != INULL || len > MAXPAYL)
      return IFALSE;
   raw = mkraw(immval(type), len);
   pos = (byte *)raw + W;
   for (ob = lst; (word)ob != INULL; ob = (word *)ob[2])
      *pos++ = immval(ob[1]);
   return raw;
}

/* this shouldn't work */
word
do_poll_fds(word a, word b, word c)
{
   word *cur;
   // puts("poll_fds");

   // return cons(mkport(1), F(2));
   for (cur = (word *)a; (word)cur != INULL; cur = (word *)cur[2]) {
      int fd = immval(G(cur[1], 1));
      if (!issocket(fd))
         return cons(make_immediate(0, TPORT), F(1));
   }
   for (cur = (word *)b; (word)cur != INULL; cur = (word *)cur[2]) {
      int fd = immval(G(cur[1], 1));
      if (!issocket(fd))
         return cons(make_immediate(1, TPORT), F(2));
   }
   return cons(make_immediate(2, TPORT), F(3));
   /*
   HANDLE hs[llen((word*)a) + llen((word*)b)];
   int n = 0, res;
   word *cur;

   for (cur = (word *)a; (word)cur != INULL; cur = (word*)cur[2]) {
      int fd = immval(G(cur[1], 1));
      if (!issocket(fd))
         if (fd > 2)
            hs[n++] = (HANDLE)_get_osfhandle(fd);
   }
   for (cur = (word *)b; (word)cur != INULL; cur = (word*)cur[2]) {
      int fd = immval(G(cur[1], 1));
      if (!issocket(fd))
         if (fd > 2)
            hs[n++] = (HANDLE)_get_osfhandle(fd);
   }

   printf("n = %d, fd = %d\n", n, immval(G(((word*)b)[1], 1)));

   res = WaitForMultipleObjects(n, hs, 0, c == IFALSE ? 0 : immval(c));

   if (res == WAIT_FAILED) {
      return cons(IFALSE, ITRUE);
   } if (res == WAIT_TIMEOUT) {
      return cons(IFALSE, IFALSE);
   } else {
      printf("wait ok %ld\n", res-WAIT_OBJECT_0);
      return cons(mkport(_open_osfhandle((intptr_t)(res-WAIT_OBJECT_0), _O_RDWR)), F(1));
   }
   */
}

word
do_poll_sockets(word a, word b, word c)
{
   fd_set rs, ws, es;
   word *cur;
   hval r1, r2;
   int nfds = -1;
   struct timeval tv;
   int res;

   // puts("do_poll_sockets");

   FD_ZERO(&rs); FD_ZERO(&ws); FD_ZERO(&es);
   for (cur = (word *)a; (word)cur != INULL; cur = (word *)cur[2]) {
      int fd = immval(G(cur[1], 1));
      if (issocket(fd)) {
         FD_SET(fd, &rs);
         FD_SET(fd, &es);
         if (fd >= nfds)
            nfds = fd + 1;
      } else {
         return do_poll_fds(a, b, c);
      }
   }
   for (cur = (word *)b; (word)cur != INULL; cur = (word *)cur[2]) {
      int fd = immval(G(cur[1], 1));
      if (issocket(fd)) {
         FD_SET(fd, &ws);
         FD_SET(fd, &es);
         if (fd >= nfds)
            nfds = fd + 1;
      } else {
         return do_poll_fds(a, b, c);
      }
   }
   if (c == IFALSE) {
      res = select(nfds, &rs, &ws, &es, NULL);
   } else {
      hval ms = immval(c);
      tv.tv_sec = ms/1000;
      tv.tv_usec = (ms%1000)*1000;
      res = select(nfds, &rs, &ws, &es, &tv);
   }

   if (res < 1) {
      r1 = IFALSE; r2 = BOOL(res < 0); /* 0 = timeout, otherwise error or signal */
   } else {
      int fd; /* something active, wake the first thing */
      for (fd = 0; ; ++fd) {
         if (FD_ISSET(fd, &rs)) {
            r1 = make_immediate(fd, TPORT); r2 = F(1); break;
         } else if (FD_ISSET(fd, &ws)) {
            r1 = make_immediate(fd, TPORT); r2 = F(2); break;
         } else if (FD_ISSET(fd, &es)) {
            r1 = make_immediate(fd, TPORT); r2 = F(3); break;
         }
      }
   }
   return cons(r1, r2);
}


/* a = rs, b = ws, c = timeout */
word do_poll(word a, word b, word c) {
   if (llen((word*)a) < 1 && llen((word*)b) < 1)
      return do_poll_sockets(a, b, c);

   if (llen((word*)a) > 0) {
      if (issocket(immval(G(((word*)a)[1], 1))))
         return do_poll_sockets(a, b, c);
   }
   if (llen((word*)b) > 0) {
      if (issocket(immval(G(((word*)b)[1], 1))))
         return do_poll_sockets(a, b, c);
   }

   return do_poll_fds(a, b, c);
}

word vm(word *ob, word arg) {
   byte *ip;
   uint bank = 0;
   uint ticker = TICKS;
   unsigned short acc;
   uint op;
   word R[NR];

   const uint16_t load_imms[] = { F(0), INULL, ITRUE, IFALSE };

   /* clear blank regs */
   for (acc = 1; acc != NR; ++acc)
      R[acc] = INULL;
   R[0] = IFALSE;
   R[3] = IHALT;
   R[4] = arg;
   acc = 2; /* boot always calls with 2 args */

 apply: /* apply something at ob to values in regs, or maybe switch context */

   if (allocp(ob)) {
      hval hdr = *ob & 4095; /* cut size out, take just header info */
      if (hdr == make_header(0, TPROC)) { /* proc */
         R[1] = (word) ob; ob = (word *) ob[1];
      } else if (hdr == make_header(0, TCLOS)) { /* clos */
         R[1] = (word) ob; ob = (word *) ob[1];
         R[2] = (word) ob; ob = (word *) ob[1];
      } else if (!is_type(hdr, TBYTECODE)) { /* not even code */
         error(259, ob, INULL);
      }
      if (!ticker--)
         goto switch_thread;
      ip = (byte *)ob + W;
      goto invoke;
   } else if ((word)ob == IHALT) {
      /* it's the final continuation */
      ob = (word *) R[0];
      if (allocp(ob)) {
         R[0] = IFALSE;
         signals = 0;
         R[4] = R[3];
         R[3] = F(2);
         R[5] = IFALSE;
         R[6] = IFALSE;
         ticker = FMAX;
         bank = 0;
         acc = 4;
         goto apply;
      }
      if (acc == 2) /* update state when main program exits with 2 values */
         state = R[4];
      return R[3];
   } else {
      word *state;
      allocate(acc+1, state);
      *state = make_header(acc+1, TTUPLE);
      memcpy(state + 1, R + 3, acc * W); /* first arg at R3 */
      error(0, ob, state); /* not callable */
   }

 switch_thread: /* enter mcp if present */
   if (R[0] == IFALSE) { /* no mcp, ignore */
      ticker = TICKS;
      goto apply;
   } else {
      /* save vm state and enter mcp cont at R0 */
      word *state;
      ticker = FMAX;
      bank = 0;
      acc = acc + 4;
      R[acc] = (word) ob;
      allocate(acc, state);
      *state = make_header(acc, TTHREAD);
      state[acc-1] = R[acc];
      memcpy(state + 1, R + 1, (acc - 2) * W);
      ob = (word *) R[0];
      R[0] = IFALSE; /* remove mcp cont */
      /* R3 marks the syscall to perform */
      R[3] = signals ? F(10) : F(1);
      R[4] = (word) state;
      R[5] = F(signals);
      R[6] = IFALSE;
      acc = 4;
      signals = 0;
      goto apply;
   }
 invoke: /* nargs and regs ready, maybe gc and execute ob */
   if (((word)fp) + 1024*64 >= ((word) memend)) {
      *fp = make_header(NR + 2, 50); /* hdr r_0 .. r_(NR-1) ob */
      memcpy(fp + 1, R, NR * W);
      fp[NR + 1] = (word)ob;
      fp = gc(1024*64, fp);
      ob = (word *)fp[NR + 1];
      memcpy(R, fp + 1, NR * W);
      ip = (byte *)ob + W;
   }

   for (;;) {
      op = *ip++;
      /* main dispatch */
      switch (op & 63) {
      case 0:
         op = *ip << 8 | ip[1];
         goto super_dispatch;
      case 1:
         A2 = G(A0, ip[1]);
         NEXT(3);
      case 2: /* jmp-nargs a hi lo */
         if (acc != *ip)
            ip += ip[1] << 8 | ip[2];
         NEXT(3);
      case 3: /* goto */
         ob = (word *)A0;
         acc = ip[1];
         goto apply;
      case 5: /* mov2 from1 to1 from2 to2 */
         A1 = A0;
         A3 = A2;
         NEXT(4);
      case 6: { /* opcodes 134, 6, 198, 70 */
         word size = *ip++, tmp;
         word *ob;
         tmp = R[op & 64 ? 1 : *ip++];
         allocate(size, ob);
         *ob = make_header(size, (op >> 7) + TPROC);
         ob[1] = G(tmp, *ip++);
         for (tmp = 2; tmp != size; ++tmp)
            ob[tmp] = R[*ip++];
         R[*ip++] = (word)ob;
         NEXT(0); }
      case 7: /* eq? a b r */
         A2 = BOOL(A0 == A1);
         NEXT(3);
      case 8: /* jeq a b o, extended jump */
         if (A0 == A1)
            ip += ip[3] << 8 | ip[2];
         NEXT(4);
      case 9:
         A1 = A0;
         NEXT(2);
      case 10: /* ldfix n to, encoding: nnnnnnnn nsoooooo (num-1/sign/op) */
         A1 = ((hval)*ip << 9) + (op << 1) + F(1) - 20;
         NEXT(2);
      case 13: /* ldi{2bit what} [to] */
         A0 = load_imms[op >> 6];
         NEXT(1);
      case 15: { /* type o r */
         word ob = A0;
         if (allocp(ob))
            ob = V(ob);
         A1 = F((hval)ob >> TPOS & 63);
         NEXT(2); }
      case 16: /* jeqi[which] a lo hi */ /* FIXME: move this to op4 after fasl update, and encode offset in big-endian (like most other jump instructions) */
         if (A0 == load_imms[op >> 6])
            ip += ip[2] << 8 | ip[1];
         NEXT(3);
      case 17: { /* fxbits num mask pos r, unsafe */
         /* r = (num & (mask << pos)) >> pos */
         int num = immval(A0);
         int mask = immval(A1);
         int pos = immval(A2);
         A3 = F((num & (mask << pos)) >> pos);
         NEXT(4); }
      case 18: /* fxand a b r, prechecked */
         A2 = A0 & A1;
         NEXT(3);
      case 21: { /* fx- a b r u, types prechecked, signs ignored */
         hval r = immval(A0) - immval(A1);
         A3 = F(r >> FBITS & 1);
         A2 = F(r);
         NEXT(4); }
      case 22: { /* fx+ a b r o, types prechecked, signs ignored */
         hval r = immval(A0) + immval(A1);
         A3 = F(r >> FBITS);
         A2 = F(r);
         NEXT(4); }
      case 23: { /* mkt t s f1 .. fs r */
         word t = *ip++;
         word s = *ip++ + 1; /* the argument is n-1 to allow making a 256-tuple with 255, and avoid 0-tuples */
         word *ob;
         uint p;
         allocate(s+1, ob); /* s fields + header */
         *ob = make_header(s+1, t);
         for (p = 0; p != s; ++p)
            ob[p + 1] = R[ip[p]];
         R[ip[p]] = (word)ob;
         NEXT(s+1); }
      case 24: /* ret val == implicit call r3 with 1 arg */
         ob = (word *) R[3];
         R[3] = A0;
         acc = 1;
         goto apply;
      case 25: { /* jmp-var-args a hi lo */
         uint needed = *ip;
         if (acc == needed) {
            R[acc + 3] = INULL; /* add empty extra arg list */
         } else if (acc > needed) {
            word tail; /* TODO: no call overflow handling yet */
            for (tail = INULL; acc > needed; --acc)
               tail = cons(R[acc + 2], tail);
            R[acc + 3] = tail;
         } else {
            ip += ip[1] << 8 | ip[2];
         }
         NEXT(3); }
      case 26: { /* fxqr ah al b qh ql r, b != 0, int32 / int16 -> int32, as fixnums */
         uint64_t a = (uint64_t)immval(A0) << FBITS | immval(A1);
         hval b = immval(A2);
         uint64_t q;
         q = a / b;
         A3 = F(q >> FBITS);
         A4 = F(q);
         A5 = F(a - q*b);
         NEXT(6); }
      case 27: /* syscall cont op arg1 arg2 */
         ob = (word *) R[0];
         R[0] = IFALSE;
         R[3] = A1;
         R[4] = A0;
         R[5] = A2;
         R[6] = A3;
         acc = 4;
         if (ticker > 10)
            bank = ticker; /* deposit remaining ticks for return to thread */
         goto apply;
      case 28: { /* sizeb obj to */
         word ob = A0;
         if (immediatep(ob)) {
            A1 = IFALSE;
         } else {
            word hdr = header(ob);
            A1 = rawp(hdr) ? F(payl_len(hdr)) : IFALSE;
         }
         NEXT(2); }
      case 29: /* fxior a b r, prechecked */
         A2 = A0 | A1;
         NEXT(3);
      case 32: { /* bind tuple <n> <r0> .. <rn> */
         word *tuple = (word *) R[*ip++];
         word hdr, pos = 1, n = *ip++ + 1;
         assert(allocp(tuple), tuple, 32);
         hdr = *tuple;
         assert_not(rawp(hdr) || objsize(hdr) != n, tuple, 32);
         while (--n)
            R[*ip++] = tuple[pos++];
         NEXT(0); }
      case 33: /* fxxor a b r, prechecked */
         A2 = A0 ^ (FMAX << IPOS & A1); /* inherit A0's type info */
         NEXT(3);
      case 35: { /* listuple type size lst to */
         uint type = immval(A0);
         hval size = immval(A1) + 1;
         word *lst = (word *)A2;
         word *ob;
         if (size < MAXOBJ) {
            allocate(size, ob);
            A3 = (word) ob;
            *ob++ = make_header(size, type);
            while (--size) {
               assert(pairp(lst), lst, 35);
               *ob++ = lst[1];
               lst = (word *) lst[2];
            }
         } else {
            A3 = IFALSE;
         }
         NEXT(4); }
      case 39: { /* fx* a b l h */
         uint64_t res = (uint64_t)immval(A0) * immval(A1);
         A3 = F(res >> FBITS);
         A2 = F(res);
         NEXT(4); }
      case 41: { /* car a r, or cdr d r */
         word *ob = (word *)A0;
         assert(pairp(ob), ob, op);
         A1 = ob[op >> 6];
         NEXT(2); }
      case 44: /* less a b r */
         A2 = prim_less(A0, A1);
         NEXT(3);
      case 45: /* set t o v r */
         A3 = prim_set(A0, A1, A2);
         NEXT(4);
      case 47: /* ref t o r */
         A2 = prim_ref(A0, A1);
         NEXT(3);
      case 50: { /* run thunk quantum */
         word hdr;
         ob = (word *)A0;
         R[0] = R[3];
         ticker = bank ? bank : immval(A1);
         bank = 0;
         assert(allocp(ob), ob, 50);
         hdr = *ob;
         if (is_type(hdr, TTHREAD)) {
            hval pos = objsize(hdr) - 1;
            word code = ob[pos];
            acc = pos - 3;
            while (--pos)
               R[pos] = ob[pos];
            ip = (byte *)code + W;
         } else {
            /* call a thunk with terminal continuation */
            R[3] = IHALT; /* exit via R0 when the time comes */
            acc = 1;
            goto apply;
         }
         NEXT(0); }
      case 51: /* cons a b r */
         A2 = cons(A0, A1);
         NEXT(3);
      case 57: { /* jmp-var-args a hi lo */
         uint needed = *ip;
         if (acc == needed) {
            R[acc + 3] = INULL; /* add empty extra arg list */
         } else if (acc > needed) {
            word tail; /* TODO: no call overflow handling yet */
            for (tail = INULL; acc > needed; --acc)
               tail = cons(R[acc + 2], tail);
            R[acc + 3] = tail;
         } else {
            error(57,ob,INULL);
         }
         NEXT(1); }
      case 58: { /* fx>> x n hi lo */
         hval x = immval(A0);
         uint n = immval(A1);
         A2 = F(x >> n);
         A3 = F(x << (FBITS - n));
         NEXT(4); }
      case 59: /* lraw lst type r (FIXME: alloc amount testing compiler pass not in place yet) */
         A2 = prim_lraw(A0, A1);
         NEXT(3);
      case 60: /* arity a  */
         if (acc == *ip) {
            NEXT(1);
         } else {
            word *t;
            // [func desired-arity given-arity a0 .. an]
            allocate(acc + 6, t);
            *t = make_header(acc + 6, TTUPLE);
            t[1] = F(*ip);
            t[2] = F(acc);
            memcpy(t + 3, R, (acc + 3) * W);
            error(60, ob, t);
         }
      case 61: { /* arity error */
         word *t;
         allocate(acc + 1, t);
         *t = make_header(acc + 1, TTUPLE);
         memcpy(t + 1, R + 3, acc * W);
         error(61, ob, t);
      }
      case 62: /* set-ticker <val> <to> -> old ticker value */
         /* ponder: it should be possible to remove this, if the only use is to yield control */
         A1 = F(ticker);
         ticker = immval(A0);
         NEXT(2);
      case 63: /* sys-prim op arg1 arg2 arg3 r1 */
         A4 = prim_sys(A0, A1, A2, A3);
         NEXT(5);
      default:
         error(256, F(op), IFALSE); /* report unimplemented opcode */
      }
   }

 super_dispatch: /* run macro instructions */
   switch (op) {
      /*AUTOGENERATED INSTRUCTIONS*/
   default:
      error(258, F(op), ITRUE);
   }
   goto apply;

 invoke_mcp: /* R4-R6 set, set R3=cont and R4=syscall and call mcp */
   ob = (word *) R[0];
   R[0] = IFALSE;
   R[3] = F(3);
   if (allocp(ob)) {
      acc = 4;
      goto apply;
   }
   return 1; /* no mcp to handle error (fail in it?), so nonzero exit */
}

/* Initial FASL image decoding */

word get_nat() {
   word result = 0;
   word new, i;
   do {
      i = *hp++;
      new = result << 7;
      if (result != new >> 7)
         exit(9); /* overflow kills */
      result = new + (i & 127);
   } while (i & 128);
   return result;
}

word *get_field(word *ptrs, int pos) {
   if (0 == *hp) {
      byte type;
      hp++;
      type = *hp++;
      *fp++ = make_immediate(get_nat(), type);
   } else {
      word diff = get_nat();
      if (ptrs != NULL)
         *fp++ = ptrs[pos - diff];
   }
   return fp;
}

word *get_obj(word *ptrs, int me) {
   uint type, size;
   if (ptrs != NULL)
      ptrs[me] = (word)fp;
   switch (*hp++) { /* TODO: adding type information here would reduce fasl and executable size */
   case 1:
      type = *hp++;
      size = get_nat();
      *fp++ = make_header(size+1, type); /* +1 to include header in size */
      while (size--)
         fp = get_field(ptrs, me);
      break;
   case 2: {
      type = *hp++ & 31; /* low 5 bits, the others are pads */
      size = get_nat();
      memcpy((word *)mkraw(type, size) + 1, hp, size);
      hp += size;
      break;
   }
   default:
      exit(42);
   }
   return fp;
}

/* dry run fasl decode - just compute sizes */
void get_obj_metrics(int *rwords, int *rnobjs) {
   int size;
   switch (*hp++) {
   case 1:
      hp++;
      size = get_nat();
      *rnobjs += 1;
      *rwords += size;
      while (size--) {
         if (0 == *hp)
            hp += 2;
         get_nat();
      }
      break;
   case 2:
      hp++;
      size = get_nat();
      *rnobjs += 1;
      *rwords += (W + size + W - 1) / W;
      hp += size;
      break;
   default:
      exit(42);
   }
}

/* count number of objects and measure heap size */
void heap_metrics(int *rwords, int *rnobjs) {
   const byte *hp_start = hp;
   while (*hp != 0)
      get_obj_metrics(rwords, rnobjs);
   hp = hp_start;
}

void read_heap(const char *path) {
   struct stat st;
   off_t pos = 0;
   ssize_t n;
   int fd = _open(path, O_BINARY, O_RDONLY);
   if (fd == -1)
      exit(1);
   if (fstat(fd, &st) != 0)
      exit(2);
   file_heap = realloc(NULL, st.st_size);
   if (file_heap == NULL)
      exit(3);
   do {
      n = read(fd, file_heap + pos, st.st_size - pos);
      if (n == -1)
         exit(4);
   } while (n && (pos += n) < st.st_size);
   close(fd);
}

/* find a fasl image source to *hp or exit */
void find_heap(int *nargs, char ***argv, int *nobjs, int *nwords) {
   file_heap = NULL;
   if ((word)heap == 0) {
      /* if no preloaded heap, try to load it from first vm arg */
      if (*nargs < 2)
         exit(1);
      read_heap(argv[0][1]);
      ++*argv;
      --*nargs;
      hp = file_heap;
      if (*hp == '#')
         while (*hp++ != '\n');
   } else {
      hp = heap; /* builtin heap */
   }
   heap_metrics(nwords, nobjs);
}

word *decode_fasl(uint nobjs) {
   word *ptrs;
   word *entry;
   uint pos;
   allocate(nobjs + 1, ptrs);
   for (pos = 0; pos != nobjs; ++pos) {
      if (fp >= memend) /* bug */
         exit(1);
      fp = get_obj(ptrs, pos);
   }
   entry = (word *) ptrs[pos - 1];
   *ptrs = make_header(nobjs + 1, 0) | RAWBIT;
   return entry;
}

word *load_heap(uint nobjs) {
   word *entry = decode_fasl(nobjs);
   if (file_heap != NULL)
      free(file_heap);
   return entry;
}

void setup(int nwords, int nobjs) {
   not_implemented("tcgetattr", "no termios");
   state = IFALSE;
   signals = 0;
   max_heap_mb = W == 4 ? 4096 : 65535;
   nwords += nobjs + INITCELLS;
   memstart = genstart = fp = realloc(NULL, (nwords + MEMPAD) * W);
   if (memstart == NULL)
      exit(4);
   memend = memstart + nwords - MEMPAD;
}

int main(int nargs, char **argv) {
   word *prog;
   int rval, nobjs=0, nwords=0;

   if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      return 21;

   find_heap(&nargs, &argv, &nobjs, &nwords);
   setup(nwords, nobjs);
   prog = load_heap(nobjs);
   rval = vm(prog, onum((uintptr_t)argv, 0));
   setdown();
   if (fixnump(rval)) {
      int n = immval(rval);
      if (!(n & ~127))
         return n;
   }
   return 127;
}

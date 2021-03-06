#ifndef _CVORB_USER_DEFINED_DRVR_H_INCLUDE_
#define _CVORB_USER_DEFINED_DRVR_H_INCLUDE_

#include "CvorbDrvr.h"
#include <ad9516o-drvr.h>

#ifdef __LYNXOS
#include <dg/port_ops_lynx.h>
#elif defined(__linux__) && defined(__KERNEL__)
#include <dg/port_ops_linux.h>
#endif /* __LYNXOS */

/**
 * @brief PLL configuration structure
 *
 * @a        -- [0-63]           PLL parameter
 * @b        -- 13 bits [0-8191] PLL parameter
 * @p        -- 16 or 32         PLL parameter
 * @r        --                  PLL parameter
 * @dvco     -- [1,6]  non-PLL divider
 * @d1       -- [1,32] first divider of the output.  non-PLL divider
 * @d2       -- [1,32] second divider of the output. non-PLL divider
 * @force    -- set to 1 to even apply this configuration when there's another
 *              channel currently playing a waveform. 0 to avoid the update in
 *              that case.
 * @external -- set to 0 to use the internal PLL; 1 to use EXTCLK
 */
struct pll {
        int a;
        int b;
        int p;
        int r;
        int dvco;
        int d1;
        int d2;
        int force;
        int external;
};

/* for kernel only */
#if defined(__LYNXOS) || defined(__KERNEL__)
struct sel; /* preliminary structure declaration to supress warnings during
	       user code compilation */

/* enable/disable i/o access debugging printout */
//#define __DEBUG

typedef CVORBBlock00_t mod;
typedef CVORBBlock01_t chd;
/**
 * @brief Description of each module
 *
 * Each module consists of 8 channels, and has general registers.
 * md  -- generic module registers
 * cd  -- channel registers
 * iol -- r/w protection for [SRAM start address] and [SRAM data] registers
 */
struct cvorb_module {
	mod *md;
	chd *cd[8];
	struct cdcm_mutex iol;
};

/* user-defined statics data table for CVORB module */
struct CVORBUserStatics_t {
	struct cvorb_module *md;
        struct cdcm_mutex clkgen_lock;
        struct cdcm_mutex pll_lock;
	struct pll pll;
};

/** @defgroup Low level r/w operations
 *
 *@{
 */
/* read register (_rr), read channel register (_rcr),
   read channel register repetitive (_rcrr), repetitive read register (_rrr)
   m -- module [0, 1]
   c -- channel [0 - 7]
   r -- register to read
   b -- buffer to put results into
   t -- times to read (counter)
 */
#ifdef __DEBUG

#define _rcr(m, c, r)							\
	({								\
		kkprintf("%s()@%d: Read [%s@0x%lx] register @module#%d channel#%d\n", \
			 __FUNCTION__, __LINE__, #r, (long)&(usp->md[m].cd[c]->r), m, c); \
		__inl((__port_addr_t)&(usp->md[m].cd[c]->r));		\
	})

#define _rr(m, r)							\
	({								\
		kkprintf("%s()@%d: Read [%s@0x%lx] register @module#%d\n", \
			 __FUNCTION__, __LINE__, #r, (long)&(usp->md[m].md->r), m); \
		__inl((__port_addr_t)&(usp->md[m].md->r));		\
	})

#define _rcrr(m, c, r, b, t)						\
	({								\
		kkprintf("%s()@%d: Repetitive read (%d times) [%s@0x%lx] register @module#%d" \
			 " channel#%d to buffer 0x%lx\n", __FUNCTION__, __LINE__, t, #r, \
			 (long)&(usp->md[m].cd[c]->r), m, c, (long)b);	\
		__rep_inl((__port_addr_t)&(usp->md[m].cd[c]->r), b, t);	\
	})

#define _rrr(m, r, b, t)						\
	({								\
		kkprintf("%s()@%d: Repetitive read (%d times) [%s@0x%lx] register @module#%d" \
			 " to buffer 0x%lx\n", __FUNCTION__, __LINE__, t, #r, (long)&(usp->md[m].md->r), m, (long)b); \
		__rep_inl((__port_addr_t)&(usp->md[m].md->r), b, t);	\
	})

#else

#define _rcr(m, c, r)						\
	({							\
		__inl((__port_addr_t)&(usp->md[m].cd[c]->r));	\
	})

#define _rr(m, r)							\
	({								\
		__inl((__port_addr_t)&(usp->md[m].md->r));		\
	})

#define _rcrr(m, c, r, b, t)						\
	({								\
		__rep_inl((__port_addr_t)&(usp->md[m].cd[c]->r), b, t); \
	})

#define _rrr(m, r, b, t)						\
	({								\
		__rep_inl((__port_addr_t)&(usp->md[m].md->r), b, t);	\
	})

#endif

/* write register (_wr),  write channel register (_wcr),
   write channel register repetitive (_wcrr), write register repetitive (_wrr)
   m -- module [0, 1]
   c -- channel [0 - 7]
   r -- register to write
   v -- value to write into hw
   b -- buffer to get data from
   t -- times to write (counter)
 */
#ifdef __DEBUG

#define _wcr(m, c, r, v)						\
	({								\
		kkprintf("%s()@%d: Write 0x%x into [%s@0x%lx] register @module#%d channel#%d\n", \
			 __FUNCTION__, __LINE__, v, #r, (long)&(usp->md[m].cd[c]->r), m, c); \
		__outl((__port_addr_t)&(usp->md[m].cd[c]->r), v);	\
	})

#define _wr(m, r, v)							\
	({								\
		kkprintf("%s()@%d: Write 0x%x into [%s@0x%lx] register @module#%d\n", \
			 __FUNCTION__, __LINE__, v, #r, (long)&(usp->md[m].md->r), m); \
		__outl((__port_addr_t)&(usp->md[m].md->r), v);		\
	})

#define _wcrr(m, c, r, b, t)						\
	({								\
		kkprintf("%s()@%d: Repetitive write (%d times) [%s0x%lx] register @module#%d" \
			 " channel#%d from buffer 0x%lx\n", __FUNCTION__, __LINE__, t, #r, \
			 (long)&(usp->md[m].cd[c]->r), m, c, (long)b);	\
		__rep_outl((__port_addr_t)&(usp->md[m].cd[c]->r), b, t); \
	})

#define _wrr(m, r, b, t)						\
	({								\
		kkprintf("%s()@%d: Repetitive write (%d times) [%s@0x%lx] register @module#%d"	\
			 " from buffer 0x%lx\n", __FUNCTION__, __LINE__, t, #r, (long)&(usp->md[m].md->r), m, (long)b); \
		__rep_outl((__port_addr_t)&(usp->md[m].md->r), b, t);	\
	})

#else

#define _wcr(m, c, r, v)						\
	({								\
		__outl((__port_addr_t)&(usp->md[m].cd[c]->r), v);	\
	})

#define _wr(m, r, v)						\
	({							\
		__outl((__port_addr_t)&(usp->md[m].md->r), v);	\
	})

#define _wcrr(m, c, r, b, t)						\
	({								\
		__rep_outl((__port_addr_t)&(usp->md[m].cd[c]->r), b, t); \
	})

#define _wrr(m, r, b, t)						\
	({								\
		__rep_outl((__port_addr_t)&(usp->md[m].md->r), b, t);	\
	})

#endif
/*@} end of group*/

int CvorbUserOpen(int*, register CVORBStatics_t*, int, struct file*);
int CvorbUserClose(int*, register CVORBStatics_t*, struct file*);
int CvorbUserRead(int*, register CVORBStatics_t*, struct file*, char*, int);
int CvorbUserWrite(int*, register CVORBStatics_t*, struct file*, char*, int);
int CvorbUserSelect(int*, register CVORBStatics_t*, struct file*, int, struct sel*);
int CvorbUserIoctl(int*, register CVORBStatics_t*, struct file*, int, int, char*);
char* CvorbUserInst(int*, register DevInfo_t*, register CVORBStatics_t*);
int CvorbUserUnInst(int*, CVORBStatics_t*);
#endif /* __LYNXOS || __KERNEL__ */

#endif /* _CVORB_USER_DEFINED_DRVR_H_INCLUDE_ */

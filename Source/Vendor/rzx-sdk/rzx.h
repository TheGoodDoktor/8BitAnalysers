/*
                                    R  Z  X

             Input Recording Library for ZX Spectrum emulators
      =================================================================
            Library version: 0.12 - last updated: 4 August 2002

      <license goes here>
*/


#ifndef RZX_LIBRARY_INCLUDE
#define RZX_LIBRARY_INCLUDE
#ifdef __cplusplus
extern "C" {
#endif


#define RZX_LIBRARY_VERSION   0x000C
#define RZX_LIBRARY_BUILD     31


/* ******************** Configuration options ******************** */

/* If needed, please edit the data types definitions as requested  */
typedef unsigned char rzx_u8;           /* must be unsigned 8-bit  */
typedef unsigned short int rzx_u16;     /* must be unsigned 16-bit */
typedef unsigned long int rzx_u32;      /* must be unsigned 32-bit */

/* Uncomment the next line for Motorola-byte-order CPUs            */
/* #define RZX_BIG_ENDIAN */

/* Uncomment the next line to enable compression support           */
#define RZX_USE_COMPRESSION

/* Uncomment this to enable some exports for debugging             */
#define RZX_DEBUG

/* *************************************************************** */


/* RZX error codes */
#define RZX_OK           0
#define RZX_NOTFOUND    -1
#define RZX_INVALID     -2
#define RZX_FINISHED    -3
#define RZX_UNSUPPORTED -4
#define RZX_NOMEMORY    -5
#define RZX_SYNCLOST    -6

/* RZX operation mode */
#define RZX_IDLE     0
#define RZX_PLAYBACK 1
#define RZX_RECORD   2


/* RZX callback messages */
#define RZXMSG_CREATOR   1
#define RZXMSG_LOADSNAP  2
#define RZXMSG_IRBNOTIFY 3


/* RZX global flags */
#define RZX_PROTECTED     0x0001
#define RZX_SEALED        0x0002
#define RZX_REMOVE        0x0004
#define RZX_EXTERNAL      0x0008
#define RZX_COMPRESSED    0x0010


/* RZX data types */
typedef rzx_u32 (*RZX_CALLBACK)(int msg, void *param);

typedef struct
{
   int mode;
   char filename[260];
   rzx_u8 ver_major;
   rzx_u8 ver_minor;
   rzx_u32 options;
} RZX_INFO;

typedef struct
{
   char name[20];
   rzx_u16 ver_major;
   rzx_u16 ver_minor;
   rzx_u8 *data;
   rzx_u32 length;
   rzx_u32 options;
} RZX_EMULINFO;

typedef struct
{
   char filename[260];
   rzx_u32 length;
   rzx_u32 options;
} RZX_SNAPINFO;

typedef struct
{
   rzx_u32 framecount;
   rzx_u32 tstates;
   rzx_u32 options;
} RZX_IRBINFO;


/* RZX public data structures */
extern RZX_INFO rzx;


/* RZX functions API */
int rzx_init(const RZX_EMULINFO *emul, const RZX_CALLBACK callback);
int rzx_record(const char *filename);
int rzx_playback(const char *filename);
void rzx_close(void);
int rzx_update(rzx_u16 *icount);
void rzx_store_input(rzx_u8 value);
rzx_u8 rzx_get_input(void);

int rzx_add_snapshot(const char *filename, const rzx_u32 flags);
int rzx_add_comment(const char *text, const rzx_u32 flags);


#ifdef RZX_DEBUG
extern rzx_u16 INcount;
extern rzx_u16 INmax;
extern rzx_u8 *inputbuffer;
#endif


#ifdef __cplusplus
}
#endif
#endif

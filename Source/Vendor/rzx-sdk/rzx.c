/*
                                    R  Z  X

             Input Recording Library for ZX Spectrum emulators
      =================================================================
            Library version: 0.12 - last updated: 04 August 2002
                 Created by Ramsoft ZX Spectrum demogroup

      This is free software. Permission to use it in non-commercial and
      commercial products is hereby granted at the terms of the present
      licence:

      ** WORK IN PROGRESS ** (in short: FREE FOR ALL)

      - A link to the official site of the RZX specifications and SDK
        should be provided either in the program executable or into the
        accompanying documentation.
      - The present licence must be not be removed or altered.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rzx.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef RZX_USE_COMPRESSION
#include <zlib.h>
#endif


/* RZX block IDs */
#define RZXBLK_CREATOR    0x10
#define RZXBLK_SESSION    0x11
#define RZXBLK_COMMENT    0x12
#define RZXBLK_SECURITY   0x20
#define RZXBLK_SNAP       0x30
#define RZXBLK_DATA       0x80

/* RZX internal status codes */
#define RZX_INIT   0x0001
#define RZX_IRB    0x0002
#define RZX_PROT   0x0004
#define RZX_PACK   0x0008

RZX_INFO rzx;


/* ======================================================================== */


#define LLO(x) (rzx_u8)(x&0xFF)
#define LHI(x) (rzx_u8)((x&0xFF00)>>8)
#define HLO(x) (rzx_u8)((x&0xFF0000)>>16)
#define HHI(x) (rzx_u8)((x&0xFF000000)>>24)

#define RZXBLKBUF 512
#define RZXINPUTMAX 32768


static int rzx_status=0;
static FILE *rzxfile=NULL;
static RZX_CALLBACK emul_handler=0;
static RZX_EMULINFO host_emul;
static RZX_EMULINFO file_emul;
static rzx_u8 *file_emul_data=0;
static RZX_SNAPINFO rzx_snap;
static RZX_IRBINFO rzx_irb;

#ifdef RZX_USE_COMPRESSION
/* Compression functions and data structures */
#define ZBUFLEN 16384
static z_stream zs;
static rzx_u8 *zbuf=0;
static int zmode=0;
static rzx_u32 packed_bytes=0;


int rzx_pwrite(rzx_u8 *buffer, int len)
{
 int err;
 zs.avail_in=len;
 zs.next_in=buffer;
 while(zs.avail_in>0)
 {
  if(zs.avail_out==0)
  {
    zs.next_out=zbuf;
    err=fwrite(zbuf,1,ZBUFLEN,rzxfile);
    packed_bytes+=err;
    zs.avail_out=ZBUFLEN;
  }
   err=deflate(&zs,Z_NO_FLUSH);
 }
 return len-zs.avail_in;
}


int rzx_pread(rzx_u8 *buffer, int len)
{
 zs.next_out=buffer;
 zs.avail_out=len;
 while(zs.avail_out>0)
 {
  if(zs.avail_in==0)
  {
   zs.avail_in=fread(zbuf,1,ZBUFLEN,rzxfile);
   if(zs.avail_in==0) return 0;
   zs.next_in=zbuf;
  }
  inflate(&zs,Z_NO_FLUSH);
 }
 return len-zs.avail_out;
}


int rzx_pclose()
{
 int len, done=0,err;
 zs.avail_in=0;
 while(!zmode)
 {
  len=ZBUFLEN-zs.avail_out;
  if(len>0)
  {
   err=fwrite(zbuf,1,len,rzxfile);
   packed_bytes+=err;
   zs.next_out=zbuf;
   zs.avail_out=ZBUFLEN;
  }
  if(done) break;
  err=deflate(&zs,Z_FINISH);
  done=(zs.avail_out>0 || err==Z_STREAM_END);
 }
 if(zbuf!=0) {free(zbuf); zbuf=0;}
 return 0;
}


int rzx_popen(long offset, char *mode)
{
 int err;
 zmode=((mode[0]|0x20)=='r')?1:0;
 memset(&zs,0,sizeof(zs));
 zbuf=(rzx_u8*)malloc(ZBUFLEN);
 if(zmode)
   {
    zs.next_in=zbuf;
    err=inflateInit2(&zs,15);
   }
 else
   {
    err=deflateInit2(&zs,9,Z_DEFLATED,15,9,Z_DEFAULT_STRATEGY);
    zs.next_out=zbuf;
   }
 zs.avail_out=ZBUFLEN;
 packed_bytes=0;
 if(err!=Z_OK) return -1;
 fseek(rzxfile,offset,SEEK_SET);
 return 0;
}
#endif


typedef struct
{
 rzx_u8 type;
 rzx_u32 length;
 long start;
 rzx_u8 buff[RZXBLKBUF];
} RZX_BLKHDR;

static RZX_BLKHDR block;
static rzx_u32 rzx_framecount=0;

#ifdef RZX_DEBUG
rzx_u16 INmax=0;
rzx_u16 INcount=0;
rzx_u8 *inputbuffer=NULL;
#else
static rzx_u16 INmax=0;
static rzx_u16 INcount=0;
static rzx_u8 *inputbuffer=NULL;
#endif
static rzx_u16 INold=0;
static rzx_u8 *oldbuffer=NULL;

int rzx_scan()
{
  long fpos=10;
  memset(&file_emul,0,sizeof(RZX_EMULINFO));
  do
  {
     fseek(rzxfile,fpos,SEEK_SET);
     if(fread(block.buff,5,1,rzxfile)<1) break;
     block.type=block.buff[0];
     block.length=block.buff[1]+256*block.buff[2]+65536*block.buff[3]+16777216*block.buff[4];
     /* printf("rzx_scan: block %02X len=%05i\n",block.type,block.length); */
     if(block.length==0) return RZX_INVALID;
     switch(block.type)
     {
      case RZXBLK_CREATOR:
           /* read the info about the emulator which created this RZX */
           fread(block.buff,24,1,rzxfile);
           strncpy(file_emul.name,block.buff,19);
           file_emul.ver_major=block.buff[20]+256*block.buff[21];
           file_emul.ver_minor=block.buff[22]+256*block.buff[23];
           /* check for custom data */
           file_emul.length=block.length-29;
           if(file_emul.length>0)
           {
            if(file_emul.data!=0) free(file_emul.data);
            file_emul.data=(rzx_u8*)malloc(file_emul.length);
            fread(file_emul.data,file_emul.length,1,rzxfile);
           }
           else file_emul.data=0;
           break;
      case RZXBLK_SECURITY:
           /* signal that the RZX contains at least one encrypted block */
           rzx.options|=RZX_PROTECTED;
           rzx.options|=RZX_SEALED;
           break;
      case RZXBLK_DATA:
           fread(block.buff,13,1,rzxfile);
      default:
           break;
     }
     fpos+=block.length;
  } while(1);
  //if(strlen(file_emul.name)>0)
  fseek(rzxfile,10,SEEK_SET);
  block.start=10;
  block.length=0;
  block.type=0;
  return emul_handler(RZXMSG_CREATOR,&file_emul);
}


void rzx_close_irb()
{
  long pos,len;

  #ifdef RZX_USE_COMPRESSION
  rzx_pclose();
  #endif
  /* suppress empty IRBs */
  if(!rzx_framecount)
  {
    fseek(rzxfile,block.start,SEEK_SET);
    rzx_status&=~RZX_IRB;
    return;
  }
  /* update the IRB header */
  pos=ftell(rzxfile);
  len=pos-block.start;
  block.buff[0]=LLO(len);
  block.buff[1]=LHI(len);
  block.buff[2]=HLO(len);
  block.buff[3]=HHI(len);
  block.buff[4]=LLO(rzx_framecount);
  block.buff[5]=LHI(rzx_framecount);
  block.buff[6]=HLO(rzx_framecount);
  block.buff[7]=HHI(rzx_framecount);
  fseek(rzxfile,block.start+1,SEEK_SET);
  fwrite(block.buff,8,1,rzxfile);
  fseek(rzxfile,pos,SEEK_SET);
  block.start=pos;
  /* signal that we have exited the IRB */
  rzx_status&=~RZX_IRB;
}


int rzx_seek_irb()
{
  int done=0;
  long fpos;
  FILE *snapfile;
  while(!done)
  {
    if(fread(block.buff,5,1,rzxfile)<1) return RZX_FINISHED;
    block.type=block.buff[0];
    #ifndef RZX_BIG_ENDIAN
    block.length=*((rzx_u32*)(&block.buff[1]));
    #else
    block.length=block.buff[1]+256*block.buff[2]+65536*block.buff[3]+16777216*block.buff[4];
    #endif
    if(block.length==0) return RZX_INVALID;
    switch(block.type)
    {
     case RZXBLK_SNAP:
          fread(block.buff,12,1,rzxfile);
          strcpy(rzx_snap.filename,"");
          rzx_snap.options=0x00;
          if(!(block.buff[0]&0x01))
          {
            /* embedded snap */
            #ifdef RZX_USE_COMPRESSION
            if(block.buff[0]&0x02) rzx_snap.options|=RZX_COMPRESSED;
            fpos=ftell(rzxfile);
            rzx_popen(fpos,"rb");
            #endif
            strcpy(rzx_snap.filename,"rzxtemp.");
            strcat(rzx_snap.filename,&block.buff[4]);
            #ifndef RZX_BIG_ENDIAN
            rzx_snap.length=*((rzx_u32*)&block.buff[8]);
            #else
            rzx_snap.length=block.buff[8]+256*block.buff[9]+65536*block.buff[10]+16777216*block.buff[11];
            #endif
            /* extract to tempfile */
            snapfile=fopen(rzx_snap.filename,"wb");
            /* if you can't, skip to next block */
            if(snapfile==NULL) break;
            /* ok */
            rzx_snap.options|=RZX_EXTERNAL|RZX_REMOVE;
            fpos=rzx_snap.length;
            while(fpos>0)
            {
              done=(fpos>RZXBLKBUF)?RZXBLKBUF:fpos;
              #ifdef RZX_USE_COMPRESSION
              if(rzx_snap.options&RZX_COMPRESSED) rzx_pread(block.buff,done);
              else fread(block.buff,done,1,rzxfile);
              #else
              fread(block.buff,done,1,rzxfile);
              #endif
              fwrite(block.buff,done,1,snapfile);
              fpos-=done;
            }
            #ifdef RZX_USE_COMPRESSION
            rzx_pclose();
            #endif
            fclose(snapfile);
            done=0;
          }
          else
          {
            /* external snap, read descriptor */
            fread(&block.buff[12],block.length-17,1,rzxfile);
            strcpy(rzx_snap.filename,&block.buff[16]);
            rzx_snap.options|=RZX_EXTERNAL;
          }
          /* tell the host emulator to load the snapshot */
          emul_handler(RZXMSG_LOADSNAP,&rzx_snap);
          if(rzx_snap.options&RZX_REMOVE) remove(rzx_snap.filename);
          break;
     case RZXBLK_DATA:
          /* recording block found, initialize the values */
          fread(block.buff,13,1,rzxfile);
          #ifndef RZX_BIG_ENDIAN
          rzx_framecount=*((rzx_u32*)&block.buff[0]);
          #else
          rzx_framecount=block.buff[0]+256*block.buff[1]+65536*block.buff[2]+16777216*block.buff[3];
          #endif
          rzx_status|=RZX_IRB;
          if(block.buff[9]&0x01) rzx_status|=RZX_PROT;
          else rzx_status&=~RZX_PROT;
          if(block.buff[9]&0x02) rzx_status|=RZX_PACK;
          else rzx_status&=~RZX_PACK;
          #ifndef RZX_USE_COMPRESSION
          if(rzx_status&RZX_PACK) return RZX_UNSUPPORTED;
          #endif
          rzx_irb.framecount=rzx_framecount;
          rzx_irb.options=0;
          if(rzx_status&RZX_PROT) rzx_irb.options|=RZX_PROTECTED;
          if(rzx_status&RZX_PACK) rzx_irb.options|=RZX_COMPRESSED;
          /* notify the emulator the new parameters */
          emul_handler(RZXMSG_IRBNOTIFY,&rzx_irb);
          #ifdef RZX_USE_COMPRESSION
          if(rzx_status&RZX_PACK)
          {
           fpos=ftell(rzxfile);
           rzx_popen(fpos,"rb");
          }
          #endif
          /* all done */
          return RZX_OK;
          break;
     case RZXBLK_SECURITY:
          /* set the new security parameters */
          break;
     default:
          break;
    }
    /* seek the next block in the file */
    block.start+=block.length;
    fseek(rzxfile,block.start,SEEK_SET);
  }
  return RZX_OK;
}


/* ======================================================================== */


int rzx_init(const RZX_EMULINFO *emul, const RZX_CALLBACK callback)
{
  if((emul==0)||(callback==0)) return RZX_INVALID;
  /* register the host emulator */
  host_emul=(*emul);
  emul_handler=callback;
  rzx.mode=RZX_IDLE;
  rzx.options=0;
  memset(&file_emul,0,sizeof(RZX_EMULINFO));
  rzx_status=RZX_INIT;
  return RZX_OK;
}


int rzx_playback(const char *filename)
{
  if(filename==0) return RZX_INVALID;
  if(inputbuffer==NULL)
  {
   inputbuffer=(rzx_u8*)malloc(RZXINPUTMAX);
   if(inputbuffer==NULL) return RZX_NOMEMORY;
   memset(inputbuffer,0,RZXINPUTMAX);
  }
  if(oldbuffer==NULL)
  {
   oldbuffer=(rzx_u8*)malloc(RZXINPUTMAX);
   if(oldbuffer==NULL) return RZX_NOMEMORY;
   memset(oldbuffer,0,RZXINPUTMAX);
  }
  memset(block.buff,0,RZXBLKBUF);
  rzx.mode=RZX_IDLE;
  rzx_status&=~RZX_IRB;
  rzxfile=fopen(filename,"rb");
  if(rzxfile==NULL) return RZX_NOTFOUND;
  memset(&block,0,16);
  fread(block.buff,6,1,rzxfile);
  if(memcmp(block.buff,"RZX!",4))
  {
     /* not an RZX file */
     rzx_close();
     return RZX_INVALID;
  }
  /* save info about the RZX */
  strcpy(rzx.filename, filename);
  rzx.ver_major=block.buff[4];
  rzx.ver_minor=block.buff[5];
  /* pre-scan the file to collect useful information and stats */
  if(rzx_scan()!=RZX_OK)
  {
     rzx_close();
     return RZX_INVALID;
  }
  /* ok, open the first IRB */
  rzx.mode=RZX_PLAYBACK;
  if(rzx_seek_irb()!=RZX_OK)
  {
     rzx_close();
     return RZX_FINISHED;
  }
  INcount=0;
  INold=0xFFFF;
  return RZX_OK;
}


int rzx_record(const char *filename)
{
  if(filename==0) return RZX_INVALID;
  if(inputbuffer==NULL)
  {
   inputbuffer=(rzx_u8*)malloc(RZXINPUTMAX);
   if(inputbuffer==NULL) return RZX_NOMEMORY;
   memset(inputbuffer,0,RZXINPUTMAX);
  }
  if(oldbuffer==NULL)
  {
   oldbuffer=(rzx_u8*)malloc(RZXINPUTMAX);
   if(oldbuffer==NULL) return RZX_NOMEMORY;
   memset(oldbuffer,0,RZXINPUTMAX);
  }
  memset(block.buff,0,RZXBLKBUF);
  rzx.mode=RZX_IDLE;
  rzxfile=fopen(filename,"wb");
  if(rzxfile==NULL)
  {
    rzx_close();
    return RZX_NOTFOUND;
  }
  strcpy(rzx.filename,filename);
  /* write the main RZX header */
  memcpy(block.buff,"RZX!",4);
  block.buff[4]=LHI(RZX_LIBRARY_VERSION);
  block.buff[5]=LLO(RZX_LIBRARY_VERSION);
  /* add the Creator Information Block */
  block.buff[10]=RZXBLK_CREATOR;
  block.length=29+host_emul.length;
  block.buff[11]=LLO(block.length);
  block.buff[12]=LHI(block.length);
  block.buff[13]=HLO(block.length);
  block.buff[14]=HHI(block.length);
  memcpy(&block.buff[15],&host_emul,24);
  #ifdef RZX_BIG_ENDIAN
  block.buff[25]=LLO(host_emul.ver_major);
  block.buff[26]=LHI(host_emul.ver_major);
  block.buff[27]=LLO(host_emul.ver_minor);
  block.buff[28]=LHI(host_emul.ver_minor);
  #endif
  fwrite(block.buff,10+block.length,1,rzxfile);
  if(host_emul.length>0) fwrite(host_emul.data,host_emul.length,1,rzxfile);
  block.start=10;
  rzx.mode=RZX_RECORD;
  rzx_status&=~RZX_IRB;
  INcount=0;
  INold=0xFFFF;
  return RZX_OK;
}


void rzx_close(void)
{
   switch(rzx.mode)
   {
      case RZX_PLAYBACK:
           rzx_pclose();
           break;
      case RZX_RECORD:
           /* is there an IRB to close? */
           if(rzx_status&RZX_IRB) rzx_close_irb();
           break;
      default:
           break;
   }
   if(rzxfile!=NULL)
   {
      fclose(rzxfile);
      rzxfile=NULL;
   }
   rzx.mode=RZX_IDLE;
   rzx_status=RZX_INIT;
   if(file_emul_data!=0)
   {
      free(file_emul_data);
      file_emul_data=0;
   }
   if(inputbuffer!=NULL)
   {
    free(inputbuffer);
    inputbuffer=NULL;
   }
   if(oldbuffer!=NULL)
   {
    free(oldbuffer);
    oldbuffer=NULL;
   }
}


int rzx_update(rzx_u16 *icount)
{
   rzx_u8 *xchgp;
   long fpos;
   switch(rzx.mode)
   {
    case RZX_PLAYBACK:
         /* check if we are at the beginning */
         if((rzx_status&RZX_IRB)&&(!rzx_framecount)) rzx_status&=~RZX_IRB;
         /* need to seek another IRB? */
         if(!(rzx_status&RZX_IRB))
         {
            if(rzx_seek_irb()!=RZX_OK)
            {
               /* no more IRBs, finished */
               rzx_close();
               return RZX_FINISHED;
            }
         }

         /* fetch the instruction and IN counters */
         INold=INmax;
         #ifdef RZX_USE_COMPRESSION
         if(rzx_status&RZX_PACK) rzx_pread(block.buff,4);
         else fread(block.buff,4,1,rzxfile);
         #else
         fread(block.buff,4,1,rzxfile);
         #endif
         #ifndef RZX_BIG_ENDIAN
         (*icount)=((rzx_u16*)&block.buff)[0];
         INmax=((rzx_u16*)&block.buff)[1];
         #else
         (*icount)=block.buff[0]+256*block.buff[1];
         INmax=block.buff[2]+256*block.buff[3];
         #endif

         /* update the input array */
         #ifdef RZX_USE_COMPRESSION
         if(INmax&&(INmax!=0xFFFF))
         {
          if(rzx_status&RZX_PACK) rzx_pread(inputbuffer,INmax);
          else fread(inputbuffer,INmax,1,rzxfile);
         }
         else INmax=INold;
         #else
         if(INmax&&(INmax!=0xFFFF)) fread(inputbuffer,INmax,1,rzxfile);
         else INmax=INold;
         #endif
         INcount=0;
         rzx_framecount--;
         break;
    case RZX_RECORD:
         /* close if parameters are not valid */
         if((!icount)&&(rzx_status&RZX_IRB))
         {
            rzx_close_irb();
            break;
         }
         /* need to start a new IRB? */
         if(!(rzx_status&RZX_IRB))
         {
            /* ask the emulator about the params */
            memset(&rzx_irb,0,sizeof(RZX_IRBINFO));
            emul_handler(RZXMSG_IRBNOTIFY,&rzx_irb);
            /* fill in the IRB header */
            block.start=ftell(rzxfile);
            memset(block.buff,0,18);
            block.buff[0]=RZXBLK_DATA;
            rzx_status&=~RZX_PACK;
            #ifdef RZX_USE_COMPRESSION
            if(rzx_irb.options&RZX_COMPRESSED)
            {
             block.buff[14]|=0x02;
             rzx_status|=RZX_PACK;
            }
            #endif
            #ifndef RZX_BIG_ENDIAN
            *((rzx_u32*)&block.buff[10])=rzx_irb.tstates;
            #else
            block.buff[10]=LLO(rzx_irb.tstates);
            block.buff[11]=LHI(rzx_irb.tstates);
            block.buff[12]=HLO(rzx_irb.tstates);
            block.buff[13]=HHI(rzx_irb.tstates);
            #endif
            fwrite(block.buff,18,1,rzxfile);
            rzx_status|=RZX_IRB;
            rzx_framecount=0;
            #ifdef RZX_USE_COMPRESSION
            if(rzx_status&RZX_PACK)
            {
             fpos=ftell(rzxfile);
             rzx_popen(fpos,"wb");
            }
            #endif
         }

         /* prepare the frame data */
         if(INold==INcount)
           if(!memcmp(inputbuffer,oldbuffer,INcount)) INcount=0xFFFF;

         #ifndef RZX_BIG_ENDIAN
         ((rzx_u16*)&block.buff)[0]=(*icount);
         ((rzx_u16*)&block.buff)[1]=INcount;
         #else
         block.buff[0]=LLO(*icount);
         block.buff[1]=LHI(*icount);
         block.buff[2]=LLO(INcount);
         block.buff[3]=LHI(INcount);
         #endif
         #ifdef RZX_USE_COMPRESSION
         if(rzx_status&RZX_PACK) rzx_pwrite(block.buff,4);
         else fwrite(block.buff,4,1,rzxfile);
         if(INcount&&(INcount!=0xFFFF))
         {
          if(rzx_status&RZX_PACK) rzx_pwrite(inputbuffer,INcount);
          else fwrite(inputbuffer,INcount,1,rzxfile);
          xchgp=inputbuffer;
          inputbuffer=oldbuffer;
          oldbuffer=xchgp;
          INold=INcount;
         }
         #else
         fwrite(block.buff,4,1,rzxfile);
         if(INcount&&(INcount!=0xFFFF))
         {
          fwrite(inputbuffer,INcount,1,rzxfile);
          xchgp=inputbuffer;
          inputbuffer=oldbuffer;
          oldbuffer=xchgp;
          INold=INcount;
         }
         #endif
         INcount=0;
         rzx_framecount++;
         break;
    default:
         return RZX_INVALID;
         break;
   }
   return RZX_OK;
}

void rzx_store_input(rzx_u8 value)
{
 if(INcount<RZXINPUTMAX) inputbuffer[INcount++]=value;
}


rzx_u8 rzx_get_input(void)
{
 if(INcount>=INmax) return RZX_SYNCLOST;
 return inputbuffer[INcount++];
}

int rzx_add_snapshot(const char *filename, const rzx_u32 flags)
{
 FILE *snapfile;
 long snaplen;
 long fpos=0;
 int blocklen=17,i;
 if(filename==0) return RZX_INVALID;
 snapfile=fopen(filename,"rb");
 if(snapfile==NULL) return RZX_NOTFOUND;
 if(rzx_status&RZX_IRB) rzx_close_irb();
 /* find the length of the snapshot file */
 fseek(snapfile,0,SEEK_END);
 snaplen=ftell(snapfile);
 fseek(snapfile,0,SEEK_SET);
 memset(block.buff,0,RZXBLKBUF);
 /* fill in the structures */
 block.buff[0]=RZXBLK_SNAP;
 if(flags&RZX_EXTERNAL)
 {
   /* leave the snapshot outside the RZX file */
   blocklen+=4+strlen(filename)+1;
   block.buff[5]=0x01;
 }
 else blocklen+=snaplen;
 block.buff[1]=LLO(blocklen);
 block.buff[2]=LHI(blocklen);
 block.buff[3]=HLO(blocklen);
 block.buff[4]=HHI(blocklen);
 /* find the filename extension and store it into the header */
 i=0; while((i<strlen(filename))&&(filename[i]!='.')) i++;
 i++; if(i<strlen(filename)) strncpy(&block.buff[9],&filename[i],3);
 block.buff[13]=LLO(snaplen);
 block.buff[14]=LHI(snaplen);
 block.buff[15]=HLO(snaplen);
 block.buff[16]=HHI(snaplen);
 /* write the snapshot or the descriptor */
 if(flags&RZX_EXTERNAL)
 {
   /* fill in the external descriptor */
   strcpy(&block.buff[21],filename);
   fwrite(block.buff,blocklen,1,rzxfile);
 }
 else
 {
   /* embedded snapshot */
   #ifdef RZX_USE_COMPRESSION
   if(flags&RZX_COMPRESSED) block.buff[5]|=0x02;
   #endif
   fwrite(block.buff,17,1,rzxfile);
   #ifdef RZX_USE_COMPRESSION
   if(flags&RZX_COMPRESSED)
     {
      fpos=ftell(rzxfile);
      rzx_popen(fpos,"wb");
     }
   #endif
   /* copy the snapshot */
   while(snaplen>0)
   {
     i=(snaplen>RZXBLKBUF)?RZXBLKBUF:snaplen;
     fread(block.buff,i,1,snapfile);
     #ifdef RZX_USE_COMPRESSION
     if(flags&RZX_COMPRESSED) rzx_pwrite(block.buff,i);
     else fwrite(block.buff,i,1,rzxfile);
     #else
     fwrite(block.buff,i,1,rzxfile);
     #endif
     snaplen-=i;
   }
   #ifdef RZX_USE_COMPRESSION
   if(flags&RZX_COMPRESSED)
   {
    rzx_pclose();
    i=ftell(rzxfile);
    blocklen=17+i-fpos;
    fseek(rzxfile,fpos-16,SEEK_SET);
    block.buff[1]=LLO(blocklen);
    block.buff[2]=LHI(blocklen);
    block.buff[3]=HLO(blocklen);
    block.buff[4]=HHI(blocklen);
    fwrite(&block.buff[1],4,1,rzxfile);
    fseek(rzxfile,i,SEEK_SET);
   }
   #endif
 }
 fclose(snapfile); snapfile=NULL;
 /* remove the snapshot if requested */
 if(flags&RZX_REMOVE) remove(filename);
 return RZX_OK;
}


int rzx_add_comment(const char *text, const rzx_u32 flags)
{
 if(rzx_status&RZX_IRB) rzx_close_irb();
 /* to do! */
 return RZX_OK;
}


#ifdef __cplusplus
}
#endif


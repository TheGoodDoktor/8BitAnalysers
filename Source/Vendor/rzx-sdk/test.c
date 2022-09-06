/*
 * RZX SDK silly test program by Ramsoft
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rzx.h"

#ifdef _MSC_VER
#define strcasecmp(x,y) stricmp(x,y)
#endif
#ifndef RZX_DEBUG
rzx_u16 INmax=0;
#endif

#define TESTVMAJ 0
#define TESTVMIN 15

#define INBUFSIZE 8192
RZX_EMULINFO emul_info;
rzx_u8 inbuffer[INBUFSIZE];
rzx_u16 icount;
rzx_u32 tstates=555;

rzx_u32 rzx_callback(int msg, void *par)
{
 switch(msg)
 {
  case RZXMSG_LOADSNAP:
       printf("> LOADSNAP: '%s' (%i bytes), %s %s\n",
              ((RZX_SNAPINFO*)par)->filename,
              (int)((RZX_SNAPINFO*)par)->length,
              (((RZX_SNAPINFO*)par)->options&RZX_EXTERNAL)?"external":"embedded",
              (((RZX_SNAPINFO*)par)->options&RZX_COMPRESSED)?"compressed":"uncompressed");
       break;
  case RZXMSG_CREATOR:
       break;
  case RZXMSG_IRBNOTIFY:
       if(rzx.mode==RZX_PLAYBACK)
       {
         /* fetch the IRB info if needed */
         tstates=((RZX_IRBINFO*)par)->tstates;
         printf("> IRB notify: tstates=%i, %s\n",(int)tstates,
                   ((RZX_IRBINFO*)par)->options&RZX_COMPRESSED?"compressed":"uncompressed");
       }
       else if(rzx.mode==RZX_RECORD)
       {
         /* fill in the relevant info, i.e. tstates, options */
         ((RZX_IRBINFO*)par)->tstates=tstates;
         ((RZX_IRBINFO*)par)->options=0;
         #ifdef RZX_USE_COMPRESSION
         ((RZX_IRBINFO*)par)->options|=RZX_COMPRESSED;
         #endif
         printf("> IRB notify: tstates=%i, %s\n",(int)tstates,
                   ((RZX_IRBINFO*)par)->options&RZX_COMPRESSED?"compressed":"uncompressed");
       }
       break;
  default:
       printf("> MSG #%02X\n",msg);
       return RZX_INVALID;
       break;
 }
 return RZX_OK;
}


///////////////////////////////////////////////////////////////////////////


extern rzx_u16 INmax;
extern rzx_u16 INold;

int main(int argc, char *argv[])
{
 int i,j,n;
 char filename[260]="";

 printf("-=[ RZX SDK test v%i.%02i ]=-   RZX library demonstration by Ramsoft\n\n",TESTVMAJ,TESTVMIN);
 printf("Using RZX Library v%i.%02i build %i\n\n",(RZX_LIBRARY_VERSION&0xFF00)>>8,RZX_LIBRARY_VERSION&0xFF,RZX_LIBRARY_BUILD);
 if(argc>1) if(!strcasecmp(argv[1],"-p")) strcpy(filename,argv[2]);
 strcpy(emul_info.name,"RZX SDK test");
 emul_info.ver_major=TESTVMAJ;
 emul_info.ver_minor=TESTVMIN;
 emul_info.data=0; emul_info.length=0;
 emul_info.options=0;
 rzx_init(&emul_info,rzx_callback);

 /* ---------------------------------------------------------------------- */

 if(!strlen(filename))
 {
 strcpy(filename,"test.rzx");
 printf("Recoding:\n");
 if(rzx_record(filename)!=RZX_OK)
 {
   printf("Unable to start recording\n");
   return 0;
 }

 if(rzx_add_snapshot("test.z80",RZX_COMPRESSED)!=RZX_OK)
   printf("Unable to insert snapshot\n");

 for(i=0;i<10;i++)
 {
  icount=100+i;
  for(j=0;j<8;j++)
  {
   if(i!=5) inbuffer[j]=j+i*8;
   else inbuffer[j]=j+32;
   rzx_store_input(inbuffer[j]);
  }
  rzx_update(&icount);
  printf("frame %04i: icount=%05i  input =",i,icount);
  for(j=0;j<8;j++) printf(" %02X",inbuffer[j]);
  printf("\n");

 }
 rzx_close();
 }
 
 /* ---------------------------------------------------------------------- */

 printf("Playback:\n");
 if(rzx_playback(filename)!=RZX_OK)
   {
     printf("Error starting playback\n");
     return 0;
   }
 n=0;
 do
 {
  memset(inbuffer,0,INBUFSIZE);
  i=rzx_update(&icount);
  if(i==RZX_OK)
  {
   for(j=0;j<8;j++) inbuffer[j]=rzx_get_input();
   printf("frame %04i: icount=%05i(%04X) #in=%04i  input =",n,icount,icount,INmax);
   for(j=0;j<8;j++) printf(" %02X",inbuffer[j]);
   printf("\n");
  }
  n++;
 } while(i==RZX_OK);
 rzx_close();
 return 0;
}



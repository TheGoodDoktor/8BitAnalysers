#include "RZXLoader.h"
#include <malloc.h>
#include "Util/FileUtil.h"

#include "rzx.h"


bool FRZXManager::Load(const char* fName)
{
    return false;
}

static RZX_EMULINFO gEmulInfo;

const size_t kInBufferSize = 8192;
rzx_u8 g_InBuffer[kInBufferSize];

static bool g_RZXInitialised = false;

rzx_u32 RZXCallback(int msg, void* par)
{
    switch (msg)
    {
    case RZXMSG_LOADSNAP:
    {
        RZX_SNAPINFO* pSnapInfo = (RZX_SNAPINFO*)par;

        printf("> LOADSNAP: '%s' (%i bytes), %s %s\n",
            pSnapInfo->filename,
            (int)pSnapInfo->length,
            (pSnapInfo->options & RZX_EXTERNAL) ? "external" : "embedded",
            (pSnapInfo->options & RZX_COMPRESSED) ? "compressed" : "uncompressed");
    }
    break;
    
    case RZXMSG_CREATOR:
    {
        RZX_EMULINFO* pInfo = (RZX_EMULINFO*)par;
    }
    break;

    case RZXMSG_IRBNOTIFY:
    {
        RZX_IRBINFO* pIRBInfo = (RZX_IRBINFO*)par;
        if (rzx.mode == RZX_PLAYBACK)
        {
            /* fetch the IRB info if needed */
            int tstates = pIRBInfo->tstates;
            printf("> IRB notify: tstates=%i, %s\n", (int)tstates,
                pIRBInfo->options & RZX_COMPRESSED ? "compressed" : "uncompressed");
        }
        else if (rzx.mode == RZX_RECORD)
        {
            int tstates = 1;
            /* fill in the relevant info, i.e. tstates, options */
            pIRBInfo->tstates = tstates;
            pIRBInfo->options = 0;
#ifdef RZX_USE_COMPRESSION
            pIRBInfo->options |= RZX_COMPRESSED;
#endif
            printf("> IRB notify: tstates=%i, %s\n", (int)tstates,
                pIRBInfo->options & RZX_COMPRESSED ? "compressed" : "uncompressed");
        }
    }
    break;
    
    default:
        printf("> MSG #%02X\n", msg);
        return RZX_INVALID;
        break;
    }
    return RZX_OK;
}

bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName)
{
	if (g_RZXInitialised == false)
	{
        strcpy(gEmulInfo.name, "RZX Loader");
        gEmulInfo.ver_major = 1;
        gEmulInfo.ver_minor = 0;
        gEmulInfo.data = 0; 
        gEmulInfo.length = 0;
        gEmulInfo.options = 0;

        if (rzx_init(&gEmulInfo, RZXCallback) != RZX_OK)
            return false;
	}
	
    if (rzx_playback(fName) != RZX_OK)
    {
        printf("Error starting playback\n");
        return false;
    }

    int n = 0;
    int ret = RZX_OK;
    do
    {
        memset(g_InBuffer, 0, kInBufferSize);
        rzx_u16 icount;
        ret = rzx_update(&icount);
        if (ret == RZX_OK)
        {
            for (int j = 0; j < 8; j++) 
                g_InBuffer[j] = rzx_get_input();
            printf("frame %04i: icount=%05i(%04X) #in=%04i  input =", n, icount, icount, INmax);
            
            for (int j = 0; j < 8; j++) 
                printf(" %02X", g_InBuffer[j]);
            printf("\n");
        }
        n++;
    } while (ret == RZX_OK);
    rzx_close();

	return false;
}
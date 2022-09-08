#include "RZXLoader.h"
#include <malloc.h>
#include "Util/FileUtil.h"
#include "GamesList.h"
#include "Z80Loader.h"
#include "SNALoader.h"

#include "rzx.h"

static FRZXManager* g_pManager = nullptr;

rzx_u32 RZXCallback(int msg, void* param)
{
    return g_pManager->RZXCallbackHandler(msg,param) ? RZX_OK : RZX_INVALID;
}


bool	FRZXManager::Init(FSpectrumEmu* pEmu) 
{ 
    pZXEmulator = pEmu; 

    RZX_EMULINFO emulInfo;
    strcpy(emulInfo.name, "RZX Loader");
    emulInfo.ver_major = 1;
    emulInfo.ver_minor = 0;
    emulInfo.data = 0;
    emulInfo.length = 0;
    emulInfo.options = 0;

    if (rzx_init(&emulInfo, RZXCallback) != RZX_OK)
        return false;

    Initialised = true;
    g_pManager = this;  // crap
    return true;
}

bool FRZXManager::RZXCallbackHandler(int msg, void* param)
{
    switch (msg)
    {
    case RZXMSG_LOADSNAP:
    {
        // This is proper shit, it loads the snapshot into memory and then saves it out to a temp file 
        // So we just load it right back in again
        RZX_SNAPINFO* pSnapInfo = (RZX_SNAPINFO*)param;

        printf("> LOADSNAP: '%s' (%i bytes), %s %s\n",
            pSnapInfo->filename,
            (int)pSnapInfo->length,
            (pSnapInfo->options & RZX_EXTERNAL) ? "external" : "embedded",
            (pSnapInfo->options & RZX_COMPRESSED) ? "compressed" : "uncompressed");

        switch (GetSnapshotTypeFromFileName(pSnapInfo->filename))
        {
        case ESnapshotType::Z80:
            return LoadZ80File(pZXEmulator, pSnapInfo->filename);
        case ESnapshotType::SNA:
            return LoadSNAFile(pZXEmulator, pSnapInfo->filename);
        default: 
            return false;
        }
    }
    break;

    case RZXMSG_CREATOR:
    {
        RZX_EMULINFO* pInfo = (RZX_EMULINFO*)param;
        CurrentRZXInfo.Name = pInfo->name;
    }
    break;

    case RZXMSG_IRBNOTIFY:
    {
        RZX_IRBINFO* pIRBInfo = (RZX_IRBINFO*)param;
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
        return false;
    }
    return true;
}


bool FRZXManager::Load(const char* fName)
{
    if (Initialised == false)
        return false;

    if (rzx_playback(fName) != RZX_OK)
    {
        printf("Error starting playback\n");
        return false;
    }

    ReplayMode = EReplayMode::Playback;

    return true;
}

uint16_t FRZXManager::Update(void)
{
    if (ReplayMode == EReplayMode::Off)
        return 0;

    uint16_t icount;
    int ret = rzx_update(&icount);
    return icount;
}

uint8_t	FRZXManager::GetInput()
{
    return rzx_get_input();
}


static RZX_EMULINFO gEmulInfo;

const size_t kInBufferSize = 8192;
rzx_u8 g_InBuffer[kInBufferSize];

static bool g_RZXInitialised = false;


bool LoadRZXFile(FSpectrumEmu* pEmu, const char* fName)
{
	
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
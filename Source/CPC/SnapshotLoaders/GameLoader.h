#if 0
#include "Misc/GamesList.h"

class FCPCEmu;
enum class ECPCModel;

class FCPCGameLoader : public IGameLoader
{
public:
	// IGameLoader
	bool LoadSnapshot(const FGameSnapshot& snapshot) override;
	// ~IGameLoader

	void Init(FCPCEmu* pEmu, ECPCModel fallbackModel);
	void SetFallbackCPCModel(ECPCModel cpcModel);
	
private:
	FCPCEmu* pCPCEmu = 0;
	
	ECPCModel FallbackCPCModel;
};
#endif


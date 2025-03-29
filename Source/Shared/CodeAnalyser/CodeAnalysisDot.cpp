#include "CodeAnalysisDot.h"
#include "CodeAnalyser.h"

bool ExportCodeAnalysisDot(const FCodeAnalysisState& state, const char* pFilename, bool bROMS)
{
	FILE* fp = fopen(pFilename, "wt");
	if (!fp)
	{
		return false;
	}

	fprintf(fp, "digraph Functions {\n");

	// TODO: output functions as nodes
	
	// TODO: join nodes
	for (const auto functionIt : state.Functions.GetFunctions())
	{
		const FFunctionInfo& function = functionIt.second;
		if (function.bROMFunction == bROMS)
		{
			for(const auto& calledPoint : function.CallPoints)
			{
				const FFunctionInfo* calledFunc = state.Functions.GetFunctionAtAddress(calledPoint.FunctionAddr);
				if(calledFunc)
				{
					fprintf(fp, "%s -> %s\n", function.Name.c_str(), calledFunc->Name.c_str());
				}
			}
		}
	}


	fprintf(fp, "}");
	fclose(fp);

	return true;
}
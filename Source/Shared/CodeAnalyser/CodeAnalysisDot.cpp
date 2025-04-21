#include "CodeAnalysisDot.h"
#include "CodeAnalyser.h"
#include "FunctionAnalyser.h"

bool ExportCodeAnalysisDot(FCodeAnalysisState& state, const char* pFilename, bool bROMS)
{
	FILE* fp = fopen(pFilename, "wt");
	if (!fp)
	{
		return false;
	}

	FFunctionInfoCollection& functions = *state.pFunctions;

	fprintf(fp, "digraph Functions {\n");

	// Set up formatting
	fprintf(fp, "\tfontname=\"Helvetica,Arial,sans-serif\" \n");
	fprintf(fp, "\tnode [fontname=\"Helvetica,Arial,sans-serif\"] \n");
	fprintf(fp, "\tedge [fontname=\"Helvetica,Arial,sans-serif\"] \n");
	fprintf(fp, "\tgraph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"]; \n");
	fprintf(fp, "\tratio = auto; \n");

	// TODO: output functions as nodes
	
	// Join nodes
	for (const auto functionIt : functions.GetFunctions())
	{
		const FFunctionInfo& function = functionIt.second;
		const FLabelInfo* functionLabel = state.GetLabelForAddress(function.StartAddress);

		if (function.bROMFunction == bROMS)
		{
			for(const auto& calledPoint : function.CallPoints)
			{
				const FFunctionInfo* calledFunc = functions.GetFunctionAtAddress(calledPoint.FunctionAddr);
				const FLabelInfo* calledLabel = state.GetLabelForAddress(calledPoint.FunctionAddr);
				if(calledFunc && calledLabel)
				{
					fprintf(fp, "\t%s -> %s\n", functionLabel->GetName(), calledLabel->GetName());
				}
			}
		}
	}


	fprintf(fp, "}");
	fclose(fp);

	return true;
}
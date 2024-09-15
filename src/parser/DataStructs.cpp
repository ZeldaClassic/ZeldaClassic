//2.53 Updated to 16th Jan, 2017
#include <assert.h>
#include <iostream>
#include "zsyssimple.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "LibrarySymbols.h"
#include "Scope.h"
#include "Types.h"
#include "ZScript.h"

using std::cout;
using std::endl;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// FunctionData

FunctionData::FunctionData(Program& program)
	: program(program),
	  globalData(program.getScope().getLocalData())
{
	for (const auto it : globalData)
	{
		Datum& datum = *it;
		if (!datum.getCompileTimeValue())
			globalVariables.push_back(&datum);
	}

	for (const auto script : program.scripts)
	{
		const auto data = script->getScope().getLocalData();
		globalVariables.insert(globalVariables.end(),
		                       data.begin(), data.end());
	}
	
	for (const auto it : program.namespaces)
	{
		const auto data = it->getScope().getLocalData();
		globalVariables.insert(globalVariables.end(),
		                       data.begin(), data.end());
	}
}

////////////////////////////////////////////////////////////////
// IntermediateData

IntermediateData::IntermediateData(FunctionData const& functionData)
	: program(functionData.program)
{}

////////////////////////////////////////////////////////////////
// OpcodeContext

OpcodeContext::OpcodeContext(ZScript::TypeStore* typeStore)
	: typeStore(typeStore)
{}

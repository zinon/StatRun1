//roofit
#include "RooWorkspace.h"
#include "RooAbsData.h"
#include "RooSimultaneous.h"
#include "RooArgSet.h"
#include "RooAbsArg.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooCatType.h"
#include "RooAbsPdf.h"
#include "RooMinimizer.h"
#include "RooFitResult.h"
#include "RooPlot.h"
#include "RooCurve.h"
#include "RooMsgService.h"



#include "RooRealSumPdf.h"
#include "RooProduct.h"

//roostats
#include "RooStats/ModelConfig.h"
#include "RooStats/RooStatsUtils.h" //RemoveConstantParameters etc


using namespace RooFit;

using namespace RooStats;

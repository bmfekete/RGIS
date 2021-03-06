/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDDichRouteMuskCoeff.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRiverAvgDepthMeanID  = MFUnset;
static int _MDInRiverWidthMeanID     = MFUnset;
static int _MDInRiverVelocityMeanID  = MFUnset;
static int _MDInRiverShapeExponentID = MFUnset;
static int _MDInRiverSlopeID         = MFUnset;
// Output
static int _MDOutMuskingumC0ID       = MFUnset;
static int _MDOutMuskingumC1ID       = MFUnset;
static int _MDOutMuskingumC2ID       = MFUnset;
static int _MDOutCourantID           = MFUnset;

static void _MDDischRouteMuskingumCoeff (int itemID) {
// Input
	float yMean;            // Average depth at mean discharge [m]
	float wMean;            // River width at mean discharge [m]
	float vMean;            // Mean velocity
	float beta;             // Riverbed shape exponent []
	float slope;            // Riverbed slope [m/km]
// Output
	float C0;               // Muskingum C0 coefficient (current inflow)
	float C1;               // Muskingum C1 coefficient (previous inflow)
	float C2;               // MUskingum C2 coefficient (previous outflow)
// Local
	float xi;               // Flood-wave/flow velocity ratio
	float C;                // Cell Courant number;
	float D;                // Cell Reynolds number;
	float dt;               // time step length [s]
	float dL;               // Cell length [m]
	
	dL        = MFModelGetLength (itemID);
	yMean     = MFVarGetFloat (_MDInRiverAvgDepthMeanID,  itemID, 0.0);
	wMean     = MFVarGetFloat (_MDInRiverWidthMeanID,     itemID, 0.0);
	vMean     = MFVarGetFloat (_MDInRiverVelocityMeanID,  itemID, 0.0);
	beta      = MFVarGetFloat (_MDInRiverShapeExponentID, itemID, 0.0);
	slope     = MFVarGetFloat (_MDInRiverSlopeID,         itemID, 0.0) / 1000.0;

	if ((dL    <= 0.0) || (slope <= 0.0) || (yMean <= 0.0) || (wMean <= 0.0) || (vMean <= 0.0) || (beta  <= 0.0)) { 
	    // Falling back to flow-accumulation
		C0 = 1.0;
		C1 = C2 = C = 0.0;
	}
	else {
		dt = MFModelGet_dt (); 

		xi = 1 + beta * (2.0 / 3.0) / (beta + 1);
		C = xi * vMean * dt / dL;
		D = yMean / (dL * slope * xi);

		C0 = (-1 + C + D) / (1 + C + D);
		C1 = ( 1 + C - D) / (1 + C + D);
		C2 = ( 1 - C + D) / (1 + C + D);

		// According to Pounce C1 and C2 can be negative, but negative values appear to cause negative discharge
		if ((C0 < 0.0) || (C1 < 0.0) || (C2 < 0.0)) { C0 = 1.0; C1 = 0; C2 = 0; }
	}
	MFVarSetFloat (_MDOutMuskingumC0ID, itemID, C0);
	MFVarSetFloat (_MDOutMuskingumC1ID, itemID, C1);
	MFVarSetFloat (_MDOutMuskingumC2ID, itemID, C2);
	MFVarSetFloat (_MDOutCourantID,     itemID, C);
}

enum { MDinput, MDstatic };

int MDRouting_DischargeInChannelMuskingumCoeffDef () {
	int  optID = MFUnset;
	const char *optStr, *optName = MDOptRouting_Muskingum;
	const char *options [] = { MDInputStr, "static", (char *) NULL };

	if (_MDOutMuskingumC0ID != MFUnset) return (_MDOutMuskingumC0ID);

	MFDefEntering ("Muskingum Coefficients");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	switch (optID) {
		case MDinput:
			if (((_MDOutMuskingumC0ID = MFVarGetID (MDVarRouting_MuskingumC0, MFNoUnit, MFInput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutMuskingumC1ID = MFVarGetID (MDVarRouting_MuskingumC1, MFNoUnit, MFInput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutMuskingumC2ID = MFVarGetID (MDVarRouting_MuskingumC2, MFNoUnit, MFInput, MFState, MFBoundary)) == CMfailed))
				return (CMfailed);
			break;
		case MDstatic:
			if (((_MDInRiverShapeExponentID  = MDRouting_RiverShapeExponentDef()) == CMfailed) ||
                	    ((_MDInRiverWidthMeanID      = MFVarGetID (MDVarRouting_RiverWidthMean,    "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
                	    ((_MDInRiverAvgDepthMeanID   = MFVarGetID (MDVarRouting_RiverAvgDepthMean, "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
                	    ((_MDInRiverVelocityMeanID   = MFVarGetID (MDVarRouting_RiverVelocityMean, "m/s",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
                	    ((_MDInRiverSlopeID          = MFVarGetID (MDVarRouting_RiverSlope,        "m/km",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                	    ((_MDOutMuskingumC0ID        = MFVarGetID (MDVarRouting_MuskingumC0,       MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                	    ((_MDOutMuskingumC1ID        = MFVarGetID (MDVarRouting_MuskingumC1,       MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                	    ((_MDOutMuskingumC2ID        = MFVarGetID (MDVarRouting_MuskingumC2,       MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                	    ((_MDOutCourantID            = MFVarGetID ("Courant",                      MFNoUnit, MFOutput, MFState, MFBoundary)) == CMfailed) ||
                	    (MFModelAddFunction (_MDDischRouteMuskingumCoeff) == CMfailed)) return (CMfailed);
			break;
		default: MFOptionMessage (optName, optStr, options); return (CMfailed);
	}
	MFDefLeaving ("Muskingum Coefficients");
	return (_MDOutMuskingumC0ID);
}

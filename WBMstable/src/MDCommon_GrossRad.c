/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDCommon_GrossRad.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

#define  DTOR 0.01745329252
static float _MDGrossRadStdTAU = 1.0;

static int _MDOutCommon_GrossRadID = MFUnset;

static void _MDCommon_GrossRadianceStd (int itemID) {
// Input
	int   day;
	float lambda;
// Output
	float  grossRad;
// Local
	int   hour;
	double eta,sigma,sinphi,sp,sbb;

	day    = MFDateGetDayOfYear ();
   lambda = MFModelGetLatitude (itemID) * DTOR;

	sp = 1360.0 * 3600.0 * 24.0 * 0.041841 / 41860.0; // FBM  0.041841 conversion from cal/cm2 to MJ/m2
	grossRad = 0;
	sigma = -23.4 * cos (2.0 * M_PI * (day + 11.0) / 365.0) * DTOR;
	for (hour = 0;hour < 24; hour++) {
		eta = (hour + 1) * M_PI / 12.0;
		sinphi = sin (lambda) * sin (sigma) + cos (lambda) * cos (sigma) * cos (eta);
		sbb = sp * sinphi * pow ((double) _MDGrossRadStdTAU,(double) (1.0 / sinphi));
		if (sbb > 0) grossRad += sbb;
	}
	MFVarSetFloat (_MDOutCommon_GrossRadID,  itemID, grossRad / 24.0);
}

static void _MDCommon_GrossRadianceOtto (int itemID) {
// Input
	int   day;
	float lambda;
// Output
	float  grossRad;
// Local
	int   hour;
	double eta, sigma,sinphi,sp,sbb,sotd;

	day    = MFDateGetDayOfYear ();
   lambda = MFModelGetLatitude (itemID) * DTOR;

	sp = 1360.0 * 3600.0 * 24.0 * 0.041841 / 41860.0; // FBM  0.041841 conversion from cal/cm2 to MJ/m2
	grossRad = 0.0;
	sigma = -23.4856 * cos (2.0 * M_PI * (day + 11.0) / 365.25) * DTOR;
	for (hour = 0;hour < 24;hour++) {
		eta = (double) ((hour + 1)) * M_PI / 12.0;
		sinphi = sin (lambda) * sin (sigma) + cos (lambda) * cos (sigma) * cos (eta);
 		sotd = 1 - (0.016729 * cos (0.9856 * (day - 4.0) * DTOR));
		sbb = sp * sinphi / pow (sotd,2.0);
		if (sbb >= 0) grossRad += sbb;
	}
	MFVarSetFloat (_MDOutCommon_GrossRadID,  itemID, grossRad / 24.0);
}

enum { MDinput, MDstandard,  MDOtto }; 

int MDCommon_GrossRadDef () {
	int optID = MFUnset;
	const char *optStr, *optName = MDVarCore_GrossRadiance;
	const char *options [] = { MDInputStr, "standard", "Otto", (char *) NULL };
	float par;

	if (_MDOutCommon_GrossRadID != MFUnset) return (_MDOutCommon_GrossRadID);

	MFDefEntering ("Gross Radiance");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);

	switch (optID) {
		case MDinput: _MDOutCommon_GrossRadID = MFVarGetID (MDVarCore_GrossRadiance, "MJ/m^2", MFInput, MFFlux, MFBoundary); break;
		case MDstandard:
			if (((optStr = MFOptionGet (MDParGrossRadTAU)) != (char *) NULL) && (sscanf (optStr,"%f",&par) == 1)) _MDGrossRadStdTAU = par;
			if (((_MDOutCommon_GrossRadID    = MFVarGetID (MDVarCore_GrossRadiance, "MJ/m^2", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction(_MDCommon_GrossRadianceStd) == CMfailed)) return (CMfailed);
			break;
		case MDOtto:
			if (((_MDOutCommon_GrossRadID    = MFVarGetID (MDVarCore_GrossRadiance, "MJ/m^2", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction(_MDCommon_GrossRadianceOtto) == CMfailed)) return (CMfailed);
			break;
		default:  MFOptionMessage (optName, optStr, options); return (CMfailed);
	}
	MFDefLeaving ("Gross Radiance");
	return (_MDOutCommon_GrossRadID);
}

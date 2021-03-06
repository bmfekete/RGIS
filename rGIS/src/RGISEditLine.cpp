/******************************************************************************

GHAAS RiverGIS V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

RGISEditLine.cpp

bfekete@gc.cuny.edu

*******************************************************************************/

#include <rgis.hpp>

void RGISEditLineDirectionCBK (Widget widget, RGISWorkspace *workspace,XmAnyCallbackStruct *callData)

	{
	DBFloat elev0, elev1;
	DBCoordinate coord;
	DBObjRecord *lineRec;
	DBDataset *dataset = UIDataset ();
	DBObjData *lineData = dataset->Data ();
	DBObjData *gridData = lineData->LinkedData ();
	DBVLineIF *lineIF = new DBVLineIF (lineData);
	DBGridIF *gridIF = new DBGridIF (gridData);

	UIPauseDialogOpen ((char *) "Changing Directionality");
	for (lineRec = lineIF->FirstItem ();lineRec != (DBObjRecord *) NULL;lineRec = lineIF->NextItem ())
		{
		DBPause (lineRec->RowID () * 100 / lineIF->ItemNum ());
		coord = lineIF->FromCoord (lineRec);
		if (gridIF->Value (coord,&elev0) == false) continue;
		coord = lineIF->ToCoord (lineRec);
		if (gridIF->Value (coord,&elev1) == false) continue;
		if (elev0 < elev1) lineIF->Flip (lineRec);
		}
	UIPauseDialogClose ();
	}

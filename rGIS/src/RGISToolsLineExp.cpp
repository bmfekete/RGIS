/******************************************************************************

GHAAS RiverGIS V3.0
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

RGISToolsLineExp.cpp

bfekete@gc.cuny.edu

*******************************************************************************/

#include <ctype.h>
#include <rgis.hpp>

int _RGISToolsLineExportARCInfo (DBObjData *data,char *selection)

	{
	FILE *file;
	DBInt pos;
	char coverName	[11], command    [FILENAME_MAX + 20]; 
	char asciiGenerate   [FILENAME_MAX];
	char asciiAttribDef  [FILENAME_MAX];
	char asciiAttribData [FILENAME_MAX];
	char amlFile    [FILENAME_MAX];

	sprintf (asciiGenerate,"%s/asciigen.tmp",selection);
	if (DBExportARCGenLine (data,asciiGenerate) == DBFault) return (DBFault);
	sprintf (asciiAttribDef,"%s/asciiattrib.def",selection);
	if (DBExportARCTableDef (data,DBrNItems,asciiAttribDef) == DBFault)
		{ unlink (asciiGenerate); return (DBFault); }
	sprintf (asciiAttribData,"%s/asciiattrib.dat",selection);
	if (DBExportARCTableData (data,DBrNItems,asciiAttribData) == DBFault)
		{ unlink (asciiGenerate); unlink (asciiAttribDef); return (DBFault); }

	for (pos = strlen (data->FileName ());pos > 0;pos--) if ((data->FileName ()) [pos] == '/') break;
	strncpy (coverName,(data->FileName ()) + pos + 1,sizeof (coverName) - 1);
	coverName	[sizeof (coverName) - 1] = '\0';
	for (pos = 0;pos < (DBInt) strlen (coverName);++pos) if (coverName [pos] == '.') coverName [pos] = '\0';
	for (pos = 0;pos < (DBInt) strlen (coverName);++pos) coverName [pos] = tolower (coverName [pos]);
	sprintf (amlFile,"%s/linecreate.aml",selection);
	if ((file = fopen (amlFile,"w")) == NULL)
		{
		unlink (asciiGenerate);
		unlink (asciiAttribDef);
		unlink (asciiAttribData);
		CMmsgPrint (CMmsgSysError, "Aml File Creation Error in: %s %d",__FILE__,__LINE__);
		return (DBFault);
		}
	fprintf (file,"&workspace %s\n",selection);
	fprintf (file,"generate c_%s\n",coverName);
	fprintf (file,"input %s\n",asciiGenerate);
	fprintf (file,"lines\n");
	fprintf (file,"quit\n");
	fprintf (file,"build c_%s lines\n",coverName);
	fprintf (file,"tables\n");
	fprintf (file,"define c_%s.lattr\n",coverName);
	fprintf (file,"&r %s\n",asciiAttribDef);
	fprintf (file,"~\n");
	fprintf (file,"alter id\n");
	fprintf (file,"c_%s-id,,,,\n",coverName);
	fprintf (file,"add\n");
	fprintf (file,"&r %s\n",asciiAttribData);
	fprintf (file,"~\n");
	fprintf (file,"q stop\n");
	fprintf (file,"joinitem c_%s.aat c_%s.lattr c_%s.aat c_%s-id c_%s-id\n",coverName,coverName,coverName,coverName,coverName);
	fprintf (file,"tables\n");
	fprintf (file,"select c_%s.lattr\n",coverName);
	fprintf (file,"erase c_%s.lattr\ny\n",coverName);
	fprintf (file,"q stop\n");
	fclose (file);

	if (getenv ("GHAAS_ARC") != NULL)
		{
		sprintf (command,getenv ("GHAAS_ARC"),amlFile);
		system (command);
		unlink (asciiGenerate);
		unlink (asciiAttribDef);
		unlink (asciiAttribData);
		unlink (amlFile);
		}
	return (DBSuccess);
	}

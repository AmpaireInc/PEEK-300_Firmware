
/* File generated by gen_cfile.py. Should not be modified. */

#ifndef CREATBOTOD_H
#define CREATBOTOD_H

#include <canfestival.h>

/* Prototypes of function provided by object dictionnary */
UNS32 CreatBotOD_valueRangeTest (UNS8 typeValue, void * value);
const indextable * CreatBotOD_scanIndexOD (CO_Data *d, UNS16 wIndex, UNS32 * errorCode);

/* Master node data struct */
extern CO_Data CreatBotOD_Data;
extern UNS8 MarchineInfo_FW_Ver[12];		/* Mapped at index 0x2000, subindex 0x01 */
extern UNS8 MarchineInfo_UI_Ver[7];		/* Mapped at index 0x2000, subindex 0x02 */
extern UNS8 MarchineInfo_ExtNum;		/* Mapped at index 0x2000, subindex 0x03 */
extern UNS8 MarchineInfo_BedSupport;		/* Mapped at index 0x2000, subindex 0x04 */
extern UNS8 MarchineInfo_ChamberSupport;		/* Mapped at index 0x2000, subindex 0x05 */
extern UNS8 MarchineInfo_FilamentChamberSupport;		/* Mapped at index 0x2000, subindex 0x06 */
extern UNS8 MarchineInfo_WaterTempSupport;		/* Mapped at index 0x2000, subindex 0x07 */
extern UNS8 MarchineInfo_WIFISupport;		/* Mapped at index 0x2000, subindex 0x08 */
extern UNS8 MarchineInfo_CameraSupport;		/* Mapped at index 0x2000, subindex 0x09 */
extern UNS16 CMD_TASK;		/* Mapped at index 0x2001, subindex 0x00*/
extern UNS16 CMD_MOVE;		/* Mapped at index 0x2002, subindex 0x00*/
extern INTEGER16 CMD_EXTRUDER;		/* Mapped at index 0x2003, subindex 0x00*/
extern UNS16 CMD_COMMON;		/* Mapped at index 0x2004, subindex 0x00*/
extern UNS16 FILE_INDEX;		/* Mapped at index 0x2005, subindex 0x00*/
extern UNS8 GCODE_INPUT[64];		/* Mapped at index 0x2006, subindex 0x00*/
extern UNS16 MaxTemp[12];		/* Mapped at index 0x2100, subindex 0x01 - 0x0C */
extern UNS8 Global_Common;		/* Mapped at index 0x3000, subindex 0x01 */
extern UNS8 Global_WIFI;		/* Mapped at index 0x3000, subindex 0x02 */
extern UNS16 Global_Switch;		/* Mapped at index 0x3000, subindex 0x03 */
extern UNS16 Global_State;		/* Mapped at index 0x3000, subindex 0x04 */
extern UNS16 Global_Reserve;		/* Mapped at index 0x3000, subindex 0x05 */
extern UNS16 CurTemp[12];		/* Mapped at index 0x3001, subindex 0x01 - 0x0C */
extern UNS16 TarTemp[12];		/* Mapped at index 0x3002, subindex 0x01 - 0x0C */
extern REAL32 TaskInfo_PrintPercent;		/* Mapped at index 0x3003, subindex 0x01 */
extern UNS32 TaskInfo_PrintTime;		/* Mapped at index 0x3003, subindex 0x02 */
extern UNS16 TaskInfo_PrintFlowrateE0;		/* Mapped at index 0x3003, subindex 0x03 */
extern UNS16 TaskInfo_PrintFlowrateE1;		/* Mapped at index 0x3003, subindex 0x04 */
extern UNS16 TaskInfo_PrintFlowrateE2;		/* Mapped at index 0x3003, subindex 0x05 */
extern UNS16 TaskInfo_PrintFlowrateE3;		/* Mapped at index 0x3003, subindex 0x06 */
extern UNS16 TaskInfo_PrintFeedRate;		/* Mapped at index 0x3003, subindex 0x07 */
extern UNS48 TaskInfo_PrintReserve;		/* Mapped at index 0x3003, subindex 0x08 */
extern UNS8 Percent[8];		/* Mapped at index 0x3004, subindex 0x01 - 0x08 */
extern REAL32 Position[4];		/* Mapped at index 0x3005, subindex 0x01 - 0x04 */
extern REAL32 StepPerMm[7];		/* Mapped at index 0x3006, subindex 0x01 - 0x07 */
extern REAL32 Acceleration[7];		/* Mapped at index 0x3007, subindex 0x01 - 0x07 */
extern REAL32 StepReserve[4];		/* Mapped at index 0x3008, subindex 0x01 - 0x04 */
extern UNS16 Statistics_PrintsTotal;		/* Mapped at index 0x3009, subindex 0x01 */
extern UNS16 Statistics_PrintsSuccess;		/* Mapped at index 0x3009, subindex 0x02 */
extern UNS32 Statistics_PrintsLongest;		/* Mapped at index 0x3009, subindex 0x03 */
extern UNS32 Statistics_PrintsUsed;		/* Mapped at index 0x3009, subindex 0x04 */
extern REAL32 Statistics_FilamentUsed;		/* Mapped at index 0x3009, subindex 0x05 */
extern UNS32 Statistics_Reserve1;		/* Mapped at index 0x3009, subindex 0x06 */
extern UNS32 Statistics_Reserve2;		/* Mapped at index 0x3009, subindex 0x07 */
extern UNS32 Register_TrialPeriod;		/* Mapped at index 0x300A, subindex 0x01 */
extern REAL32 Register_RegKey;		/* Mapped at index 0x300A, subindex 0x02 */
extern REAL32 Leveling_ZOffset;		/* Mapped at index 0x300B, subindex 0x01 */
extern REAL32 Leveling_FadeHeight;		/* Mapped at index 0x300B, subindex 0x02 */
extern UNS32 Leveling_Reserve1;		/* Mapped at index 0x300B, subindex 0x03 */
extern UNS32 Leveling_Reserve2;		/* Mapped at index 0x300B, subindex 0x04 */

extern UNS8 CreatBotOD_obj1008[32];
extern UNS8 CreatBotOD_obj1009[7];
extern UNS8 CreatBotOD_obj100A[12];

#endif // CREATBOTOD_H

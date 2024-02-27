/**
 *  canOpen.cpp
 *
 *  Created on: 2023/07/28
 *      Author: CreatBot LYN
 *
 *  Protocol layer for linux lcd.
 */

#include "../../../../inc/MarlinConfigPre.h"

#if ENABLED(CREATBOT_LINUX_LCD)

  #include <RingBuf.h>

  #include "../../../../MarlinCore.h"
  #include "../../../../module/stepper.h"
  #include "../../../../module/temperature.h"
  #include "../../ui_api.h"
  #include "../creatbotFunction.h"
  #include "CreatBotOD.h"
  #include "canOpen.h"
  #include "canOpenHelper.h"

  #define DEBUG_OUT ENABLED(DEBUG_CANOPEN)
  #include "../../../../core/debug_out.h"

static uint8_t nodeNum    = 0;
static bool    LCDIsReady = false;

static RingBuf<notifyLCD, NOTIFY_BUFLEN> notifyRing;

  #if ENABLED(SDSUPPORT)
static CreatBotFileList fileList;
  #endif

constexpr const char HWVersion[] PROGMEM = "1.0.0";
constexpr const char UIVerison[] PROGMEM = "7.0.0";
constexpr const char FWVersion[] PROGMEM = {
    // Version
    SHORT_BUILD_VERSION[0], SHORT_BUILD_VERSION[1], SHORT_BUILD_VERSION[2], '.',

    // YY year
    __DATE__[9], __DATE__[10],

    // First month letter, Oct Nov Dec = '1' otherwise '0'
    (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',

    // Second month letter
    (__DATE__[0] == 'J')   ? ((__DATE__[1] == 'a') ? '1' : ((__DATE__[2] == 'n') ? '6' : '7'))  // Jan, Jun or Jul
    : (__DATE__[0] == 'F') ? '2'                                                                // Feb
    : (__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5'                                   // Mar or May
    : (__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8'                                   // Apr or Aug
    : (__DATE__[0] == 'S') ? '9'                                                                // Sep
    : (__DATE__[0] == 'O') ? '0'                                                                // Oct
    : (__DATE__[0] == 'N') ? '1'                                                                // Nov
    : (__DATE__[0] == 'D') ? '2'                                                                // Dec
                           : 0,

    // First day letter, replace space with digit
    __DATE__[4] == ' ' ? '0' : __DATE__[4],

    // Second day letter
    __DATE__[5],

    '\0'};

// 更新温度信息
static void updateTempInfo() {
  OPTCODE(HAS_HOTEND0, CurTemp[ARRIDX(T_E0)] = CANOPEN_CURTEMP(H0))
  OPTCODE(HAS_HOTEND1, CurTemp[ARRIDX(T_E1)] = CANOPEN_CURTEMP(H1))
  OPTCODE(HAS_HOTEND2, CurTemp[ARRIDX(T_E2)] = CANOPEN_CURTEMP(H2))
  OPTCODE(HAS_HOTEND3, CurTemp[ARRIDX(T_E3)] = CANOPEN_CURTEMP(H3))
  OPTCODE(HAS_HEATED_BED, CurTemp[ARRIDX(T_Bed)] = CANOPEN_CURTEMP(BED))
  OPTCODE(HAS_HEATED_CHAMBER, CurTemp[ARRIDX(T_Chamber)] = CANOPEN_CURTEMP(CHAMBER))
  // CurTemp[ARRIDX(T_Filament)] = 0;
  // CurTemp[ARRIDX(T_Water)]    = 0;

  OPTCODE(HAS_HOTEND0, TarTemp[ARRIDX(T_E0)] = CANOPEN_TARTEMP(H0))
  OPTCODE(HAS_HOTEND1, TarTemp[ARRIDX(T_E1)] = CANOPEN_TARTEMP(H1))
  OPTCODE(HAS_HOTEND2, TarTemp[ARRIDX(T_E2)] = CANOPEN_TARTEMP(H2))
  OPTCODE(HAS_HOTEND3, TarTemp[ARRIDX(T_E3)] = CANOPEN_TARTEMP(H3))
  OPTCODE(HAS_HEATED_BED, TarTemp[ARRIDX(T_Bed)] = CANOPEN_TARTEMP(BED))
  OPTCODE(HAS_HEATED_CHAMBER, TarTemp[ARRIDX(T_Chamber)] = CANOPEN_TARTEMP(CHAMBER))
  // TarTemp[ARRIDX(T_Filament)] = 0;
}

// 更新打印相关信息
static void updatePrintInfo() {
  TaskInfo_PrintPercent  = TERN(HAS_PRINT_PROGRESS_PERMYRIAD,  //
                                ExtUI::getProgress_permyriad() / 100.0, ExtUI::getProgress_percent());
  TaskInfo_PrintTime     = ExtUI::getProgress_seconds_elapsed();
  TaskInfo_PrintFeedRate = ExtUI::getFeedrate_percent();
  OPTCODE(HAS_EXTRUDER0, TaskInfo_PrintFlowrateE0 = ExtUI::getFlow_percent(ExtUI::extruder_t::E0))
  OPTCODE(HAS_EXTRUDER1, TaskInfo_PrintFlowrateE1 = ExtUI::getFlow_percent(ExtUI::extruder_t::E1))
  OPTCODE(HAS_EXTRUDER2, TaskInfo_PrintFlowrateE2 = ExtUI::getFlow_percent(ExtUI::extruder_t::E2))
  OPTCODE(HAS_EXTRUDER3, TaskInfo_PrintFlowrateE3 = ExtUI::getFlow_percent(ExtUI::extruder_t::E3))
}

// 更新百分比相关信息
static void updatePercentInfo() {
  OPTCODE(AUTO_FAN_CAN_ADJUST, Percent[ARRIDX(P_FanT)] = ui8_to_percent(thermalManager.extruder_auto_fan_speed))
  OPTCODE(HAS_FAN0, Percent[ARRIDX(P_FanG)] = ExtUI::getTargetFan_percent(ExtUI::fan_t::FAN0))
  OPTCODE(HAS_FAN1, Percent[ARRIDX(P_FanA)] = ExtUI::getTargetFan_percent(ExtUI::fan_t::FAN1))
  // TODO 自动关机时间索引
  Percent[ARRIDX(P_ShutdownTime)] = 0;
}

// 更新坐标位置信息
static void updatePosInfo() {
  OPTCODE(HAS_X_AXIS, Position[ARRIDX(Pos_X)] = ExtUI::getAxisPosition_mm(ExtUI::axis_t::X))
  OPTCODE(HAS_Y_AXIS, Position[ARRIDX(Pos_Y)] = ExtUI::getAxisPosition_mm(ExtUI::axis_t::Y))
  OPTCODE(HAS_Z_AXIS, Position[ARRIDX(Pos_Z)] = ExtUI::getAxisPosition_mm(ExtUI::axis_t::Z))
  // OPTCODE(HAS_X2_STEPPER, Position[ARRIDX(Pos_X2)] = ExtUI::getAxisPosition_mm(ExtUI::axis_t::X2))  // TODO
}

// 更新电机相关信息
static void updateStepInfo() {
  // 电机步进值
  OPTCODE(HAS_X_AXIS, StepPerMm[ARRIDX(SPM_X)] = ExtUI::getAxisSteps_per_mm(ExtUI::axis_t::X))
  OPTCODE(HAS_Y_AXIS, StepPerMm[ARRIDX(SPM_Y)] = ExtUI::getAxisSteps_per_mm(ExtUI::axis_t::Y))
  OPTCODE(HAS_Z_AXIS, StepPerMm[ARRIDX(SPM_Z)] = ExtUI::getAxisSteps_per_mm(ExtUI::axis_t::Z))
  OPTCODE(HAS_EXTRUDER0, StepPerMm[ARRIDX(SPM_E0)] = ExtUI::getAxisSteps_per_mm(ExtUI::extruder_t::E0))
  OPTCODE(HAS_EXTRUDER1, StepPerMm[ARRIDX(SPM_E1)] = ExtUI::getAxisSteps_per_mm(ExtUI::extruder_t::E1))
  OPTCODE(HAS_EXTRUDER2, StepPerMm[ARRIDX(SPM_E2)] = ExtUI::getAxisSteps_per_mm(ExtUI::extruder_t::E2))
  OPTCODE(HAS_EXTRUDER3, StepPerMm[ARRIDX(SPM_E3)] = ExtUI::getAxisSteps_per_mm(ExtUI::extruder_t::E3))

  // 加速度
  Acceleration[ARRIDX(Acc_Print)]    = ExtUI::getPrintingAcceleration_mm_s2();
  Acceleration[ARRIDX(Acc_Rectract)] = ExtUI::getRetractAcceleration_mm_s2();
  Acceleration[ARRIDX(Acc_Travel)]   = ExtUI::getTravelAcceleration_mm_s2();
  OPTCODE(HAS_X_AXIS, Acceleration[ARRIDX(Acc_MaxX)] = ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::axis_t::X))
  OPTCODE(HAS_Y_AXIS, Acceleration[ARRIDX(Acc_MaxY)] = ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::axis_t::Y))
  OPTCODE(HAS_Z_AXIS, Acceleration[ARRIDX(Acc_MaxZ)] = ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::axis_t::Z))
  OPTCODE(HAS_EXTRUDER0, Acceleration[ARRIDX(Acc_MaxE)] = ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::extruder_t::E0))
}

// 更新统计信息
static void updateStatistics() {
  #if ENABLED(PRINTCOUNTER)
  printStatistics state    = print_job_timer.getStats();
  Statistics_PrintsTotal   = state.totalPrints;
  Statistics_PrintsSuccess = state.finishedPrints;
  Statistics_PrintsLongest = state.longestPrint;
  Statistics_PrintsUsed    = state.printTime;
  Statistics_FilamentUsed  = state.filamentUsed;
  #endif
}

// 更新调平信息
static void updateLeveling() {
  #if HAS_LEVELING
  Leveling_ZOffset    = ExtUI::getZOffset_mm() * -1.0;
  Leveling_FadeHeight = planner.z_fade_height;
  #endif
}

// 更新全局信息
static void updateGlobalInfo() {
  CanOpen_Global_Common uCommon;
  CanOpen_Global_WIFI   uWIFI;
  CanOpen_Global_Switch uSwitch;
  CanOpen_Global_State  uState;

  uCommon.validState = getValidState();
  uCommon.workState  = getWorkState();
  uCommon.activeT    = active_extruder;
  Global_Common      = uCommon.value;

  #if ENABLED(CREATBOT_WIFI_SUPPORT)
  uWIFI.workCode    = 0;
  uWIFI.cameraState = 0;
  uWIFI.cloudState  = 0;
  Global_WIFI       = uWIFI.value;
  #endif

  uSwitch.caseLight    = TERN1(CASE_LIGHT_ENABLE, ExtUI::getCaseLightState());
  uSwitch.autoPower    = 0;
  uSwitch.autoLeveling = TERN0(HAS_LEVELING, ExtUI::getLevelingActive());
  Global_Switch        = uSwitch.value;

  uState.diskState     = TERN0(SDSUPPORT, getMediaState());
  uState.moving        = ExtUI::isMoving();  // || planner.cleaning_buffer_counter
  uState.probeValid    = TERN0(HAS_LEVELING, ExtUI::getMeshValid());
  uState.waitHeating   = wait_for_heatup;
  Global_State         = uState.value;
}

static void registerHeartCallBack();
static void firstUnpack(switchVal);
static void refreshFileToLcd(fileAct);
static void enableMoter(switchVal);
static void enableLight(switchVal);
static void enableAutoLeveling(switchVal);
static void changeExtruder(uint8_t);
static void stopOperateWaiting(operate);
static void setHeaterWaiting(heating);
static void setLoadWaiting(loading);

// 温度设置回调函数
static uint32_t onTarTempSet(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_TARTEMP) {
    switch (subindex) {
      case subIndex_T_E0: OPTCODE(HAS_HOTEND, CANOPEN_SETTEMP(H0, TarTemp[ARRIDX(T_E0)])) break;
      case subIndex_T_E1: OPTCODE(HAS_HOTEND1, CANOPEN_SETTEMP(H1, TarTemp[ARRIDX(T_E1)])) break;
      case subIndex_T_E2: OPTCODE(HAS_HOTEND2, CANOPEN_SETTEMP(H2, TarTemp[ARRIDX(T_E2)])) break;
      case subIndex_T_E3: OPTCODE(HAS_HOTEND3, CANOPEN_SETTEMP(H3, TarTemp[ARRIDX(T_E3)])) break;
      case subIndex_T_Bed: OPTCODE(HAS_HEATED_BED, CANOPEN_SETTEMP(BED, TarTemp[ARRIDX(T_Bed)])) break;
      case subIndex_T_Chamber: OPTCODE(HAS_HEATED_CHAMBER, CANOPEN_SETTEMP(CHAMBER, TarTemp[ARRIDX(T_Chamber)])) break;
      case subIndex_T_Filament:
      case subIndex_T_Water:
      default: break;
    }
  }
  updateTempInfo();  // 再次更新温度信息, 避免温度设置被最大值限制

  return OD_SUCCESSFUL;
}

// 挤出量设置回调函数
static uint32_t onFlowrateSet(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_TASKINFO) {
    switch (subindex) {
      case subIndex_Task_PrintFlowrateE0:
        OPTCODE(HAS_EXTRUDERS, ExtUI::setFlow_percent(TaskInfo_PrintFlowrateE0, ExtUI::extruder_t::E0))
        break;
      case subIndex_Task_PrintFlowrateE1:
        OPTCODE(HAS_EXTRUDER1, ExtUI::setFlow_percent(TaskInfo_PrintFlowrateE1, ExtUI::extruder_t::E1))
        break;
      case subIndex_Task_PrintFlowrateE2:
        OPTCODE(HAS_EXTRUDER2, ExtUI::setFlow_percent(TaskInfo_PrintFlowrateE2, ExtUI::extruder_t::E2))
        break;
      case subIndex_Task_PrintFlowrateE3:
        OPTCODE(HAS_EXTRUDER3, ExtUI::setFlow_percent(TaskInfo_PrintFlowrateE3, ExtUI::extruder_t::E3))
        break;
      default: break;
    }
  }
  return OD_SUCCESSFUL;
}

// 打印速度设置回调函数
static uint32_t onFeedrateSet(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_TASKINFO) {
    if (subindex = subIndex_Task_PrintFeedRate) {
      ExtUI::setFeedrate_percent(TaskInfo_PrintFeedRate);
    }
  }
  return OD_SUCCESSFUL;
}

// 百分比设置回调函数
static uint32_t onPercentSet(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_PERCNET) {
    switch (subindex) {
      case subIndex_P_FanT:
        OPTCODE(AUTO_FAN_CAN_ADJUST, thermalManager.extruder_auto_fan_speed = percent_to_ui8(Percent[ARRIDX(P_FanT)]))
        break;
      case subIndex_P_FanG:
        OPTCODE(HAS_FAN0, ExtUI::setTargetFan_percent(Percent[ARRIDX(P_FanG)], ExtUI::fan_t::FAN0))
        break;
      case subIndex_P_FanA:
        OPTCODE(HAS_FAN1, ExtUI::setTargetFan_percent(Percent[ARRIDX(P_FanA)], ExtUI::fan_t::FAN1))
        break;
      case subIndex_P_ShutdownTime: break;  // TODO
      default: break;
    }
  }
  return OD_SUCCESSFUL;
}

// 步进值设置回调函数
static uint32_t onSPMSet(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_STEPPERMM) {
    switch (subindex) {
      case subIndex_SPM_X:
        OPTCODE(HAS_X_AXIS, ExtUI::setAxisSteps_per_mm(StepPerMm[ARRIDX(SPM_X)], ExtUI::axis_t::X))
        break;
      case subIndex_SPM_Y:
        OPTCODE(HAS_Y_AXIS, ExtUI::setAxisSteps_per_mm(StepPerMm[ARRIDX(SPM_Y)], ExtUI::axis_t::Y))
        break;
      case subIndex_SPM_Z:
        OPTCODE(HAS_Z_AXIS, ExtUI::setAxisSteps_per_mm(StepPerMm[ARRIDX(SPM_Z)], ExtUI::axis_t::Z))
        break;
      case subIndex_SPM_E0:
        OPTCODE(HAS_EXTRUDER0, ExtUI::setAxisSteps_per_mm(StepPerMm[ARRIDX(SPM_E0)], ExtUI::extruder_t::E0))
        break;
      case subIndex_SPM_E1:
        OPTCODE(HAS_EXTRUDER1, ExtUI::setAxisSteps_per_mm(StepPerMm[ARRIDX(SPM_E1)], ExtUI::extruder_t::E1))
        break;
      case subIndex_SPM_E2:
        OPTCODE(HAS_EXTRUDER2, ExtUI::setAxisSteps_per_mm(StepPerMm[ARRIDX(SPM_E2)], ExtUI::extruder_t::E2))
        break;
      case subIndex_SPM_E3:
        OPTCODE(HAS_EXTRUDER3, ExtUI::setAxisSteps_per_mm(StepPerMm[ARRIDX(SPM_E3)], ExtUI::extruder_t::E3))
        break;
      default: break;
    }
    settings.save();
  }
  return OD_SUCCESSFUL;
}

// 加速度设置回调函数
static uint32_t onAccSet(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_ACCELERATION) {
    switch (subindex) {
      case subIndex_Acc_Print: ExtUI::setPrintingAcceleration_mm_s2(Acceleration[ARRIDX(Acc_Print)]); break;
      case subIndex_Acc_Rectract: ExtUI::setRetractAcceleration_mm_s2(Acceleration[ARRIDX(Acc_Rectract)]); break;
      case subIndex_Acc_Travel: ExtUI::setTravelAcceleration_mm_s2(Acceleration[ARRIDX(Acc_Travel)]); break;
      case subIndex_Acc_MaxX:
        OPTCODE(HAS_X_AXIS, ExtUI::setAxisMaxAcceleration_mm_s2(Acceleration[ARRIDX(Acc_MaxX)], ExtUI::axis_t::X))
        break;
      case subIndex_Acc_MaxY:
        OPTCODE(HAS_Y_AXIS, ExtUI::setAxisMaxAcceleration_mm_s2(Acceleration[ARRIDX(Acc_MaxY)], ExtUI::axis_t::Y))
        break;
      case subIndex_Acc_MaxZ:
        OPTCODE(HAS_Z_AXIS, ExtUI::setAxisMaxAcceleration_mm_s2(Acceleration[ARRIDX(Acc_MaxZ)], ExtUI::axis_t::Z))
        break;
      case subIndex_Acc_MaxE:
        OPTCODE(HAS_EXTRUDER0,
                ExtUI::setAxisMaxAcceleration_mm_s2(Acceleration[ARRIDX(Acc_MaxE)], ExtUI::extruder_t::E0))
        break;
      default: break;
    }
    settings.save();
  }
  return OD_SUCCESSFUL;
}

// 调平参数设置回调函数
static uint32_t onLevelSet(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  #if HAS_LEVELING
  if (indexTable->index == INDEX_LEVELING) {
    switch (subindex) {
      case subIndex_Level_ZOffset: ExtUI::setZOffset_mm(Leveling_ZOffset * -1.0); break;
      case subIndex_Level_FadeHeight: planner.set_z_fade_height(Leveling_FadeHeight); break;
      default: break;
    }
    settings.save();
  }
  #endif
  return OD_SUCCESSFUL;
}

// 任务指令回调函数
static uint32_t onCmdTask(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_CMD_TASK) {
    switch ((taskCmd)(CMD_TASK & 0xFF)) {
      case taskCmd::TASK_PAUSE:
        if (ExtUI::isPrintingFromMedia() && !ExtUI::isPrintingFromMediaPaused()) {
          ExtUI::pausePrint();
        }
        break;
      case taskCmd::TASK_RESUME:
        if (ExtUI::isPrintingFromMediaPaused()) {
          if (TERN1(HAS_FILAMENT_SENSOR, ExtUI::getFilamentInsertState())) {
            ExtUI::resumePrint();
          } else {
            sendNotify(notifyLCD::FILAMENT_ERROR);
          }
        }
        break;
      case taskCmd::TASK_STOP:
        if (ExtUI::isPrintingFromMedia()) {
          quickstop_stepper();
          ExtUI::stopPrint();
        }
        break;
      case taskCmd::TASK_CHANGE:
        if (ExtUI::isPrintingFromMedia() && !ExtUI::isPrintingFromMediaPaused()) {
          ExtUI::injectCommands(F("M600"));
          sendNotify(notifyLCD::FILAMENT_INIT);
        }
        break;
      case taskCmd::ACCIDENT_RESUME: break;
      case taskCmd::ACCIDENT_CANCEL: break;
      default: break;
    }
  }
  return OD_SUCCESSFUL;
}

// 移轴指令回调函数
static uint32_t onCmdMove(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_CMD_MOVE) {
    float offset;

    // clang-format off
    switch ((moveUnit)(CMD_MOVE & 0xFF)) {  // 低字节表示移轴单位
      case moveUnit::MOVE_01MM:   offset = 0.1;   break;
      case moveUnit::MOVE_1MM:    offset = 1.0;   break;
      case moveUnit::MOVE_10MM:   offset = 10.0;  break;
      case moveUnit::MOVE_100MM:  offset = 100.0; break;
      default:                    offset = 0.0;   break;
    }

    switch ((moveIndex)(CMD_MOVE >> 8)) {  // 高字节表示移轴命令
      case moveIndex::ALL_HOME:   ExtUI::injectCommands(F("G28"));  break;
      case moveIndex::X_HOME:     ExtUI::injectCommands(F("G28X")); break;
      case moveIndex::Y_HOME:     ExtUI::injectCommands(F("G28Y")); break;
      case moveIndex::Z_HOME:     ExtUI::injectCommands(F("G28Z")); break;
      case moveIndex::X_MINUS:    offset *= -1;
      case moveIndex::X_PLUS:     delayMoveAxis(offset, ExtUI::axis_t::X); break;
      case moveIndex::Y_MINUS:    offset *= -1;
      case moveIndex::Y_PLUS:     delayMoveAxis(offset, ExtUI::axis_t::Y); break;
      case moveIndex::Z_MINUS:    offset *= -1;
      case moveIndex::Z_PLUS:     delayMoveAxis(offset, ExtUI::axis_t::Z); break;
      default: break;
    }
    // clang-format on
  }
  return OD_SUCCESSFUL;
}

// 送/退丝, 加/卸载 回调函数
static uint32_t onCmdExtrude(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_CMD_EXTRUDE) {
    ExtUI::extruder_t curTool = ExtUI::getActiveTool();
    if (thermalManager.targetTooColdToExtrude(curTool)) {
      sendNotify(notifyLCD::TAR_TEMP_LOW);
    } else {
      if (CMD_EXTRUDER == INT16_MAX) {         // 0x7FFF
        ExtUI::injectCommands(F("M701"));      // 加载耗材
      } else if (CMD_EXTRUDER == INT16_MIN) {  // 0x8000
        ExtUI::injectCommands(F("M702"));      // 卸载耗材
      } else {
        if (thermalManager.tooColdToExtrude(curTool)) {
          sendNotify(notifyLCD::CUR_TEMP_LOW);
        } else {
          UI_INCREMENT_BY(AxisPosition_mm, (float)CMD_EXTRUDER, curTool);
          sendNotify((CMD_EXTRUDER > 0) ? notifyLCD::FILA_EXTRUDE : notifyLCD::FILA_RETRACT);
        }
      }
    }
  }
  return OD_SUCCESSFUL;
}

// 其他通用指令回调函数
static uint32_t onCmdCommon(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_CMD_COMMON) {
    uint8_t cmdPara = CMD_COMMON & 0xFF;     // 低字节表示参数
    switch ((commonCmd)(CMD_COMMON >> 8)) {  // 高字节表示指令
      case commonCmd::FACTORY_RESET: settings.reset(); break;
      case commonCmd::SAVE_SETTING: settings.save(); break;
      case commonCmd::FIRST_UNPACK: firstUnpack((switchVal)cmdPara); break;
      case commonCmd::REFRESH_FILE: refreshFileToLcd((fileAct)cmdPara); break;
      case commonCmd::MOTOR_SWITCH: enableMoter((switchVal)cmdPara); break;
      case commonCmd::BABYSTEP: break;
      case commonCmd::CASELIGHT: enableLight((switchVal)cmdPara); break;
      case commonCmd::ATUOSHUTDWON: break;
      case commonCmd::ATUOLEVELING: enableAutoLeveling((switchVal)cmdPara); break;
      case commonCmd::CHANGE_TOOL: changeExtruder(cmdPara); break;
      case commonCmd::STOP_OPERATE: stopOperateWaiting((operate)cmdPara); break;
      case commonCmd::WAIT_HEATING: setHeaterWaiting((heating)cmdPara); break;
      case commonCmd::LOAD_FILAMENT: setLoadWaiting((loading)cmdPara); break;
      case commonCmd::LEVELING_PROBE: ExtUI::injectCommands(F("G29N")); break;
      case commonCmd::USER_CONFIRMED: ExtUI::setUserConfirmed(); break;
    }
  }
  return OD_SUCCESSFUL;
}

// 指定文件索引回调函数
static uint32_t onFileIndex(CO_Data *d, const indextable *indexTable, uint8_t subindex) {
  if (indexTable->index == INDEX_FILE_INDEX) {
    if (FILE_INDEX == UINT16_MAX) {  // 返回上级
      fileList.upDir();
      refreshFileToLcd(fileAct::WORKDIR);
    } else {
      if (fileList.seek(SD_ORDER(FILE_INDEX, fileList.count()))) {
        if (fileList.isDir()) {  // 打开子目录
          fileList.changeDir(fileList.shortFilename());
          refreshFileToLcd(fileAct::WORKDIR);
        } else {  // 打印文件
          ExtUI::printFile(fileList.shortFilename());
        }
      }
    }
  }
  return OD_SUCCESSFUL;
}

// 初始化更新 (只读信息)
static void initNodeInfo() {
  // 机器信息
  strcpy_P((char *)MarchineInfo_FW_Ver, FWVersion);
  strcpy_P((char *)MarchineInfo_UI_Ver, UIVerison);
  MarchineInfo_ExtNum                 = EXTRUDERS;
  MarchineInfo_BedSupport             = HAS_HEATED_BED;
  MarchineInfo_ChamberSupport         = HAS_HEATED_CHAMBER;
  MarchineInfo_FilamentChamberSupport = false;
  MarchineInfo_WaterTempSupport       = false;
  MarchineInfo_WIFISupport            = ENABLED(CREATBOT_WIFI_SUPPORT);
  MarchineInfo_CameraSupport          = false;

  strcpy_P((char *)CreatBotOD_obj1008, PSTR(CUSTOM_MACHINE_NAME));
  strcpy_P((char *)CreatBotOD_obj1009, HWVersion);
  strcpy_P((char *)CreatBotOD_obj100A, FWVersion);

  // 配置最大温度设置
  OPTCODE(HAS_HOTEND0, MaxTemp[ARRIDX(T_E0)] = thermalManager.hotend_max_target(ExtUI::heater_t::H0))
  OPTCODE(HAS_HOTEND1, MaxTemp[ARRIDX(T_E1)] = thermalManager.hotend_max_target(ExtUI::heater_t::H1))
  OPTCODE(HAS_HOTEND2, MaxTemp[ARRIDX(T_E2)] = thermalManager.hotend_max_target(ExtUI::heater_t::H2))
  OPTCODE(HAS_HOTEND3, MaxTemp[ARRIDX(T_E3)] = thermalManager.hotend_max_target(ExtUI::heater_t::H3))
  OPTCODE(HAS_HEATED_BED, MaxTemp[ARRIDX(T_Bed)] = BED_MAX_TARGET)
  OPTCODE(HAS_HEATED_CHAMBER, MaxTemp[ARRIDX(T_Chamber)] = CHAMBER_MAX_TARGET)

  // 初始化全局信息
  CMD_TASK     = 0x0000;
  CMD_MOVE     = 0x0000;
  CMD_EXTRUDER = 0x0000;
  CMD_COMMON   = 0x0000;
  FILE_INDEX   = 0x0000;
  ZERO(GCODE_INPUT);

  // 其他常量信息(不可设置)
  #if HAS_AUTO_FAN
  Percent[ARRIDX(P_FanT)] = ui8_to_percent(EXTRUDER_AUTO_FAN_SPEED);
  #endif
}

// 初始化节点 PDO
static void initNodePDO() {
  PDODisable(&CreatBotOD_Data, PDO(1));
  LOOP_S_LE_N(i, LCD_PDO_MIN_INDEX, LCD_PDO_MAX_INDEX) { PDODisable(&CreatBotOD_Data, i); }
}

void initNode() {
  // 注册回调函数
  registerODCallBackAll(INDEX_TARTEMP, subIndex_T_Count, &onTarTempSet);
  registerODCallBackOne(INDEX_TASKINFO, subIndex_Task_PrintFlowrateE0, &onFlowrateSet);
  registerODCallBackOne(INDEX_TASKINFO, subIndex_Task_PrintFlowrateE1, &onFlowrateSet);
  registerODCallBackOne(INDEX_TASKINFO, subIndex_Task_PrintFlowrateE2, &onFlowrateSet);
  registerODCallBackOne(INDEX_TASKINFO, subIndex_Task_PrintFlowrateE3, &onFlowrateSet);
  registerODCallBackOne(INDEX_TASKINFO, subIndex_Task_PrintFeedRate, &onFeedrateSet);
  registerODCallBackAll(INDEX_PERCNET, subIndex_P_Count, &onPercentSet);
  registerODCallBackAll(INDEX_STEPPERMM, subIndex_SPM_Count, &onSPMSet);
  registerODCallBackAll(INDEX_ACCELERATION, subIndex_Acc_Count, &onAccSet);
  registerODCallBackAll(INDEX_REGISTER, subIndex_Reg_Count, nullptr);
  registerODCallBackAll(INDEX_LEVELING, subIndex_Level_Count, &onLevelSet);
  registerODCallBackOne(INDEX_CMD_TASK, 0x00, &onCmdTask);
  registerODCallBackOne(INDEX_CMD_MOVE, 0x00, &onCmdMove);
  registerODCallBackOne(INDEX_CMD_EXTRUDE, 0x00, &onCmdExtrude);
  registerODCallBackOne(INDEX_CMD_COMMON, 0x00, &onCmdCommon);
  registerODCallBackOne(INDEX_FILE_INDEX, 0x00, &onFileIndex);
  registerHeartCallBack();

  initNodeInfo();  // 初始化节点常量信息
  initNodePDO();   // 初始化节点PDO信息

  canInit(&CreatBotOD_Data, CANOPEN_BAUDRATE);
  setState(&CreatBotOD_Data, Initialisation);
  setState(&CreatBotOD_Data, Operational);
}

void updateNode() {
  updateGlobalInfo();
  updateTempInfo();
  updatePrintInfo();
  updatePercentInfo();
  updatePosInfo();
  updateStepInfo();
  updateStatistics();
  updateLeveling();
}

// 通用SDO回调, 检测是否有缓存的SDO要发送
static void fisishSDOCallBack(CO_Data *d, uint8_t nodeId) {
  resetClientSDOLineFromNodeId(d, nodeId);

  // 检测是否有SDO通知要发送
  notifyLCD notifyIndex;
  if (notifyRing.pop(notifyIndex)) {
    DEBUG_ECHOLNPGM("reSendNotify");
    sendNotify(notifyIndex);
    return;
  }

  // 其他检测
}

// 发送通知
void sendNotify(notifyLCD notifyIndex) {
  DEBUG_ECHOPGM("sendNotify: ", notifyIndex, "\t");
  if (sendNotifyToLCD((uint16_t &)notifyIndex, fisishSDOCallBack)) {  // 通知发送失败, 缓存通知索引
    DEBUG_ECHOLNPGM("Cached!");
    if (!notifyRing.push(notifyIndex)) {
      DEBUG_ECHOLNPGM("NOTIFY buffer seems too small.");
    }
  } else {
    DEBUG_ECHOLNPGM("Success.");
  }
}

// 更新从节点的连接状态
static void updateSlaveState(CO_Data *d, uint8_t nodeId, bool isReady) {
  if (nodeId == LCD_NODE_ID) {
    if (LCDIsReady ^ isReady) {
      LCDIsReady = isReady;
      nodeNum += LCDIsReady ? 1 : -1;
      if (LCDIsReady) {
        // 启用屏幕相关的PDO发送
        LOOP_S_LE_N(i, LCD_PDO_MIN_INDEX, LCD_PDO_MAX_INDEX) { PDOEnable(d, i); }
        // 禁用屏幕用不到的PDO, 以节省带宽
        PDODisable(d, PDO(4));                             // 当前温度预留PDO
        PDODisable(d, PDO(7));                             // 目标温度预留PDO
        PDODisable(d, PDO(22));                            // 电机参数预留PDO
        PDODisable(d, PDO(23));                            // 电机参数预留PDO
        PDODisable(d, PDO(26));                            // 打印统计预留PDO
        PDODisable(d, PDO(29));                            // 自动调平预留PDO
        OPTCODE(DISABLE_REG_KEY, PDODisable(d, PDO(27)))   // 注册相关PDO
        OPTCODE(DISABLE_LEVELING, PDODisable(d, PDO(28)))  // 调平相关PDO
        DEBUG_ECHOLNPGM("LCD PDO enable");
      } else {
        // 禁用屏幕相关的PDO发送
        LOOP_S_LE_N(i, LCD_PDO_MIN_INDEX, LCD_PDO_MAX_INDEX) { PDODisable(d, i); }
        DEBUG_ECHOLNPGM("LCD PDO disable");
      }
    }
  }
  if (nodeNum)
    PDOEnable(d, PDO(1));
  else
    PDODisable(d, PDO(1));
}

// 更新从节点的心跳超时
static void updateSlaveHeart(CO_Data *d, UNS8 nodeId) {
  uint16_t data = 0;
  uint32_t size = 2;
  uint32_t abortCode;

  if (getReadResultNetworkDict(d, nodeId, &data, &size, &abortCode) == SDO_FINISHED) {
    uint32_t time = data * 1.5;
    NOMORE(time, 0xFFFF);
    if (time) {
      if (nodeId == LCD_NODE_ID && HEART_INDEX_LCD < d->ConsumerHeartbeatCount) {
        d->ConsumerHeartbeatEntries[HEART_INDEX_LCD] = ((uint32_t)nodeId << 16) | time;
      }
    }
    DEBUG_ECHOLNPGM("Read heartTime timeout is ", time, " , NodeID is ", nodeId);
  } else {
    DEBUG_ECHOLNPGM("Failed at read heartTime, abortCode is ", abortCode, " , NodeID is ", nodeId);
    fisishSDOCallBack(d, nodeId);
  }
}

// 注册从节点心跳相关回调函数
static void registerHeartCallBack() {
  // 从节点启动回调
  CreatBotOD_Data.post_SlaveBootup = [](CO_Data *d, uint8_t SlaveID) {
    // 获取从节点的心跳时间
    readNetworkDictCallback(d, SlaveID, 0x1017, 0x00, 0, &updateSlaveHeart, 0);
  };

  // 从节点状态改变回调
  CreatBotOD_Data.post_SlaveStateChange = [](CO_Data *d, uint8_t nodeId, e_nodeState newNodeState) {
    DEBUG_ECHOLNPGM("Node have changed, nodeID is ", nodeId, " , newNodeState is ", newNodeState);
    updateSlaveState(d, nodeId, newNodeState == Operational);
  };

  // 从节点跳超时回调
  CreatBotOD_Data.heartbeatError = [](CO_Data *d, uint8_t heartbeatID) {
    DEBUG_ECHOLNPGM("Node have left, nodeID is ", heartbeatID);
    updateSlaveState(d, heartbeatID, false);
  };
}

// 切换首次开箱
static void firstUnpack(switchVal onoff) {
  switch (onoff) {
    case switchVal::ON: {
      ExtUI::setAxisPosition_mm(Z_MAX_POS, ExtUI::axis_t::Z, 0.0, false);  // 预设Z高度为最大值
      sync_plan_position();
    } break;
    case switchVal::OFF: {
      if (!ExtUI::isAxisPositionKnown(ExtUI::axis_t::Z)) {             // Z轴位置未知
        ExtUI::setAxisPosition_mm(0.0, ExtUI::axis_t::Z, 0.0, false);  // 恢复Z高度为默认值
        sync_plan_position();
      }
    } break;
    default: break;
  }
}

// 切换电机使能
static void enableMoter(switchVal onoff) {
  switch (onoff) {
    case switchVal::ON: stepper.enable_all_steppers(); break;
    case switchVal::OFF: stepper.disable_all_steppers(); break;
    default: break;
  }
}

// 切换机箱照明
static void enableLight(switchVal onoff) {
  OPTCODE(CASE_LIGHT_ENABLE, if (onoff == ExtUI::getCaseLightState()) return)
  if (onoff == switchVal::TOGGLE) {
    OPTCODE(CASE_LIGHT_ENABLE, ExtUI::setCaseLightState(!ExtUI::getCaseLightState()))
  } else {
    OPTCODE(CASE_LIGHT_ENABLE, ExtUI::setCaseLightState(onoff))
  }
}

// 切换自动调平
static void enableAutoLeveling(switchVal onoff) {
  OPTCODE(HAS_LEVELING, if (onoff == ExtUI::getLevelingActive()) return)
  if (onoff == switchVal::TOGGLE) {
    OPTCODE(HAS_LEVELING, ExtUI::setLevelingActive(!ExtUI::getLevelingActive()))
  } else {
    OPTCODE(HAS_LEVELING, ExtUI::setLevelingActive(onoff))
  }
  settings.save();
}

// 切换喷头
static void changeExtruder(uint8_t toolIndex) {
  switch (toolIndex) {
    OPTCODE(HAS_HOTEND0, case 0 : ExtUI::setActiveTool(ExtUI::extruder_t::E0, DISABLED(DUAL_X_CARRIAGE)); break)
    OPTCODE(HAS_HOTEND1, case 1 : ExtUI::setActiveTool(ExtUI::extruder_t::E1, DISABLED(DUAL_X_CARRIAGE)); break)
    OPTCODE(HAS_HOTEND2, case 2 : ExtUI::setActiveTool(ExtUI::extruder_t::E2, DISABLED(DUAL_X_CARRIAGE)); break)
    OPTCODE(HAS_HOTEND3, case 3 : ExtUI::setActiveTool(ExtUI::extruder_t::E3, DISABLED(DUAL_X_CARRIAGE)); break)
    default: break;
  }
}

// 取消各种操作的等待 (回零, 移轴, 送丝 等)
static void stopOperateWaiting(operate action) {
  switch (action) {
    case operate::HOME: quickstop_stepper(); break;
    case operate::MOVE:
      quickstop_stepper();
      planner.cleaning_buffer_counter = 0;
      break;
    case operate::EXTRUDE:
      quickstop_stepper();
      planner.cleaning_buffer_counter = 0;
      stepper.disable_e_steppers();
      break;
    case operate::PROBE: quickstop_stepper(); break;
    case operate::SHUTDOWN:
      TERN_(POWER_OFF_WAIT_FOR_COOLDOWN, powerManager.cancelAutoPowerOff());
      // TODO if (TERN1(RECOVERY_USE_EEPROM, !recovery.check())) GotoDefault();
      break;
    default: break;
  }
}

// 设置喷头加热等待状态 (跳过加热, 重新加热)
static void setHeaterWaiting(heating action) {
  switch (action) {
    case heating::SKIP: wait_for_heatup = false; break;
    case heating::AGAIN: ExtUI::setUserConfirmed(); break;
    default: break;
  }
}

// 设置耗材加载等待状态 (继续打印, 加载更多)
static void setLoadWaiting(loading action) {
  #if M600_PURGE_MORE_RESUMABLE
  switch (action) {
    case loading::CONTINUE: ExtUI::setPauseMenuResponse(PAUSE_RESPONSE_RESUME_PRINT); break;
    case loading::MORE: ExtUI::setPauseMenuResponse(PAUSE_RESPONSE_EXTRUDE_MORE); break;
    default: break;
  }
  #else
  UNUSED(action);
  #endif
}

  #if ENABLED(SDSUPPORT)
/**
 * 刷新工作文件夹
 *    快速SDO  B1_< + B2_count + B1_isRoot
 *    分块SDO  B1_* + B2_index + B1_isDir + B4_size + B4_timeStamp + longFilename
 *    分块SDO  B1_* ...
 *    分块SDO  B1_* ...
 *    快速SDO  B1_>
 *
 * 获取当前文件名
 *    分块SDO  B1_: + longFilename
 **/
static void refreshFileToLcdCallBack(CO_Data *d, UNS8 nodeId) {
  resetClientSDOLineFromNodeId(d, nodeId);
  if ((uint16_t)(fileList.fileIndex() + 1) == fileList.count()) {
    uint8_t data = '>';  // 文件列表 尾
    sendFileInfoToLCD(1, &data, fisishSDOCallBack, false);
    DEBUG_ECHOLNPGM("refreshFileToLcd Done");
  } else {
    fileList.nextFile();          // 获取新文件名信息
    uint8_t     data[256] = {0};  // 文件列表 列表项
    const char *fname     = fileList.filename();
    data[0]               = '*';
    data[3]               = fileList.isDir();
    *(uint16_t *)&data[1] = fileList.fileIndex();
    *(uint32_t *)&data[4] = fileList.fileSize();
    *(uint32_t *)&data[8] = fileList.timeStamp();
    strcpy((char *)&data[12], fname);
    DEBUG_ECHOLNPGM("index: ", *(uint16_t *)&data[1], ",\tisDir: ", data[3], ",\tname: ", fname);
    sendFileInfoToLCD(12 + strlen(fname), data, refreshFileToLcdCallBack, true);
  }
}

static void refreshFileToLcd(fileAct action) {
  switch (action) {
    case fileAct::WORKDIR: {
      DEBUG_ECHOLNPGM("refreshFileToLcd Init");
      fileList.refresh();           // 刷新工作目录
      uint8_t data[4]       = {0};  // 文件列表 头
      data[0]               = '<';
      data[3]               = fileList.isAtRootDir();
      *(uint16_t *)&data[1] = fileList.count();
      sendFileInfoToLCD(4, data, refreshFileToLcdCallBack, false);  // 发送文件列表信息
    } break;
    case fileAct::FILENAME: {
      DEBUG_ECHOLNPGM("refreshFileToLcd curFileName");
      uint8_t     data[256] = {0};  // 文件名
      const char *fname     = fileList.filename();
      data[0]               = ':';
      strcpy((char *)&data[1], fname);
      sendFileInfoToLCD(1 + strlen(fname), data, fisishSDOCallBack, true);
    } break;
    default: break;
  }
}
  #else   //! SDSUPPORT
static void refreshFileToLcd(fileAct action) {}
  #endif  // SDSUPPORT

#endif  // CREATBOT_LINUX_LCD

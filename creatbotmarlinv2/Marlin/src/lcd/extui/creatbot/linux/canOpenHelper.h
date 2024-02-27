/**
 *  canOpenHelper.h
 *
 *  Created on: 2023/08/25
 *      Author: CreatBot LYN
 *
 *  helper for linux lcd's canOpen.
 */
#pragma once

#include "../../../../inc/MarlinConfigPre.h"
#include "../../ui_api.h"
#include "CreatBotOD.h"
#include "canOpen.h"

#if HAS_EXTRUDERS
  #define HAS_EXTRUDER0
  #if EXTRUDERS > 1
    #define HAS_EXTRUDER1
    #if EXTRUDERS > 2
      #define HAS_EXTRUDER2
      #if EXTRUDERS > 3
        #define HAS_EXTRUDER3
      #endif
    #endif
  #endif
#endif

#if HAS_HOTEND
  #define HAS_HOTEND0
  #if HOTENDS > 1
    #define HAS_HOTEND1
    #if HOTENDS > 2
      #define HAS_HOTEND2
      #if HOTENDS > 3
        #define HAS_HOTEND3
      #endif
    #endif
  #endif
#endif

#if HAS_AUTO_FAN && ENABLED(ADJUST_EXTRUDER_AUTO_FAN)
  #define AUTO_FAN_CAN_ADJUST
#endif

#if DISABLED(SUPPORT_REG_KEY)
  #define DISABLE_REG_KEY
#endif

#if DISABLED(HAS_LEVELING)
  #define DISABLE_LEVELING
#endif

// 温度范围:-300.00 ~ 1000.00; 精度:0.02度; ==> uint16 = (float + 300) * 50
#define CANOPEN_CURTEMP(index) (uint16_t)((ExtUI::getActualTemp_celsius(ExtUI::heater_t::index) + 300) * 50)
#define CANOPEN_TARTEMP(index) (uint16_t)((ExtUI::getTargetTemp_celsius(ExtUI::heater_t::index) + 300) * 50)

// 温度范围:-300.00 ~ 1000.00; 精度:0.02度; ==> float = (uint16 / 50.0) - 300
#define CANOPEN_SETTEMP(index, value) ExtUI::setTargetTemp_celsius(value / 50.0 - 300, ExtUI::heater_t::index)

// 获取激活状态
inline static uint8_t getValidState() {
#if ENABLED(SUPPORT_REG_KEY)
  return isKeyPass() ? regState::REG_VALID : isLimited() ? regState::REG_LIMIT : regState::REG_TRIAL;
#else
  return regState::REG_NONE;
#endif
}

// 获取工作状态
inline static uint8_t getWorkState() {
  if (ExtUI::isPrintingFromMedia())
    if (ExtUI::isPrintingFromMediaPaused())
      return workState::PAUSED;
    else
      return workState::PRINTING;
  else if (ExtUI::isPrintingFromSerial())
    return workState::ONLINE;
  else
    return workState::IDLE;
}

// 注册单个回调
inline static void registerODCallBackOne(uint16_t wIndex, uint8_t bSubindex, ODCallback_t callback) {
  RegisterSetODentryCallBack(&CreatBotOD_Data, wIndex, bSubindex, callback);
}

// 注册一组回调
inline static void registerODCallBackAll(uint16_t wIndex, uint8_t bCount, ODCallback_t callback) {
  for (uint8_t i = 1; i < bCount; i++) {
    RegisterSetODentryCallBack(&CreatBotOD_Data, wIndex, i, callback);
  }
}

// 发送数据到文件名缓冲区
inline static uint8_t sendFileInfoToLCD(uint32_t len, void *data, SDOCallback_t Callback, uint8_t useBlockMode) {
  return writeNetworkDictCallBack(&CreatBotOD_Data, LCD_NODE_ID, INDEX_FILE_BUFFER, 0x00,  // 屏幕文件缓存
                                  len, CanOpen_visible_string, data, Callback, useBlockMode);
}

// 发送通知索引到屏幕
inline static uint8_t sendNotifyToLCD(uint16_t &data, SDOCallback_t Callback) {
  return writeNetworkDictCallBack(&CreatBotOD_Data, LCD_NODE_ID, INDEX_NOTIFY_OUTPUT, 0x00,  // 屏幕通知缓存
                                  2, 0, &data, Callback, 0);
}

inline static uint8_t percent_to_ui8(uint8_t percent) { return map(constrain(percent, 0, 100), 0, 100, 0, 255); }

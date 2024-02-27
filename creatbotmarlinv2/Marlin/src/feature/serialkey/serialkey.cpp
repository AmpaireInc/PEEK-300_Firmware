/*
 * Serialkey.cpp
 *
 *  Created on: Dec 21, 2022
 *      Author: CreatBot-SW
 */

#include "../../inc/MarlinConfigPre.h"

#if ENABLED(SUPPORT_REG_KEY)

  #include "serialkey.h"
  #include "../../module/settings.h"  //获取EEPROM数据

  #define DEBUG_OUT ENABLED(DEBUG_SERIALKEY)
  #include "../../core/debug_out.h"

Serialkey serialkye;

uint8_t Serialkey::seq[6]    = {0};  // 内部计算用
char    Serialkey::seq_c[13] = {0};  // 外部显示用
float   Serialkey::regKey    = 0.0;  // 内部计算用
float   Serialkey::RegSN     = 0.0;  // 外部读取和输入

void Serialkey::initSequence() {
  if (getChipID()) {
    getSequence(seq_c);
  }
  loadRegsn();
}

bool Serialkey::getChipID() {
  u_chipId chipId;
  uint8_t  seq_lot = 0;  // 芯片ID批号 位与结果

  // 读取芯片96位唯一ID
  chipId.chipId_32[0] = *(uint32_t *)(DEVICE_ID_BASE_ADDERSS);
  chipId.chipId_32[1] = *(uint32_t *)(DEVICE_ID_BASE_ADDERSS + 4);
  chipId.chipId_32[2] = *(uint32_t *)(DEVICE_ID_BASE_ADDERSS + 8);

  // 计算芯片批号掩码
  for (uint8_t i = 5; i < 12; i++) {  // 芯片ID批号 高7个字节位与运算
    seq_lot += chipId.chipId_8[i];
  }

  // 取得芯片ID
  seq[0] = chipId.chipId_8[0];  // 晶元坐标X/Y, 16位, 高位总是0x00
  seq[1] = chipId.chipId_8[2];  // 晶元坐标X/Y, 16位, 高位总是0x00
  seq[2] = chipId.chipId_8[4];  // 晶元流水号, 8位
  seq[3] = seq_lot ^ seq[0];
  seq[4] = seq_lot ^ seq[1];
  seq[5] = seq_lot ^ seq[2];

  #if ENABLED(DEBUG_SERIALKEY)
  SERIAL_PRINTLN(seq[0], PrintBase::Hex);
  SERIAL_PRINTLN(seq[1], PrintBase::Hex);
  SERIAL_PRINTLN(seq[2], PrintBase::Hex);
  SERIAL_PRINTLN(seq[3], PrintBase::Hex);
  SERIAL_PRINTLN(seq[4], PrintBase::Hex);
  SERIAL_PRINTLN(seq[5], PrintBase::Hex);
  #endif

  return true;
}

float Serialkey::generateReg(uint32_t seqInt) {
  u_reg reg;
  reg.reg_32 = seqInt;
  reg.reg_32 ^= (REG_PLULIC_KEY);

  // 算法 生成 有效数字 和 指数尾数
  uint16_t fraction =
      ((uint16_t)(reg.reg_8[0] ^ reg.reg_8[1] ^ reg.reg_8[2] ^ reg.reg_8[3]) << 8) + (reg.reg_8[0] ^ reg.reg_8[2]);

  // 算法生成指数尾数(0x00, 0x01, 0x02)
  uint8_t powerSuffix = ((uint8_t)(reg.reg_8[0] + reg.reg_8[1] + reg.reg_8[2] + reg.reg_8[3])) % 3;
  // 指数基数, 保证指数在13到15之间，从而保证浮点数范围为8192到65536之间
  uint8_t powerBase = 0x8C;

  reg.reg_8[3]      = (powerBase + powerSuffix) >> 1;  // 符号位为正，保证浮点数为正数
  reg.reg_8[2]      = (powerSuffix << 7) | ((fraction >> (6 + powerSuffix)) & 0x7F);
  reg.reg_8[1] = (fraction & 0xFF) << (2 - powerSuffix);  // 左移，保证浮点数的小数位全为0，即整数表达
  reg.reg_8[0] = 0x00;                                    // 保证浮点数的小数位全为0，即整数表达

  if (reg.reg_f < 10000) {
    reg.reg_f = reg.reg_f + 10000.0;
  }

  reg.reg_f = reg.reg_f / 100.0;

  return reg.reg_f;
}

void Serialkey::getSequence(char *seqBuf) {
  u_reg regH, regL;

  regL.reg_8[0] = seq[0];
  regL.reg_8[1] = seq[1];
  regL.reg_8[2] = seq[2];
  regL.reg_8[3] = seq[3];
  regH.reg_8[0] = seq[4];
  regH.reg_8[1] = seq[5];

  // 计算注册码
  regKey = generateReg(regL.reg_32);
  DEBUG_ECHOLNPGM("regkey: ", regKey);

  // 生成可读的序列号
  sprintf(seqBuf, "%04X%08X", regH.reg_32, regL.reg_32);
  DEBUG_ECHOLN(seqBuf);
}

bool Serialkey::checkReg() {
  if (regKey > 0) {
    return (regKey == RegSN);
  } else {
    return false;
  }
}

void Serialkey::saveRegsn() {
  if (persistentStore.access_start()) {
    persistentStore.write_data(REGSN_EEPROM_OFFSET, (const uint8_t *)&serialkey.RegSN, sizeof(serialkey.RegSN));
  }
  persistentStore.access_finish();
}

void Serialkey::loadRegsn() {
  if (persistentStore.access_start()) {
    persistentStore.read_data(REGSN_EEPROM_OFFSET, (uint8_t *)&serialkey.RegSN, sizeof(serialkey.RegSN));
  }
  persistentStore.access_finish();
}

void Serialkey::clearRegsn() {
  RegSN = 0.0f;
  saveRegsn();
}

#endif  // SUPPORT_REG_KEY
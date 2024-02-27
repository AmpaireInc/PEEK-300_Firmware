/*
 * serialkey.h
 *
 *  Created on: Dec 21, 2022
 *      Author: CreatBot-SW
 */

#ifndef SERIALKEY_H_
#define SERIALKEY_H_

// #define DEBUG_SERIALKEY

#ifndef REG_PLULIC_KEY
  #define REG_PLULIC_KEY 0xFFFFFFFF
#endif

// Unique device ID register base adderss STM32F446
#define DEVICE_ID_BASE_ADDERSS 0X1FFF7A10

// support max 128bit chipID
// DS2401 use 64 bit
// STM32 use 96 bit
union u_chipId {
  uint8_t  chipId_8[16];
  uint32_t chipId_32[4];
};

union u_reg {
  uint8_t  reg_8[4];
  uint32_t reg_32;
  float    reg_f;
};

class Serialkey {
 private:
  static float   regKey;
  static uint8_t seq[6];

  /**
   * Get the sequence int.
   * Return true if success get the seq.
   */
  static bool getChipID();

  /**
   * Generate registration number.
   */
  static float generateReg(uint32_t seqInt);

  /**
   * Get the sequence.
   * @seq	the addr of to save. Bigger than 12.
   * Return true if success get the seq.
   */
  static void getSequence(char* seq);

 public:
  static float RegSN;
  static char  seq_c[13];

  /**
   * Init the sequence function.
   */
  static void initSequence();

  /**
   * Check the registration code that input from user.
   * Return if the code is right.
   */
  static bool checkReg();

  /**
   * Save the registration key.
   */
  static void saveRegsn();

  /**
   * Loading the registration key.
   */
  static void loadRegsn();

  /**
   * Clear the registration key.
   */
  static void clearRegsn();
};

extern Serialkey serialkey;

#endif /* Serialkey_H_ */

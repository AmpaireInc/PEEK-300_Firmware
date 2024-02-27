
功能关键字|功能解释|状态|功能备注|硬件备注  
---|---|---|---|---
FASTIO_INIT|针脚初始化|✔️||
BOARD_PREINIT|主板针脚预设|❌||专板专用
tmc_standby_setup|tmc待机针脚初始化|❌||部分TMC驱动用
HAL_get_reset_source|获取启动寄存器标示|✔️||
MARLIN_DEV_MODE|配置开发模式串口函数|✔️||
MYSERIAL1.begin(BAUDRATE)|串口初始化|✔️||
HAS_MULTI_SERIAL|多串口初始化|⚠️️|未开启|WIFI串口复用
SERIAL_ECHO|打印输出 start|✔️||
HAS_KILL|配置关机输入针脚|✔️||
HAS_FREEZE_PIN|配置冻结输入针脚|❌||无针脚
HAS_SUICIDE|配置自杀输出针脚|✔️||
JTAGSWD_RESET|配置JTAG调试|❌||无针脚
DYNAMIC_VECTORTABLE|动态向量表|❌|功能存疑|
HAL_init|硬件库针脚初始化(状态灯, 定时器优先级等)|✔️||
TEMP_CS_PIN|SPI温控器初始化|⚠️️|未开启|
DUET_SMART_EFFECTOR|DUET特殊针脚|❌||专板专用
HAS_FILAMENT_SENSOR|耗材检测传感器初始化|⚠️️|代码移植与合并|
HAS_TMC220x|TMC220X系列串口初始化|❌||TMC2208/9专用
PSU_CONTROL|PSU电源控制|❌|功能存疑|
POWER_LOSS_RECOVERY|断电恢复|⚠️️|代码移植与合并|
HAS_L64XX|L64系列驱动初始化|❌||L64驱动专用
HAS_STEPPER_RESET|去能 带RESET针脚的驱动|❌|不需要|部分驱动专用
HAS_TMC_SPI|TMC的SPI初始化|⚠️️|未开启|
BOARD_INIT|主板初始化|❌||专板专用
ESP_WIFI_MODULE|esp wifi 初始化|❌|功能存疑|无针脚
SERIAL_ECHO|打印重启原因|✔️||
SERIAL_ECHO|打印启动信息|✔️||
calibrate_delay_loop|矫正ARM芯片的Delay函数|✔️||
USE_BEEPER|蜂鸣器初始化|✔️️||
HAS_COLOR_LEDS<br>NEOPIXEL2_SEPARATE|灯光初始化|⚠️️|未开启|WS2812
USE_CONTROLLER_FAN|风扇控制器初始化(驱动散热风扇)|⚠️️|未开启|系统风扇
ui.init()|UI初始化(包括点阵屏的开机画面)|⚠️️|代码移植与合并|
SAFE_POWER|驱动电流检测|⚠️️|未开启|部分TMC驱动?
PROBE_TARE|压敏探针初始化|❌|功能存疑|针脚存疑
SDSUPPORT<br>SDCARD_EEPROM_EMULATION|SD卡初始化(EEPROM仿真)|❌|不需要|无针脚
settings.first_load()|加载配置|✔️||
HAS_ETHERNET|网卡初始化(网口转串口方案?)|❌|功能存疑|针脚存疑
HAS_TOUCH_BUTTONS|触摸按键初始化|❌|不需要|无针脚
HAS_M206_COMMAND|工作区偏移|✔️||
sync_plan_position|同步电机位置|✔️||
thermalManager.init()|温控管理初始化|✔️||
print_job_timer.init()|打印任务定时器初始化|✔️||
endstops.init()|零位初始化|✔️||
stepper.init()|步进电机初始化|✔️||
HAS_SERVOS|舵机初始化|✔️||
HAS_Z_SERVO_PROBE|Z舵机初始化|⚠️️|功能存疑|
HAS_PHOTOGRAPH|相机初始化|⚠️️|未开启|无针脚
HAS_CUTTER|激光刀初始化|❌|CNC相关|无针脚
COOLANT_MIST|冷却雾初始化|❌|CNC相关?|无针脚
COOLANT_FLOOD|冷却流初始化|❌|CNC相关?|无针脚
HAS_BED_PROBE|床探针初始化|⚠️️|功能存疑|
HAS_STEPPER_RESET|使能 带RESET针脚的驱动|❌|不需要|部分驱动专用
HAS_MOTOR_CURRENT_I2C|I2C 调节电流 初始化|⚠️️|功能存疑|
HAS_MOTOR_CURRENT_DAC|DAC 调节电流 初始化|❌||针脚冲突
HAS_SOLENOID_1|电磁探针初始化|⚠️️|功能存疑|
HAS_HOME|HOME针脚初始化|❌|不需要|无针脚
CUSTOM_USER_BUTTONS|自定义按键初始化|❌|不需要|无针脚
STAT_LED_RED<br>STAT_LED_BLUE|加热指示灯初始化(红蓝灯)|❌|不需要|无针脚
CASE_LIGHT_ENABLE|机箱照明灯初始化|⚠️️|未开启|无针脚
HAS_PRUSA_MMU1|多材料单元驱动器|⚠️️|未开启|
HAS_FANMUX|多路复用风扇初始化|❌|不需要|无针脚
MIXING_EXTRUDER|混合喷头初始化|⚠️️|未开启|
BLTOUCH|BLTouch初始化|✔️||
I2C_POSITION_ENCODERS|I2C编码器(闭环控制)|⚠️️|未开启|
EXPERIMENTAL_I2CBUS|实验用I2C总线|⚠️️|未开启|
DO_SWITCH_EXTRUDER|切换挤出机初始化|⚠️️|功能存疑|
SWITCHING_NOZZLE|切换喷头初始化|⚠️️|待开启|
PARKING_EXTRUDER|停靠喷头初始化(电磁, 磁吸 等)|⚠️️|功能存疑|
USE_WATCHDOG|看门狗初始化|✔️||
EXTERNAL_CLOSED_LOOP_CONTROLLER|外部闭环控制器初始化|⚠️️|未开启|无针脚
STARTUP_COMMANDS|启动命令注入|⚠️️|未开启|
HOST_PROMPT_SUPPORT|主机提示初始化|⚠️️|未开启|
HAS_TRINAMIC_CONFIG|测试TMC驱动器连接|⚠️️|未开启|
HAS_DRIVER_SAFE_POWER_PROTECT|驱动电流报告|⚠️️|未开启|
HAS_PRUSA_MMU2|二喷头多材料单元驱动器|⚠️️|未开启|
IIC_BL24CXX_EEPROM|I2C通讯EEPROM初始化(BL24系列)|❌️|不需要|专板专用
HAS_DWIN_E3V2_BASIC|DWIN屏幕E3V2|❌|不需要|专板专用
MAX7219_DEBUG|MAX7219调试(8*8矩阵LED)|❌|功能存疑|无针脚
DIRECT_STEPPING|DIRECT_STEPPING(一种控制算法)|⚠️️|功能存疑|
HAS_TFT_LVGL_UI|lvgl库TFT屏幕 初始化|⚠️️|未开启|无针脚
SHOW_BOOTSCREEN|点阵屏的开机画面延时结束|❌|不需要|
PASSWORD_ON_STARTUP|开机密码|⚠️️|未开启|
TOUCH_SCREEN_CALIBRATION|触摸屏校准|⚠️️|未开启|基于TFT
EASYTHREED_UI|EasyThreeD屏幕初始化|❌|不需要|专板专用


/*
@module  mcu
@summary 封装mcu一些特殊操作
@version core V0007
@date    2021.08.18
@tag LUAT_USE_MCU
*/
#include "luat_base.h"
#include "luat_mcu.h"
#include "luat_zbuff.h"
#include "luat_spi.h"
#define LUAT_LOG_TAG "mcu"
#include "luat_log.h"
/*
设置主频,单位MHZ
@api mcu.setClk(mhz)
@int 主频,根据设备的不同有不同的有效值,请查阅手册
@return bool 成功返回true,否则返回false
@usage
--  请注意,主频与外设主频有关联性, 例如主频2M时SPI的最高只能1M
-- 设置到80MHZ
mcu.setClk(80)
sys.wait(1000)
-- 设置到240MHZ
mcu.setClk(240)
sys.wait(1000)
-- 设置到2MHZ
mcu.setClk(2)
sys.wait(1000)
*/
static int l_mcu_set_clk(lua_State* L) {
    int ret = luat_mcu_set_clk((size_t)luaL_checkinteger(L, 1));
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
获取主频,单位MHZ
@api mcu.getClk()
@return int 若失败返回-1,否则返回主频数值,若等于0,可能处于32k晶振的省电模式
@usage
local mhz = mcu.getClk()
print("Boom", mhz)
*/
static int l_mcu_get_clk(lua_State* L) {
    int mhz = luat_mcu_get_clk();
    lua_pushinteger(L, mhz);
    return 1;
}

/*
获取设备唯一id. 注意,可能包含不可见字符,如需查看建议toHex()后打印
@api mcu.unique_id()
@return string 设备唯一id.若不支持, 会返回空字符串.
@usage
local unique_id = mcu.unique_id()
print("unique_id", unique_id)
*/
static int l_mcu_unique_id(lua_State* L) {
    size_t len = 0;
    const char* id = luat_mcu_unique_id(&len);
    lua_pushlstring(L, id, len);
    return 1;
}

/*
获取启动后的tick数,注意会出现溢出会出现负数
@api mcu.ticks()
@return int 当前tick值
@usage
local tick = mcu.ticks()
print("ticks", tick)
*/
static int l_mcu_ticks(lua_State* L) {
    long tick = luat_mcu_ticks();
    lua_pushinteger(L, tick);
    return 1;
}

/*
获取每秒的tick数量
@api mcu.hz()
@return int 每秒的tick数量,通常为1000
@usage
local hz = mcu.hz()
print("mcu.hz", hz)
*/
static int l_mcu_hz(lua_State* L) {
    uint32_t hz = luat_mcu_hz();
    lua_pushinteger(L, hz);
    return 1;
}

/*
读写mcu的32bit寄存器或者ram,谨慎使用写功能,请熟悉mcu的寄存器使用方法后再使用
@api mcu.reg32(address, value, mask)
@int 寄存器或者ram地址
@int 写入的值,如果没有,则直接返回当前值
@int 位掩码,可以对特定几个位置的bit做修改, 默认0xffffffff,修改全部32bit
@return int 返回当前寄存的值
@usage
local value = mcu.reg32(0x2009FFFC, 0x01, 0x01) --对0x2009FFFC地址上的值,修改bit0为1
*/
static int l_mcu_reg32(lua_State* L) {
    volatile uint32_t *address = (uint32_t *)(luaL_checkinteger(L, 1) & 0xfffffffc);
    if (lua_isinteger(L, 2)) {
    	volatile uint32_t value = lua_tointeger(L, 2);
    	volatile uint32_t mask = luaL_optinteger(L, 3, 0xffffffff);
    	volatile uint32_t org = *address;
    	*address = (org & ~mask)| (value & mask);
    	lua_pushinteger(L, *address);
    } else {
    	lua_pushinteger(L, *address);
    }
    return 1;
}

/*
转换10进制数为16进制字符串输出
@api mcu.x32(value)
@int 需要转换的值
@return string 16进制字符串
@usage
local value = mcu.x32(0x2009FFFC) --输出"0x2009fffc"
*/
static int l_mcu_x32(lua_State* L) {
    uint32_t value = luaL_checkinteger(L, 1);
    char c[16];
    sprintf_(c, "0x%x", value);
    lua_pushstring(L, c);
    return 1;
}

// #ifdef __LUATOS_TICK_64BIT__
/*
获取启动后的高精度tick，如果支持bit64库，可以直接输出转换好的bit64结构
@api mcu.tick64()
@boolean 是否输出bit64结构,true是,其他都是false,留空也是false,用于兼容旧的demo
@return string 当前tick值,8个字节的uint64,如果支持64bit库,同时要求输出64bit结构的话,会输出9字节的string
@return int 1us有几个tick,0表示未知
@usage
local tick_str, tick_per = mcu.tick64()
print("ticks", tick_str, tick_per)
*/
static int l_mcu_hw_tick64(lua_State* L) {

    uint64_t tick = luat_mcu_tick64();
    uint32_t us_period = luat_mcu_us_period();
#ifdef LUAT_USE_BIT64
	if (lua_isboolean(L, 1) && lua_toboolean(L, 1))
	{
		uint8_t data[9] = {0};
		memcpy(data, &tick, 8);
		lua_pushlstring(L, (const char*)data, 9);
	}
#else
    lua_pushlstring(L, (const char*)&tick, 8);
#endif
    lua_pushinteger(L, us_period);
    return 2;
}

/*
计算2个64bit tick的差值
@api mcu.dtick64(tick1, tick2, check_value)
@string 64bit的string
@string 64bit的string
@int 参考值,可选项,如果为0,则返回结果中第一个项目为true
@return boolean 与参考值比较,如果大于等于为true,反之为false
@return int 差值tick1 - tick2,如果超过了0x7fffffff,结果可能是错的
@usage
local result, diff_tick = mcu.dtick64(tick1, tick2)
print("ticks", result, diff_tick)
*/
static int l_mcu_hw_diff_tick64(lua_State* L) {
	uint64_t tick1, tick2;
	int64_t diff;
	int check_value = 0;
    size_t len1;
    const char *data1 = luaL_checklstring(L, 1, &len1);
    size_t len2;
    const char *data2 = luaL_checklstring(L, 2, &len2);
    check_value = luaL_optinteger(L, 3, 0);

    memcpy(&tick1, data1, len1);
    memcpy(&tick2, data2, len2);
    diff = tick1 - tick2;
    lua_pushboolean(L, (diff >= (int64_t)check_value)?1:0);
    lua_pushinteger(L, diff);
    return 2;
}

/*
选择时钟源,当前仅air105支持
@api mcu.setXTAL(source_main, source_32k, delay)
@boolean 高速时钟是否使用外部时钟源,如果为空则不改变
@boolean 低速32K是否使用外部时钟源,如果为空则不改变
@int PLL稳定时间,在切换高速时钟的时候,根据硬件环境,需要delay一段时间等待PLL稳定,默认是1200,建议不小于1024
@usage
mcu.setXTAL(true, true, 1248)	--高速时钟使用外部时钟,低速32K使用外部晶振, delay1248
*/
static int l_mcu_set_xtal(lua_State* L) {
	int source_main = 255;
	int source_32k = 255;
	int delay = luaL_optinteger(L, 3, 1200);
	if (lua_isboolean(L, 1)) {
		source_main = lua_toboolean(L, 1);
	}
	if (lua_isboolean(L, 2)) {
		source_32k = lua_toboolean(L, 2);
	}
	luat_mcu_set_clk_source(source_main, source_32k, delay);
    return 0;
}
// #endif
LUAT_WEAK void luat_mcu_set_hardfault_mode(int mode) {;}
/*
mcu死机时处理模式，目前只有EC618平台适用
@api mcu.hardfault(mode)
@int 处理模式，0死机停机，1死机后重启，2死机后尽量将错误信息提交给外部工具后重启
@usage
mcu.hardfault(0)	--死机后停机，一般用于调试状态
mcu.hardfault(1)	--死机后重启，一般用于正式产品
mcu.hardfault(2)	--死机后尽量将错误信息提交给外部工具后重启，一般用于压力测试或者正式产品
*/
static int l_mcu_set_hardfault_mode(lua_State* L)
{
	luat_mcu_set_hardfault_mode(luaL_optinteger(L, 1, 0));
	return 0;
}

LUAT_WEAK int luat_uart_pre_setup(int uart_id, uint8_t use_alt_type){return -1;}
LUAT_WEAK int luat_i2c_set_iomux(int id, uint8_t value){return -1;}
/*
在外设打开前，将外设IO复用到非默认配置上，目前只支持Air780E的部分外设复用到其他配置，这是一个临时接口，如果后续有更合适的api，本接口将不再更新
@api mcu.iomux(type, channel, value)
@int 外设类型，目前只有mcu.UART,mcu.I2C
@int 总线序号，0~N，
@int 新的配置，这个需要根据具体平台决定
@usage
mcu.iomux(mcu.UART, 2, 1)	-- Air780E的UART2复用到gpio12和gpio13(Air780EG默认是这个复用，不要动)
mcu.iomux(mcu.UART, 2, 2)	-- Air780E的UART2复用到gpio6和gpio7
mcu.iomux(mcu.I2C, 0, 1)	-- Air780E的I2C0复用到gpio12和gpio13
mcu.iomux(mcu.I2C, 0, 2)	-- Air780E的I2C0复用到gpio16和gpio17
mcu.iomux(mcu.I2C, 1, 1)	-- Air780E的I2C1复用到gpio4和gpio5
 */
static int l_mcu_iomux(lua_State* L)
{
	int type = luaL_optinteger(L, 1, 0xff);
	int channel = luaL_optinteger(L, 2, 0xff);
	int value = luaL_optinteger(L, 3, 0);
	LLOGD("mcu iomux %d,%d,%d", type, channel, value);
	switch(type)
	{
	case LUAT_MCU_PERIPHERAL_UART:
		luat_uart_pre_setup(channel, value);
		break;
	case LUAT_MCU_PERIPHERAL_I2C:
		luat_i2c_set_iomux(channel, value);
		break;
	}
	return 0;
}
#include "rotable2.h"
static const rotable_Reg_t reg_mcu[] =
{
    { "setClk" ,        ROREG_FUNC(l_mcu_set_clk)},
    { "getClk",         ROREG_FUNC(l_mcu_get_clk)},
    { "unique_id",      ROREG_FUNC(l_mcu_unique_id)},
    { "ticks",          ROREG_FUNC(l_mcu_ticks)},
    { "hz",             ROREG_FUNC(l_mcu_hz)},
	{ "reg32",          ROREG_FUNC(l_mcu_reg32)},
	{ "x32",             ROREG_FUNC(l_mcu_x32)},
// #ifdef __LUATOS_TICK_64BIT__
	{ "tick64",			ROREG_FUNC(l_mcu_hw_tick64)},
	{ "dtick64",		ROREG_FUNC(l_mcu_hw_diff_tick64)},
	{ "setXTAL",		ROREG_FUNC(l_mcu_set_xtal)},
	{ "hardfault",	ROREG_FUNC(l_mcu_set_hardfault_mode)},
	{ "iomux",	ROREG_FUNC(l_mcu_iomux)},
// #endif
	//@const UART number 外设类型-串口
	{ "UART",             ROREG_INT(LUAT_MCU_PERIPHERAL_UART) },
	//@const I2C number 外设类型-I2C
	{ "I2C",             ROREG_INT(LUAT_MCU_PERIPHERAL_I2C) },
	//@const SPI number 外设类型-SPI
	{ "SPI",             ROREG_INT(LUAT_MCU_PERIPHERAL_SPI) },
	{ NULL,             ROREG_INT(0) }
};

LUAMOD_API int luaopen_mcu( lua_State *L ) {
    luat_newlib2(L, reg_mcu);
    return 1;
}

/* 
* drivers/input/touchscreen/ftxxxx_ex_fun.c
*
* FocalTech ftxxxx expand function for debug. 
*
* Copyright (c) 2014  Focaltech Ltd.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*Note:the error code of EIO is the general error in this file.
*/


#include "ftxxxx_ex_fun.h"
#include "ftxxxx_ts.h"
#include "test_lib.h"
#include <linux/kernel.h>
#include <linux/mount.h>
#include <linux/netdevice.h>
#include <linux/unistd.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
/*
static unsigned char JT_TM_FW[] = {
#include "ASUS_ZE552KL_JT_TM_app.i"
};
static unsigned char GIS_AUO_FW[] = {
#include "ASUS_ZE552KL_GIS_AUO_app.i"
};
static unsigned char JT_AUO_FW[] = {
#include "ASUS_ZE552KL_JT_AUO_app.i"
};
static unsigned char GIS_TM_FW[] = {
#include "ASUS_ZE552KL_GIS_TM_app.i"
};*/
/**/
/*static unsigned char GIS_TM1_FW[] = {
#include "ASUS_LIBRA_GIS_TM_app.i"
};
static unsigned char BIEL_CTC_FW[] = {
#include "ASUS_LIBRA_BIEL_CTC_app.i"
};
static unsigned char BIEL_TM_FW[] = {
#include "ASUS_LIBRA_BIEL_TM_app.i"
};
static unsigned char JT_TM1_FW[] = {
#include "ASUS_LIBRA_JT_TM_app.i"
};
*/

/**/

/*ASUS_BSP Jacob : setting priority +++ */
#ifdef ASUS_FACTORY_BUILD
#define Focal_RW_ATTR (S_IRUGO | S_IWUGO)
#define Focal_WO_ATTR (S_IWUGO)
#define Focal_RO_ATTR (S_IRUGO)
#else
#define Focal_RW_ATTR (S_IRUGO|S_IWUSR)
#define Focal_WO_ATTR (S_IWUSR | S_IWGRP)
#define Focal_RO_ATTR (S_IRUGO)
#endif
u8 forcetouchreg[4][37];
u8 FT_value[4];
static int FT_READ=true;
int ft_data_flag=0;
u32 forcesensorreg[18];
#define FORCE_WORK_REG    0x00
#define FORCE_ERASE_REG   0x07
#define FORCE_CHECK_REG  0x16
#define FORCE_READ_REG    0x48
#define FORCE_WRITE_REG   0x14


/*ASUS_BSP Jacob : setting priority --- */
#define FTS_SELF_TEST
extern int focal_init_success;
extern struct ftxxxx_ts_data *ftxxxx_ts;
extern u8 B_VenderID;
extern u8 F_VenderID;
extern char B_projectcode[8];
extern u8 F_projectcode;
extern bool FOCAL_IRQ_DISABLE;
extern u8 FTS_gesture_register_d2;
extern u8 FTS_gesture_register_d5;
extern u8 FTS_gesture_register_d6;
extern u8 FTS_gesture_register_d7;
extern u8 g_vendor_id;
extern int HD_Modle;
extern bool ReConfigDoubleTap;
extern u8 g_touch_slop;
extern u8 g_touch_distance;
extern u8 g_time_gap;
extern u8 g_int_time;

extern bool EnableProximityCheck;

static bool fw_update_complete;
static int fw_update_progress;
static int fw_update_total_count = 100;

static int fw_size = 0;

static struct kobject *android_touch_kobj;		/* Sys kobject variable*/

int g_asus_tp_raw_data_flag;
short g_RXNum;
short g_TXNum;

/*zax 20141116 ++++++++++++++*/
//zax 20150112++++++++++++++++++++++++++++++++++
extern short TxLinearity[TX_NUM_MAX][RX_NUM_MAX];
extern short RxLinearity[TX_NUM_MAX][RX_NUM_MAX];
extern int total_item;
//zax 20150112----------------------------------
int TPrawdata[TX_NUM_MAX][RX_NUM_MAX];
#ifdef FTS_SELF_TEST
int TX_NUM;
int RX_NUM;
int SCab_1;
int SCab_2;
int SCab_3;
int SCab_4;
int SCab_5;
int SCab_6;
int SCab_7;
int SCab_8;
static int TP_TEST_RESULT;
#endif
/*zax 20141116 -------------------*/
int HidI2c_To_StdI2c(struct i2c_client *client)
{
	u8 auc_i2c_write_buf[10] = {0};
	u8 reg_val[10] = {0};
	int iRet = 0;

	auc_i2c_write_buf[0] = 0xEB;
	auc_i2c_write_buf[1] = 0xAA;
	auc_i2c_write_buf[2] = 0x09;

	reg_val[0] = reg_val[1] =  reg_val[2] = 0x00;
	iRet = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 3);

	msleep(10);
	iRet = ftxxxx_i2c_Read(client, auc_i2c_write_buf, 0, reg_val, 3);
	FTS_DBG("[Touch] %s : Change to STDI2cValue,REG1 = 0x%x,REG2 = 0x%x,REG3 = 0x%x, iRet=%d\n", __func__, reg_val[0], reg_val[1], reg_val[2], iRet);

	if (reg_val[0] == 0xEB && reg_val[1] == 0xAA && reg_val[2] == 0x08) {
		printk("[Focal][Touch] %s : HidI2c_To_StdI2c successful. \n", __func__);
		iRet = 1;
	} else {
		printk("[Focal][TOUCH_ERR] %s : HidI2c_To_StdI2c error. \n", __func__);
		iRet = 0;
	}

	return iRet;
}

struct Upgrade_Info{
	u16		delay_aa;		/*delay of write FT_UPGRADE_AA*/
	u16		delay_55;		/*delay of write FT_UPGRADE_55*/
	u8		upgrade_id_1;	/*upgrade id 1*/
	u8		upgrade_id_2;	/*upgrade id 2*/
	u16		delay_readid;	/*delay of read id*/
};

struct Upgrade_Info upgradeinfo;
struct i2c_client *G_Client;

int ftxxxx_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return ftxxxx_i2c_Write(client, buf, sizeof(buf));
}

int ftxxxx_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue)
{
	return ftxxxx_i2c_Read(client, &regaddr, 1, regvalue, 1);
}

int FTS_I2c_Read(unsigned char *wBuf, int wLen, unsigned char *rBuf, int rLen)
{
	if (NULL == G_Client) {
		return -1;
	}

	return ftxxxx_i2c_Read(G_Client, wBuf, wLen, rBuf, rLen);
}

int FTS_I2c_Write(unsigned char *wBuf, int wLen)
{
	if (NULL == G_Client) {
		return -1;
	}

	return ftxxxx_i2c_Write(G_Client, wBuf, wLen);
}

int fts_ctpm_auto_clb(struct i2c_client *client)
{
	unsigned char uc_temp;
	unsigned char i;

	/*start auto CLB*/
	msleep(200);
	ftxxxx_write_reg(client, 0, 0x40);
	msleep(100);	/*make sure already enter factory mode*/
	ftxxxx_write_reg(client, 2, 0x4);	/*write command to start calibration*/ /*asus: write 0x04 to 0x04 ???*/
	msleep(300);

	for (i = 0; i < 100; i++) {
		ftxxxx_read_reg(client, 0, &uc_temp);
		if (0x0 == ((uc_temp&0x70)>>4)) {/*return to normal mode, calibration finish*/	/*asus: auto/test mode auto switch ???*/
			break;
		}
		msleep(20);
	}

	/*calibration OK*/
	ftxxxx_write_reg(client, 0, 0x40);	/*goto factory mode for store*/
	msleep(200);	/*make sure already enter factory mode*/
	ftxxxx_write_reg(client, 2, 0x5);	/*store CLB result*/	/*asus: write 0x05 to 0x04 ???*/
	msleep(300);
	ftxxxx_write_reg(client, 0, 0x0);	/*return to normal mode*/
	msleep(300);
	/*store CLB result OK*/

	return 0;
}

/*
upgrade with *.i file
*/
int fts_ctpm_fw_upgrade_with_i_file(struct i2c_client *client, u8 *fw_buf)
{
	u8 *pbt_buf = NULL;
	int i_ret;

	/*judge the fw that will be upgraded
	* if illegal, then stop upgrade and return.
	*/
	if (fw_size < 8 || fw_size > 54*1024) {
		printk("[Focal][TOUCH_ERR] %s : FW length error \n", __func__);
		return -EIO;
	}

	/*FW upgrade*/
	pbt_buf = fw_buf;
	/*call the upgrade function*/
	i_ret = fts_ctpm_fw_upgrade(client, pbt_buf, fw_size);
	if (i_ret != 0) {
		dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : upgrade failed. err=%d.\n", __func__, i_ret);
	} else {
#ifdef AUTO_CLB
		fts_ctpm_auto_clb(client);  /*start auto CLB*/
#endif
	}
	return i_ret;
}

u8 fts_ctpm_get_i_file_ver(u8 *fw_buf)
{
	printk("[Focal][Touch] %s : Update FW size = %d \n", __func__, fw_size);
	if (fw_size > 2) {
		return fw_buf[fw_size - 2];
	} else {
		return 0x00;	/*default value*/
	}
}
u8 fts_ctpm_get_i_file_vid(u8 *fw_buf)
{
	printk("[Focal][Touch] %s : Update FW size = %d \n", __func__, fw_size);
	if (fw_size > 2) {
		return fw_buf[fw_size - 1];
	} else {
		return 0x00;	/*default value*/
	}
}

int fts_ctpm_auto_upgrade(struct i2c_client *client, u8 *fw_buf)
{
	u8 uc_host_fm_ver = FTXXXX_REG_FW_VER;
	u8 uc_host_fm_vid, uc_tp_fm_ver;
	u8 uc_tp_fm_vid;
	int i_ret;
	ftxxxx_read_reg(client,FTXXXX_REG_VENDOR_ID,&uc_tp_fm_vid);
	ftxxxx_read_reg(client, FTXXXX_REG_FW_VER, &uc_tp_fm_ver);
	uc_host_fm_ver = fts_ctpm_get_i_file_ver(fw_buf);
	uc_host_fm_vid=fts_ctpm_get_i_file_vid(fw_buf);
	printk("[Focal][Touch] %s : uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x \n", __func__, uc_tp_fm_ver, uc_host_fm_ver);
	printk("[Focal][Touch] %s : uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x \n", __func__, uc_tp_fm_vid, uc_host_fm_vid);
	if (uc_tp_fm_ver == FTXXXX_REG_FW_VER ||	/*the firmware in touch panel maybe corrupted*/
		uc_tp_fm_ver != uc_host_fm_ver ||		/*the firmware in host flash is new, need upgrade*/
		uc_tp_fm_vid != uc_host_fm_vid
		) {
//		msleep(100);
//		dev_dbg(&client->dev, "[Focal][Touch] %s : uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x \n", __func__, uc_tp_fm_ver, uc_host_fm_ver);
		i_ret = fts_ctpm_fw_upgrade_with_i_file(client, fw_buf);
		if (i_ret == 0) {
			msleep(300);
			uc_host_fm_ver = fts_ctpm_get_i_file_ver(fw_buf);
			printk("[Focal][Touch] %s : upgrade to new version 0x%x \n", __func__, uc_host_fm_ver);
		} else {
			dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : upgrade failed ret=%d. \n", __func__, i_ret);
			return -EIO;
		}
	} else {
		printk("[Focal][Touch] %s : FW is new, no need update !\n", __func__);
		fw_update_complete = 1;
	}
	return 0;
}

/*
*get upgrade information depend on the ic type
*/
static void fts_get_upgrade_info(struct Upgrade_Info *upgrade_info)
{
	switch (DEVICE_IC_TYPE) {
	/*case IC_ftxxxx:
		upgrade_info->delay_55 = ftxxxx_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = ftxxxx_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = ftxxxx_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = ftxxxx_UPGRADE_ID_2;
		upgrade_info->delay_readid = ftxxxx_UPGRADE_READID_DELAY;
		break;
	case IC_FT5606:
		upgrade_info->delay_55 = FT5606_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5606_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5606_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5606_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5606_UPGRADE_READID_DELAY;
		break;
	case IC_FT5316:
		upgrade_info->delay_55 = FT5316_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5316_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5316_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5316_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5316_UPGRADE_READID_DELAY;
		break;
	case IC_FT5X36:
		upgrade_info->delay_55 = FT5X36_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X36_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X36_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X36_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X36_UPGRADE_READID_DELAY;
		break;*/
	case IC_FT5X46:
		upgrade_info->delay_55 = FT5X46_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X46_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X46_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X46_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X46_UPGRADE_READID_DELAY;
		break;
	default:
		break;
	}
}
#define FTS_UPGRADE_LOOP	5

int ftxxxxEnterUpgradeMode(struct i2c_client *client, int iAddMs, bool bIsSoft)
{
	/*bool bSoftResetOk = false;*/
	u32 i = 0;
	u8 auc_i2c_write_buf[10];
	int i_ret;
	u8 reg_val[4] = {0};

	if (bIsSoft) {/*reset by App*/
		fts_get_upgrade_info(&upgradeinfo);
		HidI2c_To_StdI2c(client);
		msleep(10);
		for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
			/*********Step 1:Reset  CTPM *****/
			/*write 0xaa to register 0xfc*/
			i_ret = ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_AA);
			msleep(upgradeinfo.delay_aa);
			/*write 0x55 to register 0xfc*/
			i_ret = ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_55);
			msleep(200);
			HidI2c_To_StdI2c(client);
			msleep(10);
			/*
			 * auc_i2c_write_buf[0] = 0xfc;
			 * auc_i2c_write_buf[1] = 0x66;
			 * i_ret = ftxxxx_write_reg(client, auc_i2c_write_buf[0], auc_i2c_write_buf[1]);
			 *
			 * if(i_ret < 0)
			 * FTS_DBG("[FTS] failed writing  0x66 to register 0xbc or oxfc! \n");
			 * msleep(50);
			*/
			/*********Step 2:Enter upgrade mode *****/
			auc_i2c_write_buf[0] = FT_UPGRADE_55;
			auc_i2c_write_buf[1] = FT_UPGRADE_AA;
			i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
			if (i_ret < 0) {
				FTS_DBG("[TOUCH_ERR] %s : failed writing  0xaa ! \n", __func__);
				continue;
			}
			/*********Step 3:check READ-ID***********************/
			msleep(1);
			auc_i2c_write_buf[0] = 0x90;
			auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;
			reg_val[0] = reg_val[1] = 0x00;
			ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
			if (reg_val[0] == upgradeinfo.upgrade_id_1 && reg_val[1] == upgradeinfo.upgrade_id_2) {
				FTS_DBG("[Touch] %s : Step 3: READ OK CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
				break;
			} else {
				dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
				continue;
			}
		}
	} else {/*reset by hardware reset pin*/
		fts_get_upgrade_info(&upgradeinfo);
		for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
			/*********Step 1:Reset  CTPM *****/
			ftxxxx_reset_tp(0);
			msleep(10);
			ftxxxx_reset_tp(1);
			msleep(8 + iAddMs);	/*time (5~20ms)*/
			/*HidI2c_To_StdI2c(client);msleep(10);*/
			/*********Step 2:Enter upgrade mode *****/
			auc_i2c_write_buf[0] = FT_UPGRADE_55;
			auc_i2c_write_buf[1] = FT_UPGRADE_AA;
			i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
			if (i_ret < 0) {
				FTS_DBG("[TOUCH_ERR] %s : failed writing  0xaa ! \n", __func__);
				continue;
			}
			/*********Step 3:check READ-ID***********************/
			msleep(1);
			auc_i2c_write_buf[0] = 0x90;
			auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;
			reg_val[0] = reg_val[1] = 0x00;
			ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
			if (reg_val[0] == upgradeinfo.upgrade_id_1 && reg_val[1] == upgradeinfo.upgrade_id_2) {
				FTS_DBG("[Touch] %s : Step 3: READ OK CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
				break;
			} else {
				dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
				continue;
			}
		}
	}
	return 0;
}

int fts_ctpm_fw_upgrade(struct i2c_client *client, u8 *pbt_buf, u32 dw_lenth)
{
	u8 reg_val[4] = {0};
	u32 i = 0;
	u32 packet_number;
	u32 j;
	u32 temp;
	u32 lenght;
	u8 packet_buf[FTS_PACKET_LENGTH + 6];
	u8 auc_i2c_write_buf[10];
	u8 bt_ecc;
	u8 bt_ecc_check;
	int i_ret;

	i_ret = HidI2c_To_StdI2c(client);
	if (i_ret == 0) {
		FTS_DBG("[TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
	}
	fts_get_upgrade_info(&upgradeinfo);
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset  CTPM *****/
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(upgradeinfo.delay_aa);
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_55);
		msleep(200);
		/*********Step 2:Enter upgrade mode *****/
		i_ret = HidI2c_To_StdI2c(client);
		if (i_ret == 0) {
			FTS_DBG("[TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
			continue;
		}
		msleep(5);
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);	/*asus: write 0xAA to 0x55 ???*/
		if (i_ret < 0) {
			FTS_DBG("[TOUCH_ERR] %s : failed writing  0x55 and 0xaa ! \n", __func__);
			continue;
		}
		/*********Step 3:check READ-ID***********************/
		msleep(1);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;
		reg_val[0] = reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
		if (reg_val[0] == upgradeinfo.upgrade_id_1 && reg_val[1] == upgradeinfo.upgrade_id_2) {	/*asus : relate on bootloader FW*/
			FTS_DBG("[Touch] %s : Step 3: READ OK CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev, "[Focal][TOUCH_ERR] %s :  Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			continue;
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;
	/*Step 4:erase app and panel paramenter area*/
	FTS_DBG("[TOUCH_ERR] %s : Step 4:erase app and panel paramenter area \n", __func__);
	auc_i2c_write_buf[0] = 0x61;/*0x60 for erase all flash (include bootloader)*/
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);		/*erase app area*/	/*asus: trigger erase command */
	msleep(1350);
	for (i = 0; i < 15; i++) {
		auc_i2c_write_buf[0] = 0x6a;
		reg_val[0] = reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 2);
		if (0xF0 == reg_val[0] && 0xAA == reg_val[1]) {
			break;
		}
		msleep(50);
	}
	FTS_DBG("[Touch] %s : erase app area reg_val[0] = %x reg_val[1] = %x \n", __func__, reg_val[0], reg_val[1]);
/*	asus: write 0x0A to 0x09 for update all.bin
	auc_i2c_write_buf[0] = 0x09;
	auc_i2c_write_buf[1] = 0x0a;
	i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
	if (i_ret < 0) {
		FTS_DBG("[TOUCH_ERR] %s : failed writing  0x0a and 0x09 ! \n", __func__);
	}
*/
	/*write bin file length to FW bootloader.*/
	auc_i2c_write_buf[0] = 0xB0;
	auc_i2c_write_buf[1] = (u8) ((dw_lenth >> 16) & 0xFF);
	auc_i2c_write_buf[2] = (u8) ((dw_lenth >> 8) & 0xFF);
	auc_i2c_write_buf[3] = (u8) (dw_lenth & 0xFF);
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 4);
	/*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;
	bt_ecc_check = 0;
	FTS_DBG("[Touch] %s : Step 5:write firmware(FW) to ctpm flash \n", __func__);
	/*dw_lenth = dw_lenth - 8;*/
	temp = 0;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	fw_update_total_count = packet_number;

	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;
	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		fw_update_progress = j;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (lenght >> 8);
		packet_buf[5] = (u8) lenght;
		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc_check ^= pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
//		printk("[Focal][Touch] %s :  bt_ecc = %x \n", __func__, bt_ecc);
		if (bt_ecc != bt_ecc_check)
			FTS_DBG("[TOUCH_ERR] %s :  Host checksum error bt_ecc_check = %x \n", __func__, bt_ecc_check);
		ftxxxx_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH + 6);
		/*msleep(10);*/
		for (i = 0; i < 30; i++) {
			auc_i2c_write_buf[0] = 0x6a;
			reg_val[0] = reg_val[1] = 0x00;
			ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 2);
			if ((j + 0x1000) == (((reg_val[0]) << 8) | reg_val[1])) {
				break;
			}
//			printk("[Focal][Touch] %s :  reg_val[0] = %x reg_val[1] = %x \n", __func__, reg_val[0], reg_val[1]);
			msleep(1);
		}
	}
	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		fw_update_progress = packet_number;
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;
		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] = pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc_check ^= pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		ftxxxx_i2c_Write(client, packet_buf, temp + 6);
		printk("[Focal][Touch] %s :  bt_ecc = %x \n", __func__, bt_ecc);
		if (bt_ecc != bt_ecc_check)
			printk("[FTS][%s] Host checksum error bt_ecc_check = %x \n", __func__, bt_ecc_check);
		for (i = 0; i < 30; i++) {
			auc_i2c_write_buf[0] = 0x6a;
			reg_val[0] = reg_val[1] = 0x00;
			ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 2);
			printk("[Focal][Touch] %s :  reg_val[0] = %x reg_val[1] = %x \n", __func__, reg_val[0], reg_val[1]);
			if ((j + 0x1000) == (((reg_val[0]) << 8) | reg_val[1])) {
				break;
			}
			printk("[Focal][Touch] %s :  reg_val[0] = %x reg_val[1] = %x \n", __func__, reg_val[0], reg_val[1]);
			msleep(1);
		}
	}
	msleep(50);
	/*********Step 6: read out checksum***********************/
	/*send the opration head */
	FTS_DBG("[Touch] %s : Step 6: read out checksum \n", __func__);
	auc_i2c_write_buf[0] = 0x64;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(300);
	temp = 0;
	auc_i2c_write_buf[0] = 0x65;
	auc_i2c_write_buf[1] = (u8)(temp >> 16);
	auc_i2c_write_buf[2] = (u8)(temp >> 8);
	auc_i2c_write_buf[3] = (u8)(temp);
	temp = dw_lenth;
	auc_i2c_write_buf[4] = (u8)(temp >> 8);
	auc_i2c_write_buf[5] = (u8)(temp);
	i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 6);
	msleep(dw_lenth/256);
	for (i = 0; i < 100; i++) {
		auc_i2c_write_buf[0] = 0x6a;
		reg_val[0] = reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 2);
		dev_err(&client->dev, "[Focal][Touch] %s :  --reg_val[0]=%02x reg_val[0]=%02x\n", __func__, reg_val[0], reg_val[1]);
		if (0xF0 == reg_val[0] && 0x55 == reg_val[1]) {
			dev_err(&client->dev, "[Focal][Touch] %s :  --reg_val[0]=%02x reg_val[0]=%02x\n", __func__, reg_val[0], reg_val[1]);
			break;
		}
		msleep(1);
	}
	auc_i2c_write_buf[0] = 0x66;
	ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : --ecc error! FW=%02x bt_ecc=%02x\n", __func__, reg_val[0], bt_ecc);
		return -EIO;
	}
	printk("[Focal][Touch] %s : checksum %X %X \n", __func__, reg_val[0], bt_ecc);
	/*********Step 7: reset the new FW***********************/
	FTS_DBG("[Touch] %s : Step 7: reset the new FW \n", __func__);
	auc_i2c_write_buf[0] = 0x07;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(200);	/*make sure CTP startup normally */
	i_ret = HidI2c_To_StdI2c(client);/*Android to Std i2c.*/
	if (i_ret == 0) {
		FTS_DBG("[TOUCH_ERR] %s :  HidI2c change to StdI2c fail ! \n", __func__);
	}
	fw_update_complete = 1;
	return 0;
}

/* sysfs debug*/

/*
*get firmware size

@firmware_name:firmware name

note:the firmware default path is sdcard.
if you want to change the dir, please modify by yourself.
*/
static int ftxxxx_GetFirmwareSize(char *firmware_name)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize = 0;
	char filepath[128];

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", firmware_name);
	if (NULL == pfile) {
		pfile = filp_open(filepath, O_RDONLY, 0);
	}
	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}
	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	filp_close(pfile, NULL);
	return fsize;
}

/*
*read firmware buf for .bin file.

@firmware_name: fireware name
@firmware_buf: data buf of fireware

note:the firmware default path is sdcard.
if you want to change the dir, please modify by yourself.
*/
static int ftxxxx_ReadFirmware(char *firmware_name, unsigned char *firmware_buf)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize;
	char filepath[128];
	loff_t pos;
	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", firmware_name);
	if (NULL == pfile) {
		pfile = filp_open(filepath, O_RDONLY, 0);
	}
	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}
	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(pfile, firmware_buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);
	return 0;
}

/*
upgrade with *.bin file
*/
int fts_ctpm_fw_upgrade_with_app_file(struct i2c_client *client, char *firmware_name)
{
	u8 *pbt_buf = NULL;
	int i_ret;
	int fwsize = ftxxxx_GetFirmwareSize(firmware_name);
	fw_update_complete = 0;
	printk("[FTS][%s] firmware name = %s \n", __func__, firmware_name);
	printk("[FTS][%s] firmware size = %d \n", __func__, fwsize);
	if (fwsize <= 0) {
		dev_err(&client->dev, "%s ERROR:Get firmware size failed\n", __FUNCTION__);
		return -EIO;
	}
	if (fwsize < 8 || fwsize > 54*1024) {
		dev_err(&client->dev, "FW length error\n");
		return -EIO;
	}
	/*=========FW upgrade========================*/
	pbt_buf = (unsigned char *) kmalloc(fwsize+1,GFP_ATOMIC);
	if (ftxxxx_ReadFirmware(firmware_name, pbt_buf)) {
		dev_err(&client->dev, "%s() - ERROR: request_firmware failed\n", __FUNCTION__);
		kfree(pbt_buf);
		return -EIO;
	}
	/*call the upgrade function*/
	i_ret =  fts_ctpm_fw_upgrade(client, pbt_buf, fwsize);
	if (i_ret != 0) {
		dev_err(&client->dev, "%s() - ERROR:[FTS] upgrade failed i_ret = %d.\n", __FUNCTION__, i_ret);
	} else {
#ifdef AUTO_CLB
		fts_ctpm_auto_clb(client);  /*start auto CLB*/
#endif
	}
	kfree(pbt_buf);
	return i_ret;
}

static ssize_t ftxxxx_tpfwver_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	u8 fwver = 0;
/*	struct i2c_client *client = container_of(dev, struct i2c_client, dev); ASUS jacob use globle ftxxxx_ts data*/

	wake_lock(&ftxxxx_ts->wake_lock);
	mutex_lock(&ftxxxx_ts->g_device_mutex);
	fwver = get_focal_tp_fw();
	if (fwver == 255)
		num_read_chars = snprintf(buf, PAGE_SIZE, "get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%x\n", fwver);
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	return num_read_chars;
}
static ssize_t ftxxxx_ftid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	u8 ftid = 0;
/*	struct i2c_client *client = container_of(dev, struct i2c_client, dev); ASUS jacob use globle ftxxxx_ts data*/

	wake_lock(&ftxxxx_ts->wake_lock);
	mutex_lock(&ftxxxx_ts->g_device_mutex);
	ftid=g_asus_lcdID;
	//ftid = get_focal_ft_id();
	if (ftid == 255)
		num_read_chars = snprintf(buf, PAGE_SIZE, "get ft id fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%d\n", ftid);
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	return num_read_chars ;
}
static ssize_t ftxxxx_touchic_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	u8 touchicid = 0;
/*	struct i2c_client *client = container_of(dev, struct i2c_client, dev); ASUS jacob use globle ftxxxx_ts data*/

	wake_lock(&ftxxxx_ts->wake_lock);
	mutex_lock(&ftxxxx_ts->g_device_mutex);
	touchicid = get_focal_touch_id();
	if (touchicid == 255)
		num_read_chars = snprintf(buf, PAGE_SIZE, "get touchic id fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%x\n", touchicid);
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	return num_read_chars ;
}
static ssize_t ftxxxx_ftreset_ic(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* place holder for future use */
	ftxxxx_reset_tp(0);
	msleep(500);
	ftxxxx_reset_tp(1);
	return count;
}

static ssize_t set_reset_pin_level_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		ftxxxx_reset_tp(0);

		ftxxxx_ts->reset_pin_status = 0;

	} else if (tmp == 1) {

		ftxxxx_reset_tp(1);

		ftxxxx_ts->reset_pin_status = 1;

	}

	return count;

}

static ssize_t set_reset_pin_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	bool stat = 0;

	if (gpio_get_value(ftxxxx_ts->pdata->rst_gpio))
		stat = true;
	else
		stat = false;

	return sprintf(buf, "reset_pin_status = %d and realy reset pin level = %d \n", ftxxxx_ts->reset_pin_status, stat);
}

static ssize_t switch_glove_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		focal_glove_switch(0);

	} else if (tmp == 1) {

		focal_glove_switch(1);

	}

	return count;

}

static ssize_t switch_glove_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", ftxxxx_ts->glove_mode_eable);
}

static ssize_t switch_keyboard_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		focal_keyboard_switch(0);

	} else if (tmp == 1) {

		focal_keyboard_switch(1);

	}

	return count;

}

static ssize_t switch_keyboard_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", ftxxxx_ts->keyboard_mode_eable);
}

static ssize_t switch_cover_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		focal_cover_switch(0);

	} else if (tmp == 1) {

		focal_cover_switch(1);

	}

	return count;

}
static ssize_t switch_cover_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", ftxxxx_ts->cover_mode_eable);
}
static ssize_t switch_swipeup_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		ftxxxx_ts->swipeup_mode_eable = 0;

		printk("[Focal][Touch] swipeup_mode_disable ! \n");

	} else if (tmp == 1) {

		ftxxxx_ts->swipeup_mode_eable = 1;

		printk("[Focal][Touch] swipeup_mode_enable ! \n");
	}

	return count;

}

static ssize_t switch_swipeup_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", ftxxxx_ts->swipeup_mode_eable);
}
static ssize_t switch_dclick_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		ftxxxx_ts->dclick_mode_eable = 0;

		printk("[Focal][Touch] dclick_mode_disable ! \n");

	} else if (tmp == 1) {

		ftxxxx_ts->dclick_mode_eable = 1;

		printk("[Focal][Touch] dclick_mode_enable ! \n");
	}

	return count;

}

static ssize_t switch_dclick_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", ftxxxx_ts->dclick_mode_eable);
}
static ssize_t switch_gesture_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;
	u8 gesturetmp = 0;
	char gesture_buf[16];
	char cmpchar = '1';

	memset(gesture_buf, 0, sizeof(gesture_buf));
	sprintf(gesture_buf, "%s", buf);
	gesture_buf[count-1] = '\0';

	printk("[Focal][Touch] gesture_mode_store %s ! \n", gesture_buf);

	if (gesture_buf[0] == cmpchar) {
		ftxxxx_ts->gesture_mode_eable = true;
		printk("[Focal][Touch] gesture_mode enable ! \n");
	} else
		ftxxxx_ts->gesture_mode_eable = false;

	if (ftxxxx_ts->gesture_mode_eable == 1) {

		for (tmp = 0; tmp < 7; tmp++) {
			if (gesture_buf[tmp] == cmpchar) {
				gesturetmp |= (1 << (6 - tmp));
			}
		}

		ftxxxx_ts->gesture_mode_type = gesturetmp;

		if ((ftxxxx_ts->gesture_mode_type & 1 << 6))
			FTS_gesture_register_d6 = 0x10;
		else
			FTS_gesture_register_d6 = 0x0;

		if ((ftxxxx_ts->gesture_mode_type & 1 << 5))
			FTS_gesture_register_d7 = 0x20;
		else
			FTS_gesture_register_d7 = 0x0;

		if ((ftxxxx_ts->gesture_mode_type & 1 << 4))
			FTS_gesture_register_d2 |= 0x10;
		else
			FTS_gesture_register_d2 &= 0xef;

		if ((ftxxxx_ts->gesture_mode_type & 1 << 3))
			FTS_gesture_register_d2 |= 0x08;
		else
			FTS_gesture_register_d2 &= 0xf7;

		if ((ftxxxx_ts->gesture_mode_type & 1 << 2))
			FTS_gesture_register_d5 |= 0x40;
		else
			FTS_gesture_register_d5 &= 0xbf;

		if ((ftxxxx_ts->gesture_mode_type & 1 << 1))
			FTS_gesture_register_d2 |= 0x02;
		else
			FTS_gesture_register_d2 &= 0xfd;

		printk("[Focal][Touch] gesture_mode_enable type = %x ! \n", ftxxxx_ts->gesture_mode_type);

	} else {

		ftxxxx_ts->gesture_mode_eable = 0;

		ftxxxx_ts->gesture_mode_type = 0;

		FTS_gesture_register_d2 = 0;

		FTS_gesture_register_d6 = 0;

		FTS_gesture_register_d5=0;
		FTS_gesture_register_d7 = 0;

		printk("[Focal][Touch] gesture_mode_disable ! \n");
		}

	return count;

}
static ssize_t switch_gesture_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	bool tmp = 0;

	tmp = (ftxxxx_ts->gesture_mode_type >> 6) & 1;

	if (!tmp)
		return sprintf(buf, "%x\n", tmp);
	else
		return sprintf(buf, "%x\n", ftxxxx_ts->gesture_mode_type);

}
static ssize_t switch_keypad_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;
	tmp = buf[0] - 48;

	if (tmp == 0) {
		focal_keypad_switch(0);
		printk("[Focal][Touch] keypad_mode_disable ! \n");
	} else if (tmp == 1) {
		focal_keypad_switch(1);
		printk("[Focal][Touch] keypad_mode_enable ! \n");
	}

	return count;
}

static ssize_t switch_keypad_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", ftxxxx_ts->keypad_mode_enable);
}
static ssize_t irq_disable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		FOCAL_IRQ_DISABLE = false;

	} else if (tmp == 1) {

		FOCAL_IRQ_DISABLE = true;

	}

	return count;

}

static ssize_t irq_disable_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", FOCAL_IRQ_DISABLE);
}

static ssize_t enable_proximity_check_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		EnableProximityCheck = false;
		printk("[Focal][Touch] DisableProximityCheck ! \n");
	} else if (tmp == 1) {

		EnableProximityCheck = true;
		printk("[Focal][Touch] EnableProximityCheck ! \n");
	}

	return count;

}

static ssize_t enable_proximity_check_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", EnableProximityCheck);
}
static ssize_t irq_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d \n", ftxxxx_ts->irq_lock_status);
}
static ssize_t ftxxxx_init_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d\n", focal_init_success);
}

static ssize_t asus_get_tpid(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%x\n", ftxxxx_read_tp_id());
	//return sprintf(buf, "%x\n", g_vendor_id);
}

static ssize_t asus_get_tpid_ic_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%x\n", F_VenderID);
}
static ssize_t asus_get_hwid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", HD_Modle);
}

static ssize_t asus_itr_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int tmp = 0;

	if (gpio_get_value(ftxxxx_ts->irq))
		tmp = 1;
	else
		tmp = 0;

	return sprintf(buf, "%d\n", tmp);
}

static ssize_t fw_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int err = 0;
	unsigned char uc_reg_addr;

	uc_reg_addr = FTXXXX_REG_VENDOR_ID;
	err = ftxxxx_i2c_Read(ftxxxx_ts->client, &uc_reg_addr, 1, &F_VenderID, 1);
	if (err < 0)
		F_VenderID = 0xFF;

	uc_reg_addr = FTXXXX_REG_PROJECT_ID;
	err = ftxxxx_i2c_Read(ftxxxx_ts->client, &uc_reg_addr, 1, &F_projectcode, 1);
	if (err < 0)
		F_projectcode = 0xFF;

	err = fts_ctpm_fw_upgrade_ReadProjectCode(ftxxxx_ts->client);

	err = fts_ctpm_fw_upgrade_ReadVendorID(ftxxxx_ts->client, &B_VenderID);
	if (err < 0)
		B_VenderID = 0xFF;

	printk("[Focal][Touch] FW vendor ID = %x , FW project code = %x , Bootloader PRJ code = %s , Bootloader Vendor ID = %x\n", F_VenderID, F_projectcode, B_projectcode, B_VenderID);

	return sprintf(buf, "%x-%x-%x-%s\n", F_VenderID, F_projectcode, B_VenderID, B_projectcode);
}

/* +++ASUS jacob add for get tp raw data +++*/
void GetRX_TXNum(void)
{
	unsigned char rx_regvalue = 0x00;
	unsigned char tx_regvalue = 0x00;
	msleep(200);

	if (ftxxxx_write_reg(G_Client, 0x00, 0x40) >= 0) {
		ftxxxx_read_reg(G_Client, 0x02, &tx_regvalue);
		g_TXNum = (short) tx_regvalue;
		msleep(100);
		ftxxxx_read_reg(G_Client, 0x03, &rx_regvalue);
		g_RXNum = (short) rx_regvalue;
	} else {
		g_TXNum = 25;
		g_RXNum = 25;
	}
	printk("[Focal][Touch] %s : channel num tx = %d rx = %d ! \n", __func__, g_TXNum, g_RXNum);
	return;
}

int asus_StartScan(void)
{
	int err = 0, i = 0;
	unsigned char regvalue = 0x00;

	err = ftxxxx_read_reg(G_Client, 0x00, &regvalue);
	printk("[Focal][Touch] %s : StartScan !!\n", __func__);
	if (err < 0) {
		printk("[Focal][TOUCH_ERR] %s : Enter StartScan Err , Regvalue = %d \n", __func__, regvalue);
		return err;
	} else {
		regvalue |= 0x80;
		err = ftxxxx_write_reg(G_Client, 0x00, regvalue);
		if (err < 0)
			return err;
		else {
			for (i = 0; i < 20; i++) {
				msleep(8);
				err = ftxxxx_read_reg(G_Client, 0x00, &regvalue);
				if (err < 0)
					return err;
				else {
					if (0 == (regvalue >> 7))
							break;
				}
			}
			if (i >= 20)
				return -5;
		}
	}
	return 0;
}

void asus_GetRawData(int RawData[TX_NUM_MAX][RX_NUM_MAX])
{
	/*unsigned char LineNum = 0;*/
	unsigned char I2C_wBuffer[3];
	unsigned char *rrawdata = NULL;
	/*unsigned char rrawdata[iTxNum*iRxNum*2];*/
	short len = 0, i = 0,j=0,z=0;
	short ByteNum = 0;
	int ReCode = 0;
	rrawdata = kmalloc(g_RXNum*g_TXNum*2, GFP_KERNEL);

	if (ftxxxx_write_reg(G_Client, 0x00, 0x40) >= 0) {
		if (asus_StartScan() >= 0) {
			I2C_wBuffer[0] = 0x01;
			I2C_wBuffer[1] = 0xaa;
			msleep(10);
			ReCode = FTS_I2c_Write(I2C_wBuffer, 2);
			I2C_wBuffer[0] = (unsigned char)(0x36);
			msleep(10);
			ReCode = FTS_I2c_Write(I2C_wBuffer, 1);
			printk("[Focal][Touch] %s : Record value = %d \n", __func__, ReCode);
			ByteNum = g_RXNum*g_TXNum * 2;
			if (ReCode >= 0) {
				//len = ByteNum;
				len=168;
				j=ByteNum/len;
				z=ByteNum%len;
				printk("[Focal][Touch] %s : Get Rawdata len = %d \n", __func__, len);
				memset(rrawdata, 0x00, sizeof(unsigned char)*ByteNum);
				msleep(10);
				for(i=0;i<j;i++){
					ReCode = FTS_I2c_Read(NULL, 0, (rrawdata+(len*i)), len);
					}
					ReCode = FTS_I2c_Read(NULL, 0, (rrawdata+(len*j)), z);
					printk("[touch] j=%d  z=%d ",j,z);
					if (ReCode >= 0) {
						for (i = 0; i < (ByteNum >> 1); i++) {
/*							for (x = 0; x < g_TXNum; x++) {
								for (y = 0; y < g_RXNum; y++) {
									RawData[x][y] = (short)((unsigned short)(rrawdata[(x*g_RXNum + y) * 2] << 8) + (unsigned short)rrawdata[((x*g_RXNum + y) * 2) + 1]);
									printk("Rawdata[%d][%d] =  %d  buf[%d][%d]\n", x, y, RawData[x][y], ((x*g_RXNum + y) * 2), ((x*g_RXNum + y) * 2 + 1));
								}
							}
*/
							RawData[i/g_RXNum][i%g_RXNum] = (short)((unsigned short)(rrawdata[i << 1]) << 8) \
								+ (unsigned short)rrawdata[(i << 1) + 1];
/*							printk("Get Rawdata data = %d \n", RawData[i/g_RXNum][i%g_TXNum]);*/
						}

					} else {
						printk("[Focal][TOUCH_ERR] %s : Get Rawdata failure \n", __func__);
					}
			}
		}
	}
	kfree(rrawdata);
	return;
}

static ssize_t asus_dump_tp_raw_data_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if ((0 <= (buf[0] - 48)) | ((buf[0] - 48) < 5)) {
		g_asus_tp_raw_data_flag = buf[0] - 48;
/*		count = sprintf(buf, " g_asus_tp_raw_data_flag = %d\n", g_asus_tp_raw_data_flag);*/
	} else {
		g_asus_tp_raw_data_flag = 0;
/*		count =  sprintf(buf, " Please echo 1 ~ 4 to dump_tp_raw_data !!\n");*/
	}

	return count;
}


static ssize_t asus_dump_tp_raw_data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	int err = 0;
	int i = 0, j = 0;
	if (g_asus_tp_raw_data_flag == 0)
		count = sprintf(buf, "Not thing out put, please echo 1 ~ 4 to dump_tp_raw_data first \n");
	else {
		if (ftxxxx_write_reg(G_Client, 0x00, 0x40) < 0)
			count = sprintf(buf, "Enter factory mode failure \n");
		else {
			memset(TPrawdata, 0, sizeof(TPrawdata));
			GetRX_TXNum();
			switch (g_asus_tp_raw_data_flag) {
			case 1:
				printk("[Focal][Touch] %s : dump Frequency Low FIR ON raw data \n", __func__);
				ftxxxx_write_reg(G_Client, 0x0a, 0x80);
				ftxxxx_write_reg(G_Client, 0xFB, 0x01);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency Low FIR ON raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency Low FIR ON raw data result ! \n");
				count += sprintf(buf + count, "Channel Start: %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;

			case 2:
				printk("[Focal][Touch] %s : dump Frequency Low FIR OFF raw data \n", __func__);
				ftxxxx_write_reg(G_Client, 0x0a, 0x80);
				ftxxxx_write_reg(G_Client, 0xFB, 0x00);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency Low FIR OFF raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency Low FIR OFF raw data result ! \n");
				count += sprintf(buf + count, "Channel Start: %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;

			case 3:
				printk("[Focal][Touch] %s : dump Frequency High FIR ON raw data \n", __func__);
				ftxxxx_write_reg(G_Client, 0x0a, 0x81);
				ftxxxx_write_reg(G_Client, 0xFB, 0x01);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency High FIR ON raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency High FIR ON raw data result ! \n");
				count += sprintf(buf + count, "Channel Start: %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;
			case 4:
				printk("[Focal][Touch] %s : dump Frequency High FIR OFF raw data \n", __func__);
				ftxxxx_write_reg(G_Client, 0x0a, 0x81);
				ftxxxx_write_reg(G_Client, 0xFB, 0x00);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency High FIR OFF raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency High FIR OFF raw data result ! \n");
				count += sprintf(buf + count, "Channel Start: %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;

			}

		}
		for (i = 0; i < 3; i++) {
			if (ftxxxx_write_reg(G_Client, 0x00, 0x00) >= 0)
				break;
			else
			msleep(200);
		}
	}
	return count;
}

static ssize_t asus_dump_tp_raw_data_to_file(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	int err = 0;
	int i = 0, j = 0;
	struct file *filp = NULL;
	mm_segment_t oldfs = { 0 };
	if (g_asus_tp_raw_data_flag == 0)
		count = sprintf(buf, "Not thing out put, please echo 1 ~ 4 to dump_tp_raw_data_files first \n");
	else {
		if (ftxxxx_write_reg(G_Client, 0x00, 0x40) < 0)
			count = sprintf(buf, "Enter factory mode failure \n");
		else {
			memset(TPrawdata, 0, sizeof(TPrawdata));
			GetRX_TXNum();
			switch (g_asus_tp_raw_data_flag) {
			case 1:
				printk("[Focal][Touch] %s : dump Frequency Low FIR ON raw data \n", __func__);

				filp = filp_open("/sdcard/Frequency-Low-FIR-ON-raw-data.csv", O_RDWR | O_CREAT, S_IRUSR);
				if (IS_ERR(filp)) {
					printk("[Focal][TOUCH_ERR] %s: open /sdcard/Frequency-Low-FIR-ON-raw-data.txt failed\n", __func__);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());

				ftxxxx_write_reg(G_Client, 0x0a, 0x80);
				ftxxxx_write_reg(G_Client, 0xFB, 0x01);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency Low FIR ON raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency Low FIR ON raw data result ! \n");
				count += sprintf(buf + count, "Channel Start:, %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d, ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;

			case 2:
				printk("[Focal][Touch] %s : dump Frequency Low FIR OFF raw data \n", __func__);

				filp = filp_open("/sdcard/Frequency-Low-FIR-OFF-raw-data.csv", O_RDWR | O_CREAT, S_IRUSR);
				if (IS_ERR(filp)) {
					printk("[Focal][TOUCH_ERR] %s: open /sdcard/Frequency-Low-FIR-OFF-raw-data.txt\n", __func__);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());

				ftxxxx_write_reg(G_Client, 0x0a, 0x80);
				ftxxxx_write_reg(G_Client, 0xFB, 0x00);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency Low FIR OFF raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency Low FIR OFF raw data result ! \n");
				count += sprintf(buf + count, "Channel Start:, %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;

			case 3:
				printk("[Focal][Touch] %s : dump Frequency High FIR ON raw data \n", __func__);

				filp = filp_open("/sdcard/Frequency-High-FIR-ON-raw-data.csv", O_RDWR | O_CREAT, S_IRUSR);
				if (IS_ERR(filp)) {
					printk("[Focal][TOUCH_ERR] %s: open /sdcard/Frequency-High-FIR-ON-raw-data.txt failed\n", __func__);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());

				ftxxxx_write_reg(G_Client, 0x0a, 0x81);
				ftxxxx_write_reg(G_Client, 0xFB, 0x01);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency High FIR ON raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency High FIR ON raw data result ! \n");
				count += sprintf(buf + count, "Channel Start:, %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;
			case 4:
				printk("[Focal][Touch] %s : dump Frequency High FIR OFF raw data \n", __func__);

				filp = filp_open("/sdcard/Frequency-High-FIR-OFF-raw-data.csv", O_RDWR | O_CREAT, S_IRUSR);
				if (IS_ERR(filp)) {
					printk("[Focal][TOUCH_ERR] %s: open /sdcard/Frequency-High-FIR-OFF-raw-data.txt failed\n", __func__);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());

				ftxxxx_write_reg(G_Client, 0x0a, 0x81);
				ftxxxx_write_reg(G_Client, 0xFB, 0x00);
				msleep(10);
				/*read raw data*/
				for (i = 0; i < 2 ; i++) {
					msleep(10);
					err = asus_StartScan();
				}
				printk("[Focal][Touch] %s : dump Frequency High FIR OFF raw data result = %d !\n", __func__, err);
				asus_GetRawData(TPrawdata);
				/*Print RawData Start*/
				count += sprintf(buf + count, "[ASUS] dump Frequency High FIR OFF raw data result ! \n");
				count += sprintf(buf + count, "Channel Start:, %4d, %4d\n\n", g_RXNum, g_TXNum);
				for (i = 0; i < g_TXNum; i++) {
					for (j = 0; j < g_RXNum; j++) {
						count += sprintf(buf + count, "%5d ", TPrawdata[i][j]);
					}
					count += sprintf(buf + count, " \n");
				}
				/*Print RawData End*/
				break;

			}

		filp->f_op->write(filp, buf, count, &filp->f_pos);
		set_fs(oldfs);
		filp_close(filp, NULL);

		}

		for (i = 0; i < 3; i++) {
			if (ftxxxx_write_reg(G_Client, 0x00, 0x00) >= 0)
				break;
			else
			msleep(200);
		}
	}
	return count;
}
/*--- ASUS jacob add for get tp raw data ---*/
/************************************************************************
* Name: fts_ctpm_fw_upgrade_with_app_file
* Brief:  upgrade with *.bin file
* Input: i2c info, file name
* Output: no
* Return: success =0
***********************************************************************/


int fts_ctpm_read_bl_ver(struct i2c_client *client, u8 *bl_ver)
{
	u8 reg_val[2] = {0};
	u32 i = 0;
	u8 auc_i2c_write_buf[4];
	int i_ret;
	fts_get_upgrade_info(&upgradeinfo);
	i_ret = HidI2c_To_StdI2c(client);
	if (i_ret == 0)
		printk("[Focal] hid change to i2c fail\n");

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset CTPM *****/
		printk("[Focal] Step 1: reset CTPM\n");
		/*write 0xaa to register 0xfc */
		i_ret = ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(FT5X46_UPGRADE_AA_DELAY);

		i_ret = ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_55);
		msleep(250);
		/*********Step 2:Enter upgrade mode *****/
		printk("[Focal] Step 2: Enter upgrade mode\n");
		i_ret = HidI2c_To_StdI2c(client);
		if (i_ret == 0) {
			printk("[Focal] hid change to i2c fail\n");
			continue;
		}
		msleep(10);
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
		if (i_ret < 0) {
			printk("[Focal] failed writing 0x55 and 0xaa\n");
			continue;
		}
		/*********Step 3:check READ-ID***********************/
		printk("[Focal] Step 3: Check read-ID\n");
		msleep(20);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = 0x00;
		auc_i2c_write_buf[2] = 0x00;
		auc_i2c_write_buf[3] = 0x00;
		reg_val[0] = 0x00;
		reg_val[1] = 0x00;

		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);

		if (reg_val[0] == upgradeinfo.upgrade_id_1&&
					reg_val[1] == upgradeinfo.upgrade_id_2) {
			/*read from bootloader FW*/
			printk("[Focal] Check OK, CTPM ID1=0x%x, ID2=0x%x\n",
				reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev,
			"[Focal]Step3:CTPM ID,ID1 = 0x%x, ID2 = 0x%x\n",
			reg_val[0], reg_val[1]);
			continue;
		}
	}

	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;

	/*********Step 4: read the BL version********************/
	/*printk("[Focal] Step 4: read the bl version\n");
	auc_i2c_write_buf[0] = FTS_REG_BL_VER;
	bl_ver[0] = 0x00;
	bl_ver[1] = 0x00;
	i_ret = ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, bl_ver, 2);
	if (i_ret < 0) {
		printk("[Focal] Read bootloader version failed\n");
		return -EIO;
	}*/
		msleep(10);
	auc_i2c_write_buf[0] = 0x03;
	auc_i2c_write_buf[1] = 0x00;
	auc_i2c_write_buf[2] = 0xd7;
	auc_i2c_write_buf[3] = 0x84;
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		ftxxxx_i2c_Write(client, auc_i2c_write_buf, 4);		/*send param addr*/
		msleep(5);
		reg_val[0] = reg_val[1] = 0x00;
		i_ret = ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 1);

		if (ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 1) < 0) {
			FTS_DBG("[TOUCH_ERR] %s : read vendor id from app param area error i_ret=%d \n", __func__, i_ret);
		} else {
			FTS_DBG("[Touch] %s : In upgrade Vendor ID, REG1 = 0x%x, REG2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			break;
		}
	}

	msleep(50);

	/*********Step 5: reset the new FW***********************/
	printk("[Focal] Step 5: reset the new FW\n");
	auc_i2c_write_buf[0] = 0x07;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(200);	/*make sure CTP startup normally */

	i_ret = HidI2c_To_StdI2c(client);
	if (i_ret == 0)
		printk("[Focal] HidI2c change to StdI2c fail ! \n");
	return 0;
}

int fts_ctpm_fw_preupgrade_hwreset(struct i2c_client * client,
					u8* pbt_buf, u32 dw_lenth)
{
	u8 reg_val[3] = {0};
	u32 i = 0, j = 0;
	u32 packet_number;
	u32 temp;
	u32 length;
	u8 packet_buf[FTS_PACKET_LENGTH + 6];
	u8 auc_i2c_write_buf[4];
	u8 bt_ecc;
	int i_ret;

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*****Step 1:HW Reset IC*****/
		printk("[Focal] Step 1: HW Reset IC\n");
		ftxxxx_reset_tp(0);
		msleep(10);
		ftxxxx_reset_tp(1);
		msleep(2 + i*3);

		/*****Step 2:Enter upgrade mode*****/
		printk("[Focal] Step 2: Enter upgrade mode\n");
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
		if (i_ret < 0) {
			printk("[Focal] failed writing 0x55 and 0xaa\n");
			continue;
		}

		/*****Step 3:check READ-ID*****/
		printk("[Focal] Step 3: Check read-ID\n");
		msleep(20);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = 0x00;
		auc_i2c_write_buf[2] = 0x00;
		auc_i2c_write_buf[3] = 0x00;
		reg_val[0] = 0x00;
		reg_val[1] = 0x00;

		/*read from bootloader FW*/
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);

		if (reg_val[0] == 0x58 && reg_val[1] == 0x22) {

			reg_val[2] = 0x00;
			ftxxxx_read_reg(client, 0xd0, &reg_val[2]);

			if (reg_val[2] == 0) {
				printk("[Focal]Step 3: READ State fail\n");
				continue;
			}

			printk("[Focal]Step3: read 0xd0=0x%02x\n", reg_val[2]);

			printk("[Focal] Check OK, CTPM ID1=0x%x, ID2=0x%x\n",
				reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev,
			"[Focal]Step3:CTPM ID,ID1 = 0x%x, ID2 = 0x%x\n",
			reg_val[0], reg_val[1]);
			continue;
		}
	}

	if (i >= FTS_UPGRADE_LOOP )
		return -EIO;

	/*****Step 4:write data to pram*****/
	printk("[Focal] Step 4: Write data to ctpm flash\n");
	bt_ecc = 0;
	temp = 0;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xae;
	packet_buf[1] = 0x00;
	dev_dbg(&client->dev, "packet_number: %d\n", packet_number);

	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		length = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (length >> 8);
		packet_buf[5] = (u8) length;

		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		dev_dbg(&client->dev, "bt_ecc = %x\n", bt_ecc);
		ftxxxx_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH + 6);
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;

		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] =
				pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		dev_dbg(&client->dev, "bt_ecc = %x\n", bt_ecc);
		ftxxxx_i2c_Write(client, packet_buf, temp + 6);
	}

	/*****Step 5: read out checksum*****/
	printk("[Focal] Step 5: Read out checksum\n");
	auc_i2c_write_buf[0] = 0xcc;
	ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		dev_err(&client->dev,
			"[Focal]--ecc error! FW=%02x bt_ecc=%02x\n",
			reg_val[0], bt_ecc);
		return -EIO;
	}
	printk("[Focal] checksum %X %X\n", reg_val[0], bt_ecc);
	msleep(50);

	/*****Step 6: start app*****/
	printk("[Focal] Step 6: Start app\n");
	auc_i2c_write_buf[0] = 0x08;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(20);
	return 0;
}

int fts_ctpm_fw_download(struct i2c_client *client, u8 *pbt_buf, u32 dw_lenth)
{
	u8 reg_val[2] = {0};
	u32 i = 0;
	u32 packet_number;
	u32 j=0;
	u32 temp;
	u32 length;
	u8 packet_buf[FTS_PACKET_LENGTH + 6];
	u8 auc_i2c_write_buf[6];
	u8 bt_ecc;
	int i_ret;

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*****Step 1:Reset  CTPM*****/
		printk("[Focal] Step 1: reset CTPM\n");
		/*write 0xaa to register 0xfc*/
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(FT5X46_UPGRADE_AA_DELAY);
		/*write 0x55 to register 0xfc*/
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_55);
		msleep(250);
		/*****Step 2:Enter upgrade mode*****/
		printk("[Focal] Step 2: Enter upgrade mode\n");
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
		if (i_ret < 0) {
			printk("[Focal] failed writing 0x55 and 0xaa\n");
			continue;
		}
		/*****Step 3:check READ-ID*****/
		printk("[Focal] Step 3: Check read-ID\n");
		msleep(20);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = 0x00;
		auc_i2c_write_buf[2] = 0x00;
		auc_i2c_write_buf[3] = 0x00;
		reg_val[0] = 0x00;
		reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);

		if (reg_val[0] == 0x58 && reg_val[1] == 0x2b) {
			/*read from bootloader FW*/
			printk("[Focal] Check OK, CTPM ID1=0x%x, ID2=0x%x\n",
				reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev,
			"[Focal]Step3:CTPM ID,ID1 = 0x%x, ID2 = 0x%x\n",
			reg_val[0], reg_val[1]);
			continue;
		}
	}

	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;

	/*****Step 4:erase app and panel paramenter area*****/
	printk("[Focal] Step 4-1:change to write flash mode\n");
	ftxxxx_write_reg(client, 0x09, 0x0a);

	printk("[Focal] Step 4-2:erase app and panel paramenter area\n");
	auc_i2c_write_buf[0] = 0x61;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);	/*erase app area*/

	msleep(2000);
	for(i = 0; i < 15; i++) {
		auc_i2c_write_buf[0] = 0x6a;
		reg_val[0] = 0x00;
		reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 2);
		if (0xF0 == reg_val[0] && 0xAA == reg_val[1])
			break;
		msleep(50);
	}

	/*****Step 5:write firmware(FW) to ctpm flash*****/
	printk("[Focal] Step 5:write firmware(FW) to ctpm flash\n");
	bt_ecc = 0;
	temp = 0;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;
	dev_dbg(&client->dev, "packet_number: %d\n", packet_number);

	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		length = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (length >> 8);
		packet_buf[5] = (u8) length;

		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		dev_dbg(&client->dev, "bt_ecc = %x\n", bt_ecc);
		ftxxxx_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH + 6);

		for (i = 0; i < 30; i++) {
			auc_i2c_write_buf[0] = 0x6a;
			reg_val[0] = 0x00;
			reg_val[1] = 0x00;
			ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1,
							reg_val, 2);

			if ((j + 0x1000) == (((reg_val[0]) << 8) | reg_val[1]))
				break;

			msleep(20);
		}
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;
		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] =
				pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		ftxxxx_i2c_Write(client, packet_buf, temp + 6);

		for(i = 0; i < 30; i++) {
			auc_i2c_write_buf[0] = 0x6a;
			reg_val[0] = reg_val[1] = 0x00;
			ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1,
							reg_val, 2);

			if ((j + 0x1000) == (((reg_val[0]) << 8) | reg_val[1]))
				break;
			msleep(20);
		}
	}
	msleep(50);

	/*****Step 6: read out checksum*****/
	printk("[Focal] Step 6: read out checksum\n");
	auc_i2c_write_buf[0] = 0x64;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(300);

	temp = 0;
	auc_i2c_write_buf[0] = 0x65;
	auc_i2c_write_buf[1] = (u8)(temp >> 16);
	auc_i2c_write_buf[2] = (u8)(temp >> 8);
	auc_i2c_write_buf[3] = (u8)(temp);
	temp = dw_lenth;
	auc_i2c_write_buf[4] = (u8)(temp >> 8);
	auc_i2c_write_buf[5] = (u8)(temp);
	i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 6);
	msleep(dw_lenth/256);

	for (i = 0; i < 100; i++) {
		auc_i2c_write_buf[0] = 0x6a;
		reg_val[0] = 0x00;
		reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 2);
		if ((0xF0 == reg_val[0]) && (0x55 == reg_val[1])) {
			printk("[Focal]--reg_val[0]=%02x reg_val[0]=%02x\n",
						reg_val[0], reg_val[1]);
			break;
		}
		msleep(10);
	}

	auc_i2c_write_buf[0] = 0x66;
	ftxxxx_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		dev_err(&client->dev,
			"[Focal]--ecc error! FW=%02x bt_ecc=%02x\n",
			reg_val[0], bt_ecc);
		return -EIO;
	}

	printk("[Focal] checksum %X %X \n", reg_val[0], bt_ecc);
	/*****Step 7: reset the new FW*****/
	printk("[Focal] Step 7: reset the new FW\n");
	auc_i2c_write_buf[0] = 0x07;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(200);	/*make sure CTP startup normally */

	i_ret = HidI2c_To_StdI2c(client);
	if (i_ret == 0)
		printk("[Focal] HidI2c change to StdI2c fail ! \n");
	return 0;
}

int fts_ctpm_fw_upgrade_with_all_file(struct i2c_client *client,
				       char *firmware_name)
{
	u8 *pbt_buf = NULL;
	int i_ret = 0;
	int fwsize = ftxxxx_GetFirmwareSize(firmware_name);

	if (fwsize <= 0) {
		dev_err(&client->dev,
			"%s: Get firmware size failed\n", __func__);
		return -EIO;
	}

	if (fwsize < 8 || fwsize > (64 * 1024)) {
		dev_err(&client->dev, "%s: FW length error\n", __func__);
		return -EIO;
	}

	/*=====FW upgrade=====*/
	pbt_buf = kmalloc(fwsize + 1, GFP_ATOMIC);

	if (ftxxxx_ReadFirmware(firmware_name, pbt_buf)) {
		dev_err(&client->dev,
			"%s: Request_firmware failed\n", __func__);
		kfree(pbt_buf);
		return -EIO;
	}

	/*load pramboot code*/
	i_ret = fts_ctpm_fw_preupgrade_hwreset(client, aucFW_PRAM_BOOT,
					sizeof(aucFW_PRAM_BOOT));
	if (i_ret != 0) {
		dev_err(&client->dev, "%s: Download pram failed\n", __func__);
		kfree(pbt_buf);
		return i_ret;
	}

	/*call the upgrade function */
	i_ret = fts_ctpm_fw_download(client, pbt_buf, fwsize);
	if (i_ret != 0) {
		dev_err(&client->dev, "%s: download all.bin failed\n",
								__func__);
	}

	kfree(pbt_buf);
	return i_ret;
}




static ssize_t ftxxxx_tpfwver_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* place holder for future use */
	return -EPERM;
}

static ssize_t ftxxxx_tprwreg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	return -EPERM;
}

static ssize_t ftxxxx_tprwreg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	int retval;
	//u32 wmreg=0;
	long unsigned int wmreg=0;
	u8 regaddr=0xff,regvalue=0xff;
	u8 valbuf[5]={0};

	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	memset(valbuf, 0, sizeof(valbuf));

	num_read_chars = count - 1;
	if (num_read_chars != 2) {
		if (num_read_chars != 4) {
			dev_err(dev, "[Focal][Touch] %s : please input 2 or 4 character \n", __func__);
			goto error_return;
		}
	}
	memcpy(valbuf, buf, num_read_chars);
	retval = strict_strtoul(valbuf, 16, &wmreg);
	if (0 != retval) {
		dev_err(dev, "[Focal][Touch] %s() - ERROR: Could not convert the given input to a number. The given input was: \"%s\"\n", __FUNCTION__, buf);
		goto error_return;
	}
	if (2 == num_read_chars) {
		//read register
		regaddr = wmreg;
		if (ftxxxx_read_reg(client, regaddr, &regvalue) < 0)
			printk("[Focal][TOUCH_ERR] %s : Could not read the register(0x%02x) \n", __func__, regaddr);
		else
			printk("[Focal][Touch] %s : the register(0x%02x) is 0x%02x \n", __func__, regaddr, regvalue);
	} else {
		regaddr = wmreg>>8;
		regvalue = wmreg;
		if (ftxxxx_write_reg(client, regaddr, regvalue)<0)
			dev_err(dev, "[Focal][TOUCH_ERR] %s : Could not write the register(0x%02x) \n", __func__, regaddr);
		else
			dev_dbg(dev, "[Focal][Touch] %s : Write 0x%02x into register(0x%02x) successful \n", __func__, regvalue, regaddr);
	}
	error_return:
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	return count;
	//return 1;
}
static ssize_t forcetouch_erase_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	u8 regvalue=0xff;
	int ret=00;
	//u8	regaddr=0x07;
	/*enter force calibration mode */

	ftxxxx_nosync_irq_disable(ftxxxx_ts->client);
	regvalue=0x60;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0){
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
		ret++;
		}
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
	/*erease counter */
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_ERASE_REG, regvalue) < 0){
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x07)  \n", __func__ );
		ret++;
		}
	else
		printk("[Focal][Touch] %s : the register(0x07) is 0x%02x ,erase force sensor flash value \n", __func__, regvalue);
	/* enter work mode*/
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0){
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
		ret++;
		}
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
	ftxxxx_irq_enable(ftxxxx_ts->client);
	if(ret==0)
		return sprintf(buf, "erase successful !\n");
	else
		return sprintf(buf, "erase fail  !\n");

}
static ssize_t forcetouch_cal_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	u8 regvalue=0xff;
	int reg=0,value=0;
	//u8	regaddr=0x07;

	//enter force calibration mode 

	ftxxxx_nosync_irq_disable(ftxxxx_ts->client);
	regvalue=0x60;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
	//erease counter 
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_ERASE_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x07)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x07) is 0x%02x ,erase force sensor flash value \n", __func__, regvalue);
	//write data to 0x14 for calibration 
	forcetouchreg[0][0]=0x14;
	//forcetouchreg[1][0]=0x14;
	if (ftxxxx_i2c_Write(client,forcetouchreg[0], 37) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x14)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : write value to 0x14 with 0.5kg value susccfully! \n", __func__);
	/*if (ftxxxx_i2c_Write(client, forcetouchreg[1], 37) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x14)  \n", __func__ );
	else
		printk("[Focal][Touch] %s :write value to 0x14 with 1kg value susccfully  \n", __func__);*/
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);

	regvalue=0x60;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_ERASE_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x07)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x07) is 0x%02x ,erase force sensor flash value \n", __func__, regvalue);
	forcetouchreg[2][0]=0x12;
	if (ftxxxx_i2c_Read(client,forcetouchreg[2], 1 ,forcetouchreg[2]+1 , 36) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not read the register(0x14)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : write value to 0x14 with 0.5kg value susccfully! \n", __func__);
	/*if (ftxxxx_i2c_Write(client, forcetouchreg+36, 37) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x14)  \n", __func__ );
	else
		printk("[Focal][Touch] %s :write value to 0x14 with 1kg value susccfully  \n", __func__);*/
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);


	/*1kg calibration*/
	regvalue=0x60;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
	//erease counter 
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_ERASE_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x07)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x07) is 0x%02x ,erase force sensor flash value \n", __func__, regvalue);
	//write data to 0x14 for calibration 
	forcetouchreg[1][0]=0x1e;
	if (ftxxxx_i2c_Write(client,forcetouchreg[1], 37) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x1e)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : write value to 0x1e with 1kg value susccfully! \n", __func__);
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);

	regvalue=0x60;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_ERASE_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x07)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x07) is 0x%02x ,erase force sensor flash value \n", __func__, regvalue);
	forcetouchreg[3][0]=0x1c;
	if (ftxxxx_i2c_Read(client,forcetouchreg[3], 1 ,forcetouchreg[3]+1 , 36) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not read the register(0x1c)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : read value to 0x1c with 1kg value susccfully! \n", __func__);
	/*if (ftxxxx_i2c_Write(client, forcetouchreg+36, 37) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x14)  \n", __func__ );
	else
		printk("[Focal][Touch] %s :write value to 0x14 with 1kg value susccfully  \n", __func__);*/
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);

	for(value=1;value < 37;value++){
		printk("[Focal][Touch] write 0.5 kg [%d]= %d ,read[%d] =%d \n ",value,forcetouchreg[0][value],value,forcetouchreg[2][value]);
		if(forcetouchreg[0][value] != forcetouchreg[2][value])
			reg=reg+1;
		}
	for(value=1;value < 37;value++){
		if(forcetouchreg[1][value] != forcetouchreg[3][value])
			reg=reg+1;
		printk("[Focal][Touch] write 1 kg [%d]= %d ,read[%d] =%d \n ",value,forcetouchreg[1][value],value,forcetouchreg[3][value]);
		}

	ftxxxx_irq_enable(ftxxxx_ts->client);
	if(reg ==0){
		printk("pass!\n");
		return sprintf(buf,"Pass !! \n") ;
		
	}
	else{
		printk("fail!\n");
		return sprintf(buf, "Fail !!\n");}

}

static ssize_t forcetouch_cal_data_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;

	tmp = buf[0] - 48;

	if (tmp == 0) {

		ft_data_flag =0;

	} else if (tmp == 1) {

		ft_data_flag = 1;
	}

	return count;
	
}

static ssize_t forcetouch_cal_data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	u8 regvalue=0xff;
	//u8	regaddr=0x07;
	//enter force calibration mode 
	int count = 0;
	int i = 0;
	struct file *filp = NULL;
	mm_segment_t oldfs = { 0 };

	regvalue=0x60;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_ERASE_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x07)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x07) is 0x%02x ,erase force sensor flash value \n", __func__, regvalue);
	if(ft_data_flag == 0){
		forcetouchreg[2][0]=0x12;
		if (ftxxxx_i2c_Read(client,forcetouchreg[2], 1 ,forcetouchreg[2]+1 , 36) < 0)
			printk("[Focal][TOUCH_ERR] %s : Could not read the register(0x14)  \n", __func__ );
		else
			printk("[Focal][Touch] %s : write value to 0x14 with 0.5kg value susccfully! \n", __func__);

		printk("[Focal][Touch] %s : dump 500g calibration data \n", __func__);
		filp = filp_open("/sdcard/500g-calibration-data.csv", O_RDWR | O_CREAT, S_IRUSR);
		if (IS_ERR(filp)) {
			printk("[Focal][TOUCH_ERR] %s: open /sdcard/500g-calibration-data.csv.txt failed\n", __func__);
			return 0;
		}
			oldfs = get_fs();
			set_fs(get_ds());
			/*Print RawData Start*/;
			count += sprintf(buf + count, "[ASUS] dump 500g calibration data result ! \n");
			for (i = 1; i < 37; i++) {
				count += sprintf(buf + count, "%d ", forcetouchreg[2][i]);
			}
			/*Print RawData End*/
	}else if (ft_data_flag == 1){
		forcetouchreg[3][0]=0x1c;
		if (ftxxxx_i2c_Read(client,forcetouchreg[3], 1 ,forcetouchreg[3]+1 , 36) < 0)
			printk("[Focal][TOUCH_ERR] %s : Could not read the register(0x1c)  \n", __func__ );
		else
			printk("[Focal][Touch] %s : read value to 0x1c with 1kg value susccfully! \n", __func__);
		printk("[Focal][Touch] %s : dump 1000g calibration data \n", __func__);
		filp = filp_open("/sdcard/1000g-calibration-data.csv", O_RDWR | O_CREAT, S_IRUSR);
		if (IS_ERR(filp)) {
			printk("[Focal][TOUCH_ERR] %s: open /sdcard/1000g-calibration-data.csv.txt failed\n", __func__);
			return 0;
		}
			oldfs = get_fs();
			set_fs(get_ds());
			/*Print RawData Start*/;
			count += sprintf(buf + count, "[ASUS] dump 1000g calibration data result ! \n");
			for (i = 1; i < 37; i++) {
				count += sprintf(buf + count, "%d ", forcetouchreg[3][i]);
			}
			/*Print RawData End*/
	}	
	filp->f_op->write(filp, buf, count, &filp->f_pos);
	set_fs(oldfs);
	filp_close(filp, NULL);
	
	regvalue=0x00;
	if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
	else
		printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);

	return count;
}




static ssize_t forcetouch_read_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	u8 regaddr=0x07;
	u8 regvalue=0x00;
	if (ftxxxx_i2c_Read(client,&regaddr, 1 ,&regvalue,1) < 0)
		printk("[Focal][TOUCH_ERR] %s : Could not read the register(0x07)  \n", __func__ );
	else	
		printk("[Focal][Touch] %s : read pressure successful ! \n", __func__);
	printk("[Touch] pressure =%d",regvalue);	
	if (FT_READ==true)
		return sprintf(buf, "%d \n",regvalue) ;
	else
		return sprintf(buf, "Touch_location_input_fail! \n") ;
}
static ssize_t forcetouch_read_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	int retval;
	int abc=0;
	//u32 wmreg=0;
	long unsigned int wmreg=0;
	u8 regvalue=0x00,regaddr=0x00;
	u8 svalue[4]={0};
	int weight=0,location=0;
	u8 readforce[72]={0};
	u8 valbuf[3]={0};
//	u8 ft_pressure;
	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	memset(valbuf, 0, sizeof(valbuf));

	num_read_chars = count - 1;
	if (num_read_chars != 2) {
		dev_err(dev, "[Focal][Touch] %s : please input 2  character \n", __func__);
		goto error_return;
	}
	memcpy(valbuf, buf, num_read_chars);
	retval = strict_strtoul(valbuf, 10, &wmreg);
	printk("[touch] value= %lu  \n",wmreg);
	if (0 != retval) {
		dev_err(dev, "[Focal][Touch] %s() - ERROR: Could not convert the given input to a number. The given input was: \"%s\"\n", __FUNCTION__, buf);
		goto error_return;
	}
	if (2 == num_read_chars) {
		//read register
		regvalue=0x60;
		if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
			printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
		else
			printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
		regvalue=0x00;
		if (ftxxxx_write_reg(client, FORCE_ERASE_REG, regvalue) < 0)
			printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x07)  \n", __func__ );
		else
			printk("[Focal][Touch] %s : the register(0x07) is 0x%02x ,erase force sensor flash value \n", __func__, regvalue);
		regaddr=0x16;
		if (ftxxxx_i2c_Read(client,&regaddr, 1 , readforce,72) < 0)
			printk("[Focal][TOUCH_ERR] %s : Could not read the register(0x16)  \n", __func__ );
		else
			printk("[Focal][Touch] %s : read force sensor value successful ! \n", __func__);
		for(abc=0;abc<72;abc++){
		printk("[Focal][Touch] readforce value[%d] = %x \n",abc,readforce[abc]);
		}
		regvalue=0x00;
		if (ftxxxx_write_reg(client, FORCE_WORK_REG, regvalue) < 0)
			printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x00)  \n", __func__ );
		else
			printk("[Focal][Touch] %s : the register(0x00) is 0x%02x ,touch in read force sensor mode \n", __func__, regvalue);
		weight = wmreg/10;
		location = wmreg%10;
		if(weight < 2 ){
			FT_READ =true;
			if(location <= 3){
				svalue[0]=readforce[location*4-2];
				svalue[1]=readforce[location*4-1];
				svalue[2]=readforce[(location+3)*4-2];
				svalue[3]=readforce[(location+3)*4-1];
				printk("[Focal][Touch] readforce [%d] =%d \n ",location*4-2,readforce[location*4-2]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",location*4-1,readforce[location*4-1]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+3)*4-2,readforce[(location+3)*4-2]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+3)*4-1,readforce[(location+3)*4-1]);
				forcetouchreg[weight][location*2-1]= svalue[0];
				forcetouchreg[weight][location*2]= svalue[1];
				forcetouchreg[weight][(location+3)*2-1]=svalue[2];
				forcetouchreg[weight][(location+3)*2]=svalue[3];
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, location*2-1, forcetouchreg[weight][location*2-1]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, location*2, forcetouchreg[weight][location*2]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+3)*2-1, forcetouchreg[weight][(location+3)*2-1]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+3)*2, forcetouchreg[weight][(location+3)*2]);
				FT_value[0]=forcetouchreg[weight][location*2-1];
				FT_value[1]=forcetouchreg[weight][location*2];
				FT_value[2]=forcetouchreg[weight][(location+3)*2-1];
				FT_value[3]=forcetouchreg[weight][(location+3)*2];
			}else if(location <= 6){
				svalue[0]=readforce[(location+3)*4-2];
				svalue[1]=readforce[(location+3)*4-1];
				svalue[2]=readforce[(location+6)*4-2];
				svalue[3]=readforce[(location+6)*4-1];
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+3)*4-2,readforce[(location+3)*4-2]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+3)*4-1,readforce[(location+3)*4-1]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+6)*4-2,readforce[(location+6)*4-2]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+6)*4-1,readforce[(location+6)*4-1]);
				forcetouchreg[weight][(location+3)*2-1]= svalue[0];
				forcetouchreg[weight][(location+3)*2]= svalue[1];
				forcetouchreg[weight][(location+6)*2-1]=svalue[2];
				forcetouchreg[weight][(location+6)*2]=svalue[3];
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+3)*2-1, forcetouchreg[weight][(location+3)*2-1]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+3)*2, forcetouchreg[weight][(location+3)*2]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+6)*2-1, forcetouchreg[weight][(location+6)*2-1]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+6)*2, forcetouchreg[weight][(location+6)*2]);
				FT_value[0]=forcetouchreg[weight][(location+3)*2-1];
				FT_value[1]=forcetouchreg[weight][(location+3)*2];
				FT_value[2]=forcetouchreg[weight][(location+3)*2-1];
				FT_value[3]=forcetouchreg[weight][(location+3)*2];

			}else if(location <= 9){
				svalue[0]=readforce[(location+6)*4-2];
				svalue[1]=readforce[(location+6)*4-1];
				svalue[2]=readforce[(location+9)*4-2];
				svalue[3]=readforce[(location+9)*4-1];
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+6)*4-2,readforce[(location+6)*4-2]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+6)*4-1,readforce[(location+6)*4-1]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+9)*4-2,readforce[(location+9)*4-2]);
				printk("[Focal][Touch] readforce [%d] =%d \n ",(location+9)*4-1,readforce[(location+9)*4-1]);
				forcetouchreg[weight][(location+6)*2-1]= svalue[0];
				forcetouchreg[weight][(location+6)*2]= svalue[1];
				forcetouchreg[weight][(location+9)*2-1]=svalue[2];
				forcetouchreg[weight][(location+9)*2]=svalue[3];
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+6)*2-1, forcetouchreg[weight][(location+6)*2-1]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+6)*2, forcetouchreg[weight][(location+6)*2]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+9)*2-1, forcetouchreg[weight][(location+9)*2-1]);
				printk("[Focal][Touch]%s : forcetouchreg[%d][%d]= %d \n", __func__, weight, (location+9)*2, forcetouchreg[weight][(location+9)*2]);
				FT_value[0]=forcetouchreg[weight][(location+6)*2-1];
				FT_value[1]=forcetouchreg[weight][(location+6)*2];
				FT_value[2]=forcetouchreg[weight][(location+9)*2-1];
				FT_value[3]=forcetouchreg[weight][(location+9)*2];
				}
			else{
				FT_READ =false;
				printk("[Focal][TOUCH_ERR] %s : force touch location input error! (please input 1-9) \n", __func__);
			}
			}
		else{
			FT_READ =false;
			printk("[Focal][TOUCH_ERR] %s : force touch location input error!!  (please input 0 or 1 ) \n", __func__);
			}
	}
	error_return:
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	return count;

}


static ssize_t DoubleTapConfig_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	u8 chck_buf[4] = { 0 };
	chck_buf[0] = 0xe2;

	ftxxxx_i2c_Read(ftxxxx_ts->client, chck_buf, 1, chck_buf, 4);

	count += sprintf(buf, "Touch Slop = %x, Touch Distance = %x, Time Gap = %x, Int Delay Time = %x \n", chck_buf[0], chck_buf[1], chck_buf[2], chck_buf[3]);
	
	return count;
	//return 0;
}
static ssize_t DoubleTapConfig_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
//	ssize_t num_read_chars = 0;
	int retval, i;
	//u32 wmreg=0;
	long unsigned int wmreg=0;
	int LoopBaseNum = 8;
	u8 regaddr=0xff,regvalue=0xff;
	u8 cvtbuf[5]={0};

	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	printk("[Focal][Touch] %s : Input char num = %ld ! \n", __func__, count);

	if (count/LoopBaseNum) {
		for (i = 0; i < (count/LoopBaseNum); i++) {
			if ((buf[LoopBaseNum*i] == 'w') && (buf[LoopBaseNum*i + 1] == ':') && (buf[LoopBaseNum*i + 2] == 'x')) {
				memcpy(cvtbuf, buf + (LoopBaseNum*i + 3), 4);
				printk("[Focal][Touch] %s : Set char %s ! \n", __func__, cvtbuf);
				retval = strict_strtoul(cvtbuf, 16, &wmreg);
				if (0 != retval) {
					printk("[Focal][Touch] %s() - ERROR: Could not convert the given input to a number. The given input was: \"%s\"\n", __FUNCTION__, buf);
					goto error_return;
				}

				regaddr = wmreg>>8;
				regvalue = wmreg;
				switch(regaddr) {
				case 0xe2 : 
					g_touch_slop = regvalue;
					ReConfigDoubleTap = true;
					break;
				case 0xe3 : 
					g_touch_distance = regvalue;
					ReConfigDoubleTap = true;
					break;
				case 0xe4 : 
					g_time_gap = regvalue;
					ReConfigDoubleTap = true;
					break;
				case 0xe5 : 
					g_int_time = regvalue;
					ReConfigDoubleTap = true;
					break;
				}

				if (ftxxxx_write_reg(client, regaddr, regvalue)<0)
					printk("[Focal][TOUCH_ERR] %s : Could not write the register(0x%02x) \n", __func__, regaddr);
				else
					printk("[Focal][Touch] %s : Write 0x%02x into register(0x%02x) successful \n", __func__, regvalue, regaddr);

			}
		}
	}

	error_return:
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);

	return count;
}

int iHigh = 1;
static ssize_t ftxxxx_fwupdate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (iHigh) {
		iHigh=0;
		ftxxxx_reset_tp(1);
	} else {
		iHigh=1;
		ftxxxx_reset_tp(0);
	}
	/* place holder for future use */
	return -EPERM;
}

/*upgrade from *.i*/
static ssize_t ftxxxx_fwupdate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct ftxxxx_ts_data *data = NULL;
/*	u8 uc_host_fm_ver;*/
/*	int i_ret;*/
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	data = (struct ftxxxx_ts_data *) i2c_get_clientdata(client);

	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	ftxxxx_nosync_irq_disable(ftxxxx_ts->client);

/*	i_ret = fts_ctpm_fw_upgrade_with_i_file(client);
	if (i_ret == 0)
	{
		msleep(300);
		uc_host_fm_ver = fts_ctpm_get_i_file_ver();
		dev_dbg(dev, "%s [FTS] upgrade to new version 0x%x\n", __FUNCTION__, uc_host_fm_ver);
	}
	else
	{
		dev_err(dev, "%s ERROR:[FTS] upgrade failed ret=%d.\n", __FUNCTION__, i_ret);
	}
*/
	/*ftxxxxEnterUpgradeMode(client, 0 ,false);*/	/*asus : mark by focal Elmer*/
	fw_size = sizeof(TP_5446_FW);
	fts_ctpm_auto_upgrade(client, TP_5446_FW);
	ftxxxx_irq_enable(ftxxxx_ts->client);
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	return count;
}

static ssize_t ftxxxx_fwupgradeapp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	size_t count = 0;
	if (fw_update_complete) {
		printk("[Touch_H] FW Update Complete \n");
	} else {
		printk("[Touch_H] FW Update Fail \n");
	}
	count += sprintf(buf, "%d\n", fw_update_complete);

	return count;
}

static ssize_t update_progress_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	return sprintf(buf, "%d\n", ((fw_update_progress*100)/fw_update_total_count));
}


/*upgrade from app.bin*/
static ssize_t ftxxxx_fwupgradeapp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int err_tmp = 0;
	char fwname[128];
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';

	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	ftxxxx_nosync_irq_disable(ftxxxx_ts->client);

	err_tmp = fts_ctpm_fw_upgrade_with_app_file(ftxxxx_ts->client, fwname);

	if (err_tmp != 0)
		printk("[Touch] touch update fw fail ! \n");

	asus_check_touch_mode();

	ftxxxx_irq_enable(ftxxxx_ts->client);

	mutex_unlock(&ftxxxx_ts->g_device_mutex);

	wake_unlock(&ftxxxx_ts->wake_lock);

	return count;
}

static ssize_t ftxxxx_ftsgetprojectcode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	char projectcode[32];
	/*struct i2c_client *client = container_of(dev, struct i2c_client, dev);*/
	memset(projectcode, 0, sizeof(projectcode));
	/*mutex_lock(&g_device_mutex);
	if(ft5x36_read_project_code(client, projectcode) < 0)
	num_read_chars = snprintf(buf, PAGE_SIZE, "get projcet code fail!\n");
	else
	num_read_chars = snprintf(buf, PAGE_SIZE, "projcet code = %s\n", projectcode);
	mutex_unlock(&g_device_mutex);
	*/
	return num_read_chars;
}

static ssize_t ftxxxx_ftsgetprojectcode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* place holder for future use */
	return -EPERM;
}

static ssize_t fts_fwupgradeall_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int err_tmp = 0;
	char fwname[128];
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';

	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	ftxxxx_nosync_irq_disable(ftxxxx_ts->client);

	err_tmp = fts_ctpm_fw_upgrade_with_all_file(ftxxxx_ts->client, fwname);

	if (err_tmp != 0)
		printk("[Touch] touch all.bin update fw fail  ! \n");

	asus_check_touch_mode();

	ftxxxx_irq_enable(ftxxxx_ts->client);

	mutex_unlock(&ftxxxx_ts->g_device_mutex);

	wake_unlock(&ftxxxx_ts->wake_lock);

	return count;
}

#ifdef FTS_SELF_TEST
#define FTXXXX_INI_FILEPATH "/asusfw/touch"
//#if 0
static int ftxxxx_GetInISize(char *config_name)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize = 0;
	char filepath[128];

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", config_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		pr_err("[Focal][TOUCH_ERR] %s : error occured while opening file %s. \n", __func__, filepath);
		return -EIO;
	}
	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	filp_close(pfile, NULL);
	return fsize;
}

static int ftxxxx_ReadInIData(char *config_name, char *config_buf)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize;
	char filepath[128];
	loff_t pos;
	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", config_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		pr_err("[Focal][TOUCH_ERR] %s : error occured while opening file %s. \n", __func__, filepath);
		return -EIO;
	}
	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(pfile, config_buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);
	return 0;
}
static int ftxxxx_get_testparam_from_ini(char *config_name)
{
	char *filedata = NULL;

	int inisize = ftxxxx_GetInISize(config_name);
	printk("[Focal][Touch] %s : inisize = %d \n ", __func__, inisize);
	if (inisize <= 0) {
		pr_err("[Focal][TOUCH_ERR] %s : ERROR : Get firmware size failed \n", __func__);
		return -EIO;
	}
	filedata = kmalloc(inisize + 1, GFP_ATOMIC);
	if (ftxxxx_ReadInIData(config_name, filedata)) {
		pr_err("[Focal][TOUCH_ERR] %s() - ERROR: request_firmware failed \n", __func__);
		kfree(filedata);
		return -EIO;
	} else {
		pr_info("[Focal][Touch] %s : ftxxxx_ReadInIData successful \n", __func__);
	}
	SetParamData(filedata);
	return 0;
}
//#endif
static ssize_t ftxxxx_ftsscaptest_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	int count = 0, frame_flag = 0;
	int err = 0;
	int i = 0, j = 0, space = (11 + TX_NUM);
	int flag[8];
	int BufLen = 80 * 1024;
	char *tmpbuf = NULL;
	int result=0;
	
	struct file *filp = NULL;
	mm_segment_t oldfs = { 0 };

	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	tmpbuf = vmalloc(BufLen);

	if (tmpbuf == NULL) {
		printk("alloc data buf fail !\n");
		return 0;
	}

	flag[0] = SCab_1;
	flag[1] = SCab_2;
	flag[2] = SCab_3;
	flag[3] = SCab_4;
	flag[4] = SCab_5;
	flag[5] = SCab_6;
	flag[6] = SCab_7;
	flag[7] = SCab_8;
	filp = filp_open("/mnt/sdcard/TP_Raw_data.csv", O_RDWR | O_CREAT, S_IRUSR);
	if (IS_ERR(filp)) {
		printk("[Focal][TOUCH_ERR] %s: open /data/TP_Raw_data failed\n", __func__);
		mutex_unlock(&ftxxxx_ts->g_device_mutex);
		wake_unlock(&ftxxxx_ts->wake_lock);
		vfree(tmpbuf);
		return 0;
	}
	oldfs = get_fs();
	set_fs(get_ds());
	focal_save_scap_sample1();
	for(i=0;i<2;i++)
	{
		if((flag[2*i]+flag[2*i+1])>0)
		{
			total_item++;
		}
	}
	
	for(i=2;i<4;i++)
	{
		if((flag[2*i]+flag[2*i+1])>0)
		{
			total_item++;
		}
	}
	/*count += sprintf(buf + count,"TestItem Num, 3, RawData Test, 7, %d, %d, 11, 1, SCap CB Test, 9, %d, %d, %d, 1, SCap CB Test, 9, %d, %d, %d, 2, SCap RawData Test, 10, %d, %d, %d, 1, SCap RawData Test, 10, %d, %d, %d, 2\n",TX_NUM,RX_NUM,(SCab_1+SCab_2),RX_NUM,(11+TX_NUM),(SCab_3+SCab_4),RX_NUM,(11+TX_NUM+SCab_1+SCab_2),(SCab_5+SCab_6),RX_NUM,(11+TX_NUM+SCab_1+SCab_2+SCab_3+SCab_4),(SCab_7+SCab_8),RX_NUM,(11+TX_NUM+SCab_1+SCab_2+SCab_3+SCab_4+SCab_5+SCab_6));*/

	count += snprintf(tmpbuf + count, BufLen - count, "ECC, 85, 170, IC Name, FT5X46, IC Code, 21\n");
	count += snprintf(tmpbuf + count, BufLen - count, "TestItem Num, %d, RawData Test, 7, %d, %d, 11, 2, ", total_item, TX_NUM, RX_NUM);
	count += snprintf(tmpbuf + count, BufLen - count," RawData Uniformity Test, 16, %d, %d, %d, 1, ",TX_NUM,RX_NUM,space);
	space=space+TX_NUM;
	count += snprintf(tmpbuf + count, BufLen - count," RawData Uniformity Test, 16, %d, %d, %d, 2, ",TX_NUM,RX_NUM,space);

	space=space+TX_NUM;
	frame_flag = 0;
	for (i = 0; i < 2; i++) {
		if ((flag[2*i] + flag[2*i + 1]) > 0) {
			frame_flag++;
			count += snprintf(tmpbuf + count, BufLen - count, "SCap CB Test, 9, %d, %d, %d, %d, ", (flag[2*i] + flag[2*i+1]), RX_NUM, space, frame_flag);
			space += flag[2*i] + flag[2*i + 1];
		}
	}
	frame_flag = 0;
	for (i = 2; i < 4; i++) {
		if ((flag[2*i] + flag[2*i + 1]) > 0) {
			frame_flag++;
			count += snprintf(tmpbuf + count, BufLen - count, "SCap RawData Test, 10, %d, %d, %d, %d, ", (flag[2*i] + flag[2*i + 1]), RX_NUM, space, frame_flag);
			space += flag[2*i] + flag[2*i + 1];
		}
	}

	count += snprintf(tmpbuf + count, BufLen - count, "\n");
	for (i = 0; i < 8; i++)
		count += snprintf(tmpbuf + count, BufLen - count, "\n");
	printk("[Focal][%s]	123 data result = %d !\n", __func__, err);

	for (i = 0; i < TX_NUM; i++) {
		
		for (j = 0; j < RX_NUM; j++) {

			count += snprintf(tmpbuf + count, BufLen - count, "%5d, ", TPrawdata[i][j]);
			/*msleep(2000);*/
		}
		/*msleep(10000);*/
		count += snprintf(tmpbuf + count, BufLen - count, "\n");
	}

	for (i = 0; i < TX_NUM; i++) 
	{
		
		for (j = 0; j < RX_NUM; j++) 
		{
			count += snprintf(tmpbuf + count, BufLen - count,"%5d, ",  TxLinearity[i][j]);
			//msleep(2000);
		}
		//msleep(10000);
		count += snprintf(tmpbuf + count, BufLen - count,"\n");
	}

	for (i = 0; i < TX_NUM; i++) 
	{
		
		for (j = 0; j < RX_NUM; j++) 
		{
			count += snprintf(tmpbuf + count, BufLen - count,"%5d, ",  RxLinearity[i][j]);
			//msleep(2000);
		}
		//msleep(10000);
		count += snprintf(tmpbuf + count, BufLen - count,"\n");
	}

	for (i = TX_NUM; i < TX_NUM+8; i++) 
	{
		if(i>=TX_NUM && flag[i-TX_NUM]==0)
		{			
			continue;
		}
		for (j = 0; j < RX_NUM; j++) 
		{
			if(i>=TX_NUM && j==TX_NUM && ((i-TX_NUM)%2)==1)
			{			
				break;
			}
			
				
			count += snprintf(tmpbuf + count, BufLen - count,"%5d, ",  TPrawdata[i][j]);
			//msleep(2000);
		}
		//msleep(10000);
		count += snprintf(tmpbuf + count, BufLen - count,"\n");
	}
	/*Print RawData End*/

	filp->f_op->write(filp, tmpbuf, count, &filp->f_pos);
	set_fs(oldfs);
	filp_close(filp, NULL);

	msleep(1000);

	filp = filp_open("/mnt/sdcard/TP_Raw_data.txt", O_RDWR | O_CREAT, S_IRUSR);
	if (IS_ERR(filp)) {
		printk("[Focal][TOUCH_ERR] %s: open /data/TP_Raw_data.txt failed\n", __func__);
		mutex_unlock(&ftxxxx_ts->g_device_mutex);
		wake_unlock(&ftxxxx_ts->wake_lock);
		vfree(tmpbuf);
		return 0;
	}
	oldfs = get_fs();
	set_fs(get_ds());
	printk("[Focal][%s]	dump data to CVS count = %d !\n", __func__, count);
	count = 0;
	for (i = 0; i < TX_NUM; i++) {
		
		for (j = 0; j < RX_NUM; j++) {

			count += snprintf(tmpbuf + count, BufLen - count, "%5d, ", TPrawdata[i][j]);
			/*msleep(2000);*/
		}
		/*msleep(10000);*/
		count += snprintf(tmpbuf + count, BufLen - count, "\n");
	}

	for (i = 0; i < TX_NUM; i++) 
	{
		
		for (j = 0; j < RX_NUM; j++) 
		{
			count += snprintf(tmpbuf + count, BufLen - count,"%5d, ",  TxLinearity[i][j]);
			//msleep(2000);
		}
		//msleep(10000);
		count += snprintf(tmpbuf + count, BufLen - count,"\n");
	}

	for (i = 0; i < TX_NUM; i++) 
	{
		
		for (j = 0; j < RX_NUM; j++) 
		{
			count += snprintf(tmpbuf + count, BufLen - count,"%5d, ",  RxLinearity[i][j]);
			//msleep(2000);
		}
		//msleep(10000);
		count += snprintf(tmpbuf + count, BufLen - count,"\n");
	}

	for (i = TX_NUM; i < TX_NUM+8; i++) 
	{
		if(i>=TX_NUM && flag[i-TX_NUM]==0)
		{			
			continue;
		}
		for (j = 0; j < RX_NUM; j++) 
		{
			if(i>=TX_NUM && j==TX_NUM && ((i-TX_NUM)%2)==1)
			{			
				break;
			}
			
				
			count += snprintf(tmpbuf + count, BufLen - count,"%5d, ",  TPrawdata[i][j]);
			//msleep(2000);
		}
		//msleep(10000);
		count += snprintf(tmpbuf + count, BufLen - count,"\n");
	}
	/*Print RawData End*/

	filp->f_op->write(filp, tmpbuf, count, &filp->f_pos);
	set_fs(oldfs);
	filp_close(filp, NULL);
	printk("[Focal][%s]	dump data to TXT count = %d !\n", __func__, count);
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	vfree(tmpbuf);
/*zax 20141116 --------------------*/
	result=TP_TEST_RESULT;
	if (result == 2) {
		return sprintf(buf, "TP_Raw_Data_Test_PASS ! \n");
	} else if (result == 1) {
		return sprintf(buf, "TP_Raw_Data_Test_Fail ! \n");
	} else {
		return sprintf(buf, "Please_echo_ini_file_first ! \n");
	}
}

static ssize_t ftxxxx_ftsscaptest_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	/* place holder for future use */
	char cfgname[128];
	int i=0;

	struct i2c_client *client = container_of(dev, struct i2c_client, dev);

	/*	struct i2c_client *client = container_of(dev, struct i2c_client, dev);*/
	wake_lock(&ftxxxx_ts->wake_lock);

	mutex_lock(&ftxxxx_ts->g_device_mutex);

	ftxxxx_nosync_irq_disable(ftxxxx_ts->client);

	memset(cfgname, 0, sizeof(cfgname));
	sprintf(cfgname, "%s", buf);
	cfgname[count-1] = '\0';
	/*
	//Init_I2C_Read_Func(FTS_I2c_Read);
	//Init_I2C_Write_Func(FTS_I2c_Write);
	//StartTestTP();
	*/
	Init_I2C_Write_Func(FTS_I2c_Write);
	Init_I2C_Read_Func(FTS_I2c_Read);

	if (ftxxxx_get_testparam_from_ini(cfgname) < 0)
		printk("[Focal][TOUCH_ERR] %s : get testparam from ini failure \n", __func__);
	else {
		printk("[Focal][Touch] %s : tp test Start... \n", __func__);
		if (true == StartTestTP()) {
			printk("[Focal][Touch] %s : tp test pass \n", __func__);
			TP_TEST_RESULT = 2;
		} else {
			printk("[Focal][Touch] %s : tp test failure \n", __func__);
			TP_TEST_RESULT = 1;
			//TP_TEST_RESULT = 2;
		}
/*		GetData_RawDataTest(&TestData, &iTxNumber, &iRxNumber, 0xff, 0xff);
		printk("TestData Addr 0x%5X  \n", (unsigned int)TestData);
*/
		for (i = 0; i < 3; i++) {
			if (ftxxxx_write_reg(client, 0x00, 0x00) >= 0)
				break;
			else
			msleep(200);
		}

		FreeTestParamData();
	}
	ftxxxx_irq_enable(ftxxxx_ts->client);
	mutex_unlock(&ftxxxx_ts->g_device_mutex);
	wake_unlock(&ftxxxx_ts->wake_lock);
	return count;
}
#endif
/****************************************/
/* sysfs */
/*get the fw version
*example:cat ftstpfwver
*/
static DEVICE_ATTR(ftstpfwver, Focal_RW_ATTR, ftxxxx_tpfwver_show, ftxxxx_tpfwver_store);
/*upgrade from *.i
*example: echo 1 > ftsfwupdate
*/
static DEVICE_ATTR(ftsfwupdate, Focal_RW_ATTR, ftxxxx_fwupdate_show, ftxxxx_fwupdate_store);
/*read and write register
*read example: echo 88 > ftstprwreg ---read register 0x88
*write example:echo 8807 > ftstprwreg ---write 0x07 into register 0x88
*
*note:the number of input must be 2 or 4.if it not enough,please fill in the 0.
*/
static DEVICE_ATTR(ftstprwreg, Focal_RW_ATTR, ftxxxx_tprwreg_show, ftxxxx_tprwreg_store);

/* +++ ASUS add for read write Double tapsetting config +++ */
static DEVICE_ATTR(DoubleTapConfig, Focal_RW_ATTR, DoubleTapConfig_show, DoubleTapConfig_store);
/* --- ASUS add for read write Double tapsetting config --- */

/*upgrade from app.bin
*example:echo "*_app.bin" > ftsfwupgradeapp
*/
static DEVICE_ATTR(ftsfwupgradeapp, Focal_RW_ATTR, ftxxxx_fwupgradeapp_show, ftxxxx_fwupgradeapp_store);
static DEVICE_ATTR(ftsgetprojectcode, Focal_RW_ATTR, ftxxxx_ftsgetprojectcode_show, ftxxxx_ftsgetprojectcode_store);
#ifdef FTS_SELF_TEST
static DEVICE_ATTR(ftsscaptest, Focal_RW_ATTR, ftxxxx_ftsscaptest_show, ftxxxx_ftsscaptest_store);
#endif
static DEVICE_ATTR(ftresetic, Focal_RW_ATTR, NULL, ftxxxx_ftreset_ic);
static DEVICE_ATTR(ftinitstatus, Focal_RW_ATTR, ftxxxx_init_show, NULL);
static DEVICE_ATTR(dump_tp_raw_data, Focal_RW_ATTR, asus_dump_tp_raw_data_show, asus_dump_tp_raw_data_store);
static DEVICE_ATTR(dump_tp_raw_data_to_files, Focal_RW_ATTR, asus_dump_tp_raw_data_to_file, asus_dump_tp_raw_data_store);
static DEVICE_ATTR(TP_ID, Focal_RW_ATTR, asus_get_tpid, NULL);
static DEVICE_ATTR(tp_id_ic, Focal_RW_ATTR, asus_get_tpid_ic_show, NULL);
static DEVICE_ATTR(INR_STATUS, Focal_RW_ATTR, asus_itr_status_show, NULL);
static DEVICE_ATTR(tp_fw_info, Focal_RW_ATTR, fw_info_show, NULL);
static DEVICE_ATTR(update_progress, Focal_RW_ATTR, update_progress_show, NULL);
static DEVICE_ATTR(set_reset_pin_level, Focal_RW_ATTR, set_reset_pin_level_show, set_reset_pin_level_store);
static DEVICE_ATTR(glove_mode, Focal_RW_ATTR, switch_glove_mode_show, switch_glove_mode_store);
static DEVICE_ATTR(cover_mode, Focal_RW_ATTR, switch_cover_mode_show, switch_cover_mode_store);
static DEVICE_ATTR(keyboard_enable, Focal_RW_ATTR, switch_keyboard_mode_show, switch_keyboard_mode_store);
static DEVICE_ATTR(dclick_mode, Focal_RW_ATTR, switch_dclick_mode_show, switch_dclick_mode_store);
static DEVICE_ATTR(swipeup_mode, Focal_RW_ATTR, switch_swipeup_mode_show, switch_swipeup_mode_store);
static DEVICE_ATTR(gesture_mode, Focal_RW_ATTR, switch_gesture_mode_show, switch_gesture_mode_store);
static DEVICE_ATTR(keypad_mode, Focal_RW_ATTR, switch_keypad_mode_show, switch_keypad_mode_store);
static DEVICE_ATTR(HW_ID, Focal_RW_ATTR, asus_get_hwid_show, NULL);
static DEVICE_ATTR(IRQ_FORCE_DISABLE, Focal_RW_ATTR, irq_disable_show, irq_disable_store);
static DEVICE_ATTR(IRQ_status, Focal_RW_ATTR, irq_status_show, NULL);
static DEVICE_ATTR(Enable_Proximyty_Check, Focal_RW_ATTR, enable_proximity_check_show, enable_proximity_check_store);
static DEVICE_ATTR(FT_ID, Focal_RW_ATTR, ftxxxx_ftid_show, NULL);
static DEVICE_ATTR(Touch_IC_ID, Focal_RW_ATTR, ftxxxx_touchic_show, NULL);
/*read force touch location pressure
echo 12 >ftreadpre
(0:0.5kg 1:1kg 
 1-9 :location)
*/


static DEVICE_ATTR(ftreadpre, Focal_RW_ATTR,forcetouch_read_show,forcetouch_read_store);
static DEVICE_ATTR(force_touch_cal, Focal_RW_ATTR,forcetouch_cal_show,NULL);
static DEVICE_ATTR(force_touch_cal_data, Focal_RW_ATTR,forcetouch_cal_data_show,forcetouch_cal_data_store);
static DEVICE_ATTR(force_touch_erase, Focal_RW_ATTR,forcetouch_erase_show,NULL);

//static DEVICE_ATTR(dump_force, Focal_RW_ATTR,forcetouch_cal_show,NULL);
/*upgrade from all.bin
*example:echo "*_all.bin" > ftsfwupgradeall
*/
static DEVICE_ATTR(ftsfwupgradeall, Focal_RW_ATTR,NULL,fts_fwupgradeall_store);
/*add your attr in here*/
static struct attribute *ftxxxx_attributes[] = {
	&dev_attr_ftstpfwver.attr,
	&dev_attr_ftsfwupdate.attr,
	&dev_attr_ftstprwreg.attr,
	&dev_attr_DoubleTapConfig.attr,
	&dev_attr_ftsfwupgradeapp.attr,
	&dev_attr_ftsgetprojectcode.attr,
#ifdef FTS_SELF_TEST
	&dev_attr_ftsscaptest.attr,
#endif
	&dev_attr_ftresetic.attr,
	&dev_attr_ftinitstatus.attr,
	&dev_attr_dump_tp_raw_data.attr,
	&dev_attr_dump_tp_raw_data_to_files.attr,
	&dev_attr_TP_ID.attr,
	&dev_attr_INR_STATUS.attr,
	&dev_attr_tp_fw_info.attr,
	&dev_attr_update_progress.attr,
	&dev_attr_set_reset_pin_level.attr,
	&dev_attr_glove_mode.attr,
	&dev_attr_keyboard_enable.attr,
	&dev_attr_cover_mode.attr,
	&dev_attr_gesture_mode.attr,
	&dev_attr_keypad_mode.attr,
	&dev_attr_HW_ID.attr,
	&dev_attr_dclick_mode.attr,
	&dev_attr_swipeup_mode.attr,
	&dev_attr_IRQ_FORCE_DISABLE.attr,
	&dev_attr_IRQ_status.attr,
	&dev_attr_Enable_Proximyty_Check.attr,
	&dev_attr_ftsfwupgradeall.attr,
	&dev_attr_FT_ID.attr,
	&dev_attr_tp_id_ic.attr,
	&dev_attr_ftreadpre.attr,
	&dev_attr_force_touch_cal.attr,
	&dev_attr_force_touch_cal_data.attr,
	&dev_attr_force_touch_erase.attr,
	&dev_attr_Touch_IC_ID.attr,
	NULL
};

static struct attribute_group ftxxxx_attribute_group = {
	.attrs = ftxxxx_attributes
};

/*create sysfs for debug*/
int ftxxxx_create_sysfs(struct i2c_client * client)
{
	int err;
	G_Client = client;
	err = sysfs_create_group(&client->dev.kobj, &ftxxxx_attribute_group);
	if (0 != err) {
		dev_err(&client->dev, "[Focal][TOUCH_ERR] %s() - ERROR: sysfs_create_group() failed.error code: %d\n", __FUNCTION__, err);
		sysfs_remove_group(&client->dev.kobj, &ftxxxx_attribute_group);
		return -EIO;
	} else {
		dev_dbg(&client->dev, "[Focal][Touch] %s() - sysfs_create_group() succeeded. \n", __FUNCTION__);
	}

	android_touch_kobj = kobject_create_and_add("android_touch", NULL);
	if (android_touch_kobj == NULL) {
		printk("[Focal][TOUCH_ERR] : subsystem_register failed\n");
		return -1;
	}

	if (sysfs_create_file(android_touch_kobj, &dev_attr_ftsfwupgradeapp.attr)) {
		printk("[Focal][TOUCH_ERR] : create_file ftsfwupgradeapp failed\n");
		return -1;
	}

	HidI2c_To_StdI2c(client);
	return err;
}

int ftxxxx_remove_sysfs(struct i2c_client * client)
{
	sysfs_remove_group(&client->dev.kobj, &ftxxxx_attribute_group);
	return 0;
}

#ifdef FTS_APK_DEBUG
/*create apk debug channel*/
#define PROC_UPGRADE			0
#define PROC_READ_REGISTER		1
#define PROC_WRITE_REGISTER		2
#define PROC_AUTOCLB			4
#define PROC_UPGRADE_INFO		5
#define PROC_WRITE_DATA			6
#define PROC_READ_DATA			7
#define PROC_NAME	"ftxxxx-debug"

static unsigned char proc_operate_mode = PROC_UPGRADE;
static struct proc_dir_entry *ftxxxx_proc_entry;
/*interface of write proc*/
static int ftxxxx_debug_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
{
	struct i2c_client *client = (struct i2c_client *)ftxxxx_ts->client;
	unsigned char writebuf[FTS_PACKET_LENGTH];
	int buflen = len;
	int writelen = 0;
	int ret = 0;
	char upgrade_file_path[128];

	if (copy_from_user(&writebuf, buff, buflen)) {
		dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : copy from user error \n", __func__);
		return -EFAULT;
	}
	proc_operate_mode = writebuf[0] - 48;
	printk("[Focal][Touch] %s : proc write proc_operate_mode = %d !! \n", __func__, proc_operate_mode);
	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
		sprintf(upgrade_file_path, "%s", writebuf + 1);
		upgrade_file_path[buflen-1] = '\0';
		FTS_DBG("%s\n", upgrade_file_path);
		ftxxxx_irq_disable(ftxxxx_ts->client);
		ret = fts_ctpm_fw_upgrade_with_app_file(client, upgrade_file_path);
		ftxxxx_irq_enable(ftxxxx_ts->client);
		if (ret < 0) {
			dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : upgrade failed. \n", __func__);
			return ret;
		}
		break;
	case PROC_READ_REGISTER:
		writelen = 1;
		ret = ftxxxx_i2c_Write(client, writebuf + 1, writelen);
		if (ret < 0) {
			dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : write iic error \n", __func__);
			return ret;
		}
		break;
	case PROC_WRITE_REGISTER:
		writelen = 2;
		ret = ftxxxx_i2c_Write(client, writebuf + 1, writelen);
		if (ret < 0) {
			dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : write iic error \n", __func__);
			return ret;
		}
		break;
	case PROC_AUTOCLB:
		FTS_DBG("[Touch] %s : autoclb\n", __func__);
		fts_ctpm_auto_clb(client);
		break;
	case PROC_READ_DATA:
	case PROC_WRITE_DATA:
		if(writelen>0)
		{
			writelen = len - 1;
			ret = ftxxxx_i2c_Write(client, writebuf + 1, writelen);
			if (ret < 0) {
				dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : write iic error\n", __func__);
				return ret;
			}
		}
		break;
	default:
		break;
	}
	return len;
}

/*interface of read proc*/
bool proc_read_status;
static ssize_t ftxxxx_debug_read(struct file *filp, char __user *buff, unsigned long len, void *data)
{
	struct i2c_client *client = (struct i2c_client *)ftxxxx_ts->client;
	int ret = 0;
	unsigned char buf[PAGE_SIZE];
	/*unsigned char *buf=NULL;*/
	ssize_t num_read_chars = 0;
	int readlen = 0;
	u8 regvalue = 0x00, regaddr = 0x00;

	/*buf = kmalloc(PAGE_SIZE, GFP_KERNEL);*/
	switch (proc_operate_mode) {
	case PROC_UPGRADE:
		/*after calling ftxxxx_debug_write to upgrade*/
		regaddr = 0xA6;
		ret = ftxxxx_read_reg(client, regaddr, &regvalue);
		if (ret < 0)
			num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
		else
			num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
		break;
	case PROC_READ_REGISTER:
		readlen = 1;
		ret = ftxxxx_i2c_Read(client, NULL, 0, buf, readlen);
		if (ret < 0) {
			dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : read iic error\n", __func__);
			return ret;
		}
		num_read_chars = 1;
		break;
	case PROC_READ_DATA:
		readlen = len;
		ret = ftxxxx_i2c_Read(client, NULL, 0, buf, readlen);
		if (ret < 0) {
			dev_err(&client->dev, "[Focal][TOUCH_ERR] %s : read iic error\n", __func__);
			return ret;
		}
		num_read_chars = readlen;
		break;
	case PROC_WRITE_DATA:
		break;
	default:
		break;
	}
	copy_to_user(buff, buf, num_read_chars);
	/*memcpy(buff, buf, num_read_chars);*/
	/*kfree(buf);*/
	if (!proc_read_status) {
		proc_read_status = 1;
		return num_read_chars;
	} else {
		proc_read_status = 0;
		return 0;
	}
}

static const struct file_operations debug_flag_fops = {
		.owner = THIS_MODULE,
		/*.open = proc_chip_check_running_open,*/
		.read = ftxxxx_debug_read,
		.write = ftxxxx_debug_write,
};

int ftxxxx_create_apk_debug_channel(struct i2c_client * client)
{
	ftxxxx_proc_entry = proc_create(PROC_NAME, 0666, NULL, &debug_flag_fops);
	/*ftxxxx_proc_entry = proc_create(PROC_NAME, 0777, NULL);*/
	if (NULL == ftxxxx_proc_entry) {
		dev_err(&client->dev, "[Focal][TOUCH_ERR] %s: Couldn't create proc entry!\n", __func__);
		return -ENOMEM;
	} else {
		dev_info(&client->dev, "[Focal][Touch] %s: Create proc entry success! \n", __func__);

/*		ftxxxx_proc_entry->data = client;
		ftxxxx_proc_entry->write_proc = ftxxxx_debug_write;
		ftxxxx_proc_entry->read_proc = ftxxxx_debug_read;
*/
	}
	return 0;
}

void ftxxxx_release_apk_debug_channel(void)
{
	if (ftxxxx_proc_entry)
		remove_proc_entry(PROC_NAME, NULL);
}
#endif
/*asus */

int fts_ctpm_fw_upgrade_ReadVendorID(struct i2c_client *client, u8 *ucPVendorID)
{
	u8 reg_val[4] = {0};
	u32 i = 0;
	u8 auc_i2c_write_buf[10];
	int i_ret;
	*ucPVendorID = 0;

	i_ret = HidI2c_To_StdI2c(client);
	if (i_ret == 0) {
		FTS_DBG("[TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
		return -EIO;
	}
	fts_get_upgrade_info(&upgradeinfo);
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset  CTPM *****/
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(upgradeinfo.delay_aa);
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_55);
		msleep(200);
		/*********Step 2:Enter upgrade mode *****/
		i_ret = HidI2c_To_StdI2c(client);
		if (i_ret == 0) {
			FTS_DBG("[TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
			/*continue;*/
		}
		msleep(10);
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
		if (i_ret < 0) {
			FTS_DBG("[TOUCH_ERR] %s : failed writing  0x55 and 0xaa ! \n", __func__);
			continue;
		}
		/*********Step 3:check READ-ID***********************/
		msleep(10);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;
		reg_val[0] = reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
		if (reg_val[0] == upgradeinfo.upgrade_id_1 && reg_val[1] == upgradeinfo.upgrade_id_2) {
			FTS_DBG("[Touch] %s : Step 3: READ OK CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev, "[Touch] %s : Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			continue;
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;
	/*********Step 4: read vendor id from app param area***********************/
	msleep(10);
	auc_i2c_write_buf[0] = 0x03;
	auc_i2c_write_buf[1] = 0x00;
	auc_i2c_write_buf[2] = 0xd7;
	auc_i2c_write_buf[3] = 0x84;
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		ftxxxx_i2c_Write(client, auc_i2c_write_buf, 4);		/*send param addr*/
		msleep(5);
		reg_val[0] = reg_val[1] = 0x00;
		i_ret = ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 1);

		if (ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 1) < 0) {
			*ucPVendorID = 0;
			FTS_DBG("[TOUCH_ERR] %s : read vendor id from app param area error i_ret=%d \n", __func__, i_ret);
		} else {
			*ucPVendorID = reg_val[0];
			FTS_DBG("[Touch] %s : In upgrade Vendor ID, REG1 = 0x%x, REG2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			break;
		}
	}

	msleep(50);
	/*********Step 5: reset the new FW***********************/
	FTS_DBG("[Touch] %s : Step 5: reset the new FW \n", __func__);
	auc_i2c_write_buf[0] = 0x07;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(200);	/*make sure CTP startup normally */
	i_ret = HidI2c_To_StdI2c(client);	/*Android to Std i2c.*/
	if (i_ret == 0) {
		FTS_DBG("[TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
	}
	msleep(10);
	return 0;
}

int fts_ctpm_fw_upgrade_ReadProjectCode(struct i2c_client *client)
{
	u8 reg_val[4] = {0};
	u32 i = 0, j = 0;
	u8 auc_i2c_write_buf[10];
	int i_ret;

	memset(B_projectcode, 0, sizeof(B_projectcode));

	i_ret = HidI2c_To_StdI2c(client);
	if (i_ret == 0) {
		FTS_DBG("[TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
	}
	fts_get_upgrade_info(&upgradeinfo);
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset  CTPM *****/
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(upgradeinfo.delay_aa);
		ftxxxx_write_reg(client, 0xfc, FT_UPGRADE_55);
		msleep(200);
		/*********Step 2:Enter upgrade mode *****/
		i_ret = HidI2c_To_StdI2c(client);
		if (i_ret == 0) {
			FTS_DBG("[TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
			/*continue;*/
		}
		msleep(10);
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		i_ret = ftxxxx_i2c_Write(client, auc_i2c_write_buf, 2);
		if (i_ret < 0) {
			FTS_DBG("[TOUCH_ERR] %s : failed writing  0x55 and 0xaa ! \n", __func__);
			continue;
		}
		/*********Step 3:check READ-ID***********************/
		msleep(10);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;
		reg_val[0] = reg_val[1] = 0x00;
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
		if (reg_val[0] == upgradeinfo.upgrade_id_1 && reg_val[1] == upgradeinfo.upgrade_id_2) {
			FTS_DBG("[Touch] %s : Step 3: READ OK CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev, "[TOUCH_ERR] %s : Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x \n", __func__, reg_val[0], reg_val[1]);
			continue;
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;
	/*********Step 4: read vendor id from app param area***********************/
	msleep(10);
	/*auc_i2c_write_buf[0] = 0x03;
	auc_i2c_write_buf[1] = 0x00;
	auc_i2c_write_buf[2] = 0xd7;
	auc_i2c_write_buf[3] = 0x84;
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		ftxxxx_i2c_Write(client, auc_i2c_write_buf, 4);     //send param addr
		msleep(5);
		reg_val[0] = reg_val[1] = 0x00;
		i_ret = ftxxxx_i2c_Read(client, auc_i2c_write_buf, 0, reg_val, 2);
		if (G_ucVenderID != reg_val[0]) {
			*pProjectCode=0;
			DBG("In upgrade Vendor ID Mismatch, REG1 = 0x%x, REG2 = 0x%x, Definition:0x%x, i_ret=%d\n", reg_val[0], reg_val[1], G_ucVenderID, i_ret);
		} else {
			*pProjectCode=reg_val[0];
			DBG("In upgrade Vendor ID, REG1 = 0x%x, REG2 = 0x%x\n", reg_val[0], reg_val[1]);
			break;
		}
	}
	*/
	/*read project code*/
	for (j = 0; j < 10; j++) {
		msleep(10);
		auc_i2c_write_buf[0] = 0x03;
		auc_i2c_write_buf[1] = 0x00;
/*
		//if (is_5336_new_bootloader == BL_VERSION_Z7 || is_5336_new_bootloader == BL_VERSION_GZF)
			//temp = 0x07d0 + j;
		//else
*/
		auc_i2c_write_buf[2] = 0xD7;
		auc_i2c_write_buf[3] = 0xA0;
		ftxxxx_i2c_Write(client, auc_i2c_write_buf, 4);		/*send param addr*/
		ftxxxx_i2c_Read(client, auc_i2c_write_buf, 0, B_projectcode, 8);
	}

	printk("[Focal][Touch] %s : project code =  %s  \n", __func__, B_projectcode);

	msleep(50);
	/*********Step 5: reset the new FW***********************/
	FTS_DBG("[Focal][Touch] %s : Step 5: reset the new FW \n", __func__);
	auc_i2c_write_buf[0] = 0x07;
	ftxxxx_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(200);	/*make sure CTP startup normally */
	i_ret = HidI2c_To_StdI2c(client);	/*Android to Std i2c.*/
	if (i_ret == 0) {
		FTS_DBG("[Focal][TOUCH_ERR] %s : HidI2c change to StdI2c fail ! \n", __func__);
	}
	msleep(10);
	return 0;
}
/*asus*/

/* +++ asus jacob add for auto fw update +++ */


/*
int focal_fw_auto_update(struct i2c_client *client)
{
	int err_code = 0;
//	int tar_fw_ver = 0;
//	int tar_vendor_config = 0;
	u8 tar_vendor_id = 0;
	
	// 1. get touch ic tp vendor 
	g_vendor_id = ftxxxx_read_tp_id();
	if (g_vendor_id == 0xFF) {
		//printk("[TOUCH_ERR] %s : Read Vendor ID fail ! \n", __func__);
		//return 2;
		printk("[Focal][Touch] %s : Vendor ID = %x \n", __func__, g_vendor_id);
		printk("[Focal][Touch] touch IC=5446 \n");
		wake_lock(&ftxxxx_ts->wake_lock);
		mutex_lock(&ftxxxx_ts->g_device_mutex);
		fw_size = sizeof(TP_5446_FW);
		err_code = fts_ctpm_auto_upgrade(client, TP_5446_FW);
	}
	else{
		printk("[Focal][Touch] %s : Vendor ID = %x \n", __func__, g_vendor_id);
		
		if(HD_Modle==2)//sr
		{
			wake_lock(&ftxxxx_ts->wake_lock);
			mutex_lock(&ftxxxx_ts->g_device_mutex);
			switch (g_vendor_id)
			{
			case TP_BIEL_TM :
				//fw_size = sizeof(BIEL_TM_FW);
				//err_code=fts_ctpm_fw_upgrade_with_app_file(client, /sdcard/ASUS_LIBRA_3C47U_0x69_0x01_20151110_app.bin);
				//err_code = fts_ctpm_auto_upgrade(client, BIEL_TM_FW);
				break;
			case TP_BIEL_CTC :
				//fw_size = sizeof(BIEL_CTC_FW);
				//err_code = fts_ctpm_auto_upgrade(client, BIEL_CTC_FW);
				break;
			case TP_GIS_TM :
				//fw_size = sizeof(GIS_TM1_FW);
				//err_code = fts_ctpm_auto_upgrade(client, GIS_TM1_FW);
				break;
			case TP_GIS_CTC :
				//fw_size = sizeof(GIS_CTC_FW);
				//err_code = fts_ctpm_auto_upgrade(client, GIS_CTC_FW);
				break;
			case TP_JT_TM :
				//fw_size = sizeof(JT_TM1_FW);
				//err_code = fts_ctpm_auto_upgrade(client, JT_TM1_FW);
				break;	
			case TP_JT_AUO :
				//fw_size = sizeof(TP_5446_FW);
				//err_code = fts_ctpm_auto_upgrade(client, TP_5446_FW);
				break;
			}
		}else {
			if (gpio_get_value(ftxxxx_ts->gpio_lcd1id)==0){//AUO
				tar_vendor_id=(g_vendor_id& 0xf0) | 0x03;
			}else if(gpio_get_value(ftxxxx_ts->gpio_lcd1id)==1)
				tar_vendor_id=(g_vendor_id& 0xf0) | 0x01;
			else 
				printk("[Focal][Touch] get TPID wrong! \n");

			printk("[Focal][Touch] %s : ASUS judgment Vendor ID = %x \n", __func__, tar_vendor_id);
			g_vendor_id = tar_vendor_id;
			wake_lock(&ftxxxx_ts->wake_lock);
			mutex_lock(&ftxxxx_ts->g_device_mutex);
			switch (tar_vendor_id) {
			case TP_JT_TM :
				fw_size = sizeof(JT_TM_FW);
				err_code = fts_ctpm_auto_upgrade(client, JT_TM_FW);
				break;
			case TP_GIS_AUO :
				fw_size = sizeof(GIS_AUO_FW);
				err_code = fts_ctpm_auto_upgrade(client, GIS_AUO_FW);
				break;
			case TP_JT_AUO :
				fw_size = sizeof(JT_AUO_FW);
				err_code = fts_ctpm_auto_upgrade(client, JT_AUO_FW);
				break;
			case TP_GIS_TM :
				fw_size = sizeof(GIS_TM_FW);
				err_code = fts_ctpm_auto_upgrade(client, GIS_TM_FW);
				break;
			}
		}
	}
	mutex_unlock(&ftxxxx_ts->g_device_mutex);

	wake_unlock(&ftxxxx_ts->wake_lock);

	return err_code;
}*/

/* --- asus jacob add for auto fw update --- */


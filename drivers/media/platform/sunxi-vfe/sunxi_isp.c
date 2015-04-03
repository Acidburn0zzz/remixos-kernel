
/* 
 ***************************************************************************************
 * 
 * sunxi_isp.c
 * 
 * Hawkview ISP - sunxi_isp.c module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   3.0		  Yang Feng   	2014/12/11	ISP Tuning Tools Support
 * 
 ****************************************************************************************
 */

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>
#include "vfe_os.h"
#include "platform_cfg.h"
#include "lib/bsp_isp.h"
#include "sunxi_isp.h"
#include "vfe.h"

#define ISP_MODULE_NAME "sunxi_isp"
#define ISP_HEIGHT_16B_ALIGN 1
struct isp_dev *isp_gbl;

static int __isp_set_input_fmt_internal(enum bus_pixeltype type)
{
	enum isp_input_fmt fmt;
	enum isp_input_seq seq_t;
	switch (type) {

		/* yuv420 */
		case BUS_FMT_YY_YUYV:
			fmt = ISP_YUV420;
			seq_t = ISP_YUYV;
			break;
		case BUS_FMT_YY_YVYU:
			fmt = ISP_YUV420;
			seq_t = ISP_YVYU;
			break;
		case BUS_FMT_YY_UYVY:
			fmt = ISP_YUV420;
			seq_t = ISP_UYVY;
			break;
		case BUS_FMT_YY_VYUY:
			fmt = ISP_YUV420;
			seq_t = ISP_VYUY;
			break;

		/* yuv422 */
		case BUS_FMT_YUYV:
			fmt = ISP_YUV422;
			seq_t = ISP_YUYV;
			break;
		case BUS_FMT_YVYU:
			fmt = ISP_YUV422;
			seq_t = ISP_YVYU;
			break;
		case BUS_FMT_UYVY:
			fmt = ISP_YUV422;
			seq_t = ISP_UYVY;
			break;
		case BUS_FMT_VYUY:
			fmt = ISP_YUV422;
			seq_t = ISP_VYUY;
			break;

		/* raw */
		case BUS_FMT_SBGGR:
			fmt = ISP_RAW;
			seq_t = ISP_BGGR;
			break;
		case BUS_FMT_SGBRG:
			fmt = ISP_RAW;
			seq_t = ISP_GBRG;
			break;
		case BUS_FMT_SGRBG:
			fmt = ISP_RAW;
			seq_t = ISP_GRBG;
			break;
		case BUS_FMT_SRGGB:
			fmt = ISP_RAW;
			seq_t = ISP_RGGB;
			break;
		default:
			return -1;
			break;
	}
	//printk("fmt = %d,seq_t = %d\n",fmt,seq_t);
	bsp_isp_set_input_fmt(fmt, seq_t);
	return 0;
}

static int __isp_set_output_fmt_internal(enum pixel_fmt fmt, enum isp_channel ch)
{
	enum isp_output_fmt isp_fmt;
	enum isp_output_seq seq_t;
	//printk("output fmt = %d! \n seq_t= %d!\n", fmt,seq_t);
	switch (fmt) {

		/* yuv_p */
		case PIX_FMT_YUV422P_8:
			isp_fmt = ISP_YUV422_P;
			seq_t = ISP_UV;
			break;
		case PIX_FMT_YVU422P_8:
			isp_fmt = ISP_YUV422_P;
			seq_t = ISP_VU;
			break;
		case PIX_FMT_YUV420P_8:
			isp_fmt = ISP_YUV420_P;
			seq_t = ISP_UV;
			break;
		case PIX_FMT_YVU420P_8:
			isp_fmt = ISP_YUV420_P;
			seq_t = ISP_VU;
			break;

		/* yuv_sp */
		case PIX_FMT_YUV420SP_8:
			isp_fmt = ISP_YUV420_SP;
			seq_t = ISP_UV;
			break;
		case PIX_FMT_YVU420SP_8:
			isp_fmt = ISP_YUV420_SP;
			seq_t = ISP_VU;
			break;
		case PIX_FMT_YUV422SP_8:
			isp_fmt = ISP_YUV422_SP;
			seq_t = ISP_UV;
			break;
		case PIX_FMT_YVU422SP_8:
			isp_fmt = ISP_YUV422_SP;
			seq_t = ISP_VU;
			break;

		default:
			return -1;
			break;
	}
	bsp_isp_set_output_fmt(isp_fmt,seq_t,ch);
	return 0;
}

static int __isp_cal_ch_size(enum pixel_fmt fmt, struct isp_size *size,
  struct isp_yuv_size_addr_info *isp_size_info)
{
	switch(fmt)
	{
		case PIX_FMT_YUV420P_8:
		case PIX_FMT_YVU420P_8:
			isp_size_info->line_stride_y = ALIGN_16B(size->width);
			isp_size_info->line_stride_c = ALIGN_16B(isp_size_info->line_stride_y>>1);
			if(ISP_HEIGHT_16B_ALIGN) {
				isp_size_info->buf_height_y = ALIGN_16B(size->height);
			}else{
				isp_size_info->buf_height_y = size->height;
			}
			
			isp_size_info->buf_height_cb = isp_size_info->buf_height_y >>1;
			isp_size_info->buf_height_cr = isp_size_info->buf_height_y  >>1;

			isp_size_info->valid_height_y = size->height;
			isp_size_info->valid_height_cb = isp_size_info->valid_height_y >>1;
			isp_size_info->valid_height_cr = isp_size_info->valid_height_y  >>1;
			break;
		case PIX_FMT_YUV422P_8:
		case PIX_FMT_YVU422P_8:
			isp_size_info->line_stride_y = ALIGN_16B(size->width);
			isp_size_info->line_stride_c = ALIGN_16B(isp_size_info->line_stride_y>>1);
			if(ISP_HEIGHT_16B_ALIGN) {
				isp_size_info->buf_height_y = ALIGN_16B(size->height);
			}else{
				isp_size_info->buf_height_y = size->height;
			}
			isp_size_info->buf_height_cb = isp_size_info->buf_height_y;
			isp_size_info->buf_height_cr = isp_size_info->buf_height_y;
			
			isp_size_info->valid_height_y = size->height;
			isp_size_info->valid_height_cb = isp_size_info->valid_height_y;
			isp_size_info->valid_height_cr = isp_size_info->valid_height_y;
			break;
		case PIX_FMT_YUV420SP_8:
		case PIX_FMT_YVU420SP_8:
			isp_size_info->line_stride_y = ALIGN_16B(size->width);
			isp_size_info->line_stride_c = isp_size_info->line_stride_y;
			if(ISP_HEIGHT_16B_ALIGN) {
				isp_size_info->buf_height_y = ALIGN_16B(size->height);
			}else{
				isp_size_info->buf_height_y = size->height;
			}
			
			isp_size_info->buf_height_cb = isp_size_info->buf_height_y>>1;
			isp_size_info->buf_height_cr = 0;

			isp_size_info->valid_height_y = size->height;
			isp_size_info->valid_height_cb = isp_size_info->valid_height_y>>1;
			isp_size_info->valid_height_cr = 0;
			break;
		case PIX_FMT_YUV422SP_8:
		case PIX_FMT_YVU422SP_8:
			isp_size_info->line_stride_y = ALIGN_16B(size->width);
			isp_size_info->line_stride_c = isp_size_info->line_stride_y;
			if(ISP_HEIGHT_16B_ALIGN) {
				isp_size_info->buf_height_y = ALIGN_16B(size->height);
			}else{
				isp_size_info->buf_height_y = size->height;
			}
			isp_size_info->buf_height_cb = isp_size_info->buf_height_y;
			isp_size_info->buf_height_cr = 0;

			isp_size_info->valid_height_y = size->height;
			isp_size_info->valid_height_cb = isp_size_info->valid_height_y;
			isp_size_info->valid_height_cr = 0;
			break;
		default:
			break;
	}
	isp_size_info->isp_byte_size = isp_size_info->line_stride_y * isp_size_info->buf_height_y +
			isp_size_info->line_stride_c * isp_size_info->buf_height_cb +
			isp_size_info->line_stride_c * isp_size_info->buf_height_cr;
	return isp_size_info->isp_byte_size;
}

static int __isp_cal_ch_addr(enum enable_flag flip, unsigned int buf_base_addr,
		struct isp_yuv_size_addr_info *isp_size_info)
{
	isp_size_info->yuv_addr.y_addr = buf_base_addr;
	isp_size_info->yuv_addr.u_addr = isp_size_info->yuv_addr.y_addr + isp_size_info->line_stride_y * isp_size_info->buf_height_y;
	isp_size_info->yuv_addr.v_addr = isp_size_info->yuv_addr.u_addr + isp_size_info->line_stride_c * isp_size_info->buf_height_cb;
	if (flip == ENABLE)
	{
		isp_size_info->yuv_addr.y_addr = isp_size_info->yuv_addr.y_addr +
				isp_size_info->line_stride_y * isp_size_info->valid_height_y - isp_size_info->line_stride_y;
		isp_size_info->yuv_addr.u_addr = isp_size_info->yuv_addr.u_addr +
				isp_size_info->line_stride_c * isp_size_info->valid_height_cb - isp_size_info->line_stride_c;
		isp_size_info->yuv_addr.v_addr = isp_size_info->yuv_addr.v_addr +
				isp_size_info->line_stride_c * isp_size_info->valid_height_cr - isp_size_info->line_stride_c;
	}
	return 0;
}
static unsigned int __isp_new_set_size_internal(enum pixel_fmt *fmt, struct isp_size_settings *size_settings)
{
	int x_ratio,y_ratio,weight_shift;
	struct coor *ob_start = &size_settings->ob_start;
	struct isp_size *ob_black_size, *ob_valid_size, *full_size, *scale_size, *ob_rot_size;
	struct isp_yuv_size_addr_info *isp_yuv_size_addr = &isp_gbl->isp_yuv_size_addr[0];
	//ob zone config
	ob_black_size  = &size_settings->ob_black_size;
	ob_valid_size = &size_settings->ob_valid_size;
	full_size = &size_settings->full_size;
	scale_size = &size_settings->scale_size;
	ob_rot_size = &size_settings->ob_rot_size;

	bsp_isp_set_ob_zone(ob_black_size, ob_valid_size, ob_start ,ISP_SRC0);
	if(scale_size && scale_size->width != 0 && scale_size->height != 0)
	{
		//full size channel
		full_size->width  = full_size->width&0x1ffc;
		full_size->height = full_size->height&(~1);
		printk("[ISP] full_size width = %d, height = %d.\n",full_size->width,full_size->height);
		x_ratio = ob_valid_size->width*256/full_size->width;
		y_ratio = ob_valid_size->height*256/full_size->height;
		weight_shift = min_scale_w_shift(x_ratio,y_ratio);
		bsp_isp_channel_enable(MAIN_CH);
		bsp_isp_scale_enable(MAIN_CH);
		bsp_isp_set_output_size(MAIN_CH,full_size);
		bsp_isp_scale_cfg(MAIN_CH,x_ratio,y_ratio,weight_shift);

		__isp_cal_ch_size(fmt[MAIN_CH], full_size, &isp_yuv_size_addr[MAIN_CH]);
		bsp_isp_set_stride_y(isp_yuv_size_addr[MAIN_CH].line_stride_y, MAIN_CH);
		bsp_isp_set_stride_uv(isp_yuv_size_addr[MAIN_CH].line_stride_c, MAIN_CH);

		//scale channel
		//scale config
		scale_size->width  = scale_size->width&0x1ffc;//4 byte
		scale_size->height = scale_size->height&(~1);
		printk("[ISP] scale width = %d, height = %d\n",scale_size->width,scale_size->height);
		x_ratio = ob_valid_size->width*256/scale_size->width;
		y_ratio = ob_valid_size->height*256/scale_size->height;
		weight_shift = min_scale_w_shift(x_ratio,y_ratio);

		bsp_isp_channel_enable(SUB_CH);
		bsp_isp_scale_enable(SUB_CH);
		bsp_isp_set_output_size(SUB_CH,scale_size);
		bsp_isp_scale_cfg(SUB_CH,x_ratio,y_ratio,weight_shift);

		__isp_cal_ch_size(fmt[SUB_CH], scale_size, &isp_yuv_size_addr[SUB_CH]);

		bsp_isp_set_stride_y(isp_yuv_size_addr[SUB_CH].line_stride_y, SUB_CH);
		bsp_isp_set_stride_uv(isp_yuv_size_addr[SUB_CH].line_stride_c, SUB_CH);
	}
	else
	{
		//main channel, hardware use sub_ch
		//scale config
		full_size->width  = full_size->width&0x1ffc;
		full_size->height = full_size->height&(~1);
		printk("[ISP] full_size width = %d, height = %d, ob_valid = %d, %d\n",full_size->width,full_size->height, ob_valid_size->width, ob_valid_size->height);
		x_ratio = ob_valid_size->width*256/full_size->width;
		y_ratio = ob_valid_size->height*256/full_size->height;
		weight_shift = min_scale_w_shift(x_ratio,y_ratio);

		bsp_isp_channel_enable(SUB_CH);
		bsp_isp_scale_enable(SUB_CH);
		bsp_isp_set_output_size(SUB_CH, full_size);
		bsp_isp_scale_cfg(SUB_CH,x_ratio,y_ratio,weight_shift);

		//for exchange sub and man ch. NOTE: size and fmt.
		__isp_cal_ch_size(fmt[MAIN_CH], full_size, &isp_yuv_size_addr[SUB_CH]);

		bsp_isp_set_stride_y(isp_yuv_size_addr[SUB_CH].line_stride_y, SUB_CH);
		bsp_isp_set_stride_uv(isp_yuv_size_addr[SUB_CH].line_stride_c, SUB_CH);

		bsp_isp_channel_disable(MAIN_CH);
		isp_yuv_size_addr[MAIN_CH].isp_byte_size = 0;
	}

	if(ob_rot_size && 0 != ob_rot_size->height && 0 != ob_rot_size->width)
	{
		__isp_cal_ch_size(fmt[ROT_CH], ob_rot_size, &isp_yuv_size_addr[ROT_CH]);
		bsp_isp_set_stride_y(isp_yuv_size_addr[ROT_CH].line_stride_y, ROT_CH);
		bsp_isp_set_stride_uv(isp_yuv_size_addr[ROT_CH].line_stride_c, ROT_CH);
	} else {
		isp_yuv_size_addr[ROT_CH].isp_byte_size = 0;
	}

	isp_yuv_size_addr[MAIN_CH].isp_byte_size = ALIGN_4K(isp_yuv_size_addr[MAIN_CH].isp_byte_size);
	isp_yuv_size_addr[SUB_CH].isp_byte_size = ALIGN_4K(isp_yuv_size_addr[SUB_CH].isp_byte_size);
	isp_yuv_size_addr[ROT_CH].isp_byte_size = ALIGN_4K(isp_yuv_size_addr[ROT_CH].isp_byte_size);
	return (isp_yuv_size_addr[MAIN_CH].isp_byte_size + isp_yuv_size_addr[SUB_CH].isp_byte_size +
					isp_yuv_size_addr[ROT_CH].isp_byte_size);
}

unsigned int sunxi_isp_set_size(enum pixel_fmt *fmt, struct isp_size_settings *size_settings)
{
	return __isp_new_set_size_internal(fmt, size_settings);
}
void sunxi_isp_set_fmt(enum bus_pixeltype type, enum pixel_fmt *fmt)
{
	//printk("type = %d\n",type);
	__isp_set_input_fmt_internal(type);

	if(fmt[SUB_CH]!=PIX_FMT_NONE)
	{
		__isp_set_output_fmt_internal(fmt[MAIN_CH],MAIN_CH);

		if(fmt[MAIN_CH] == PIX_FMT_YVU420P_8 || fmt[MAIN_CH] == PIX_FMT_YVU422P_8)
		{
			 isp_gbl->plannar_uv_exchange_flag[MAIN_CH] = 1;
		}
		else
		{
			isp_gbl->plannar_uv_exchange_flag[MAIN_CH] = 0;
		}


		bsp_isp_module_enable(TG_EN);
		__isp_set_output_fmt_internal(fmt[SUB_CH],SUB_CH);

		if(fmt[SUB_CH] == PIX_FMT_YVU420P_8 || fmt[SUB_CH] == PIX_FMT_YVU422P_8)
		{
			isp_gbl->plannar_uv_exchange_flag[SUB_CH] = 1;
		}
		else
		{
			isp_gbl->plannar_uv_exchange_flag[SUB_CH] = 0;
		}
	} else {
		//for exchange sub and man ch.
		__isp_set_output_fmt_internal(fmt[MAIN_CH], SUB_CH);

		if(fmt[MAIN_CH] == PIX_FMT_YVU420P_8 || fmt[MAIN_CH] == PIX_FMT_YVU422P_8)
		{
			isp_gbl->plannar_uv_exchange_flag[SUB_CH] = 1;
		}
		else
		{
			isp_gbl->plannar_uv_exchange_flag[SUB_CH] = 0;
		}
		bsp_isp_module_disable(TG_EN);
	}

	if(fmt[ROT_CH]!=PIX_FMT_NONE) 
	{
		isp_gbl->rotation_en = 1;
		bsp_isp_module_enable(ROT_EN);
		__isp_set_output_fmt_internal(fmt[ROT_CH],ROT_CH);

		if(fmt[SUB_CH] == PIX_FMT_YVU420P_8 || fmt[SUB_CH] == PIX_FMT_YVU422P_8)
		{
			isp_gbl->plannar_uv_exchange_flag[ROT_CH] = 1;
		}
		else
		{
			isp_gbl->plannar_uv_exchange_flag[ROT_CH] = 0;
		}
	} else {
		bsp_isp_module_disable(ROT_EN);
	}
}

void sunxi_isp_set_flip(enum isp_channel ch, enum enable_flag on_off)
{
	enum enable_flag *flip_en_glb = &isp_gbl->flip_en_glb[0];
	bsp_isp_set_flip(ch,on_off);
	flip_en_glb[ch] = on_off;
}
void sunxi_isp_set_mirror(enum isp_channel ch, enum enable_flag on_off)
{
	bsp_isp_set_mirror(ch,on_off);
}

void sunxi_isp_set_output_addr(unsigned long buf_base_addr)
{
	int tmp_addr;
	struct isp_yuv_size_addr_info *isp_yuv_size_addr = &isp_gbl->isp_yuv_size_addr[0];
	__isp_cal_ch_addr(isp_gbl->flip_en_glb[MAIN_CH], buf_base_addr, &isp_yuv_size_addr[MAIN_CH]);
	if(isp_gbl->plannar_uv_exchange_flag[MAIN_CH] == 1)
	{
		tmp_addr = isp_yuv_size_addr[MAIN_CH].yuv_addr.u_addr;
		isp_yuv_size_addr[MAIN_CH].yuv_addr.u_addr = isp_yuv_size_addr[MAIN_CH].yuv_addr.v_addr;
		isp_yuv_size_addr[MAIN_CH].yuv_addr.v_addr = tmp_addr;
	}

	__isp_cal_ch_addr(isp_gbl->flip_en_glb[SUB_CH], ALIGN_4K(buf_base_addr + isp_yuv_size_addr[MAIN_CH].isp_byte_size), &isp_yuv_size_addr[SUB_CH]);
	if(isp_gbl->plannar_uv_exchange_flag[SUB_CH] == 1)
	{
		tmp_addr = isp_yuv_size_addr[SUB_CH].yuv_addr.u_addr;
		isp_yuv_size_addr[SUB_CH].yuv_addr.u_addr = isp_yuv_size_addr[SUB_CH].yuv_addr.v_addr;
		isp_yuv_size_addr[SUB_CH].yuv_addr.v_addr = tmp_addr;
	}

	bsp_isp_set_yuv_addr(&isp_yuv_size_addr[MAIN_CH].yuv_addr, MAIN_CH, ISP_SRC0);
	bsp_isp_set_yuv_addr(&isp_yuv_size_addr[SUB_CH].yuv_addr, SUB_CH, ISP_SRC0);

	//printk("%s, line: %d\n", __FUNCTION__, __LINE__);
	if(isp_gbl->rotation_en == 1)
	{
		//printk("%s, line: %d\n", __FUNCTION__, __LINE__);//flip_en_glb must be disable
		__isp_cal_ch_addr(DISABLE, ALIGN_4K(buf_base_addr + isp_yuv_size_addr[MAIN_CH].isp_byte_size +
		isp_yuv_size_addr[SUB_CH].isp_byte_size), &isp_yuv_size_addr[ROT_CH]);

		//printk("%s, line: %d\n", __FUNCTION__, __LINE__);
		if(isp_gbl->plannar_uv_exchange_flag[ROT_CH] == 1)
		{
			tmp_addr = isp_yuv_size_addr[ROT_CH].yuv_addr.u_addr;
			isp_yuv_size_addr[ROT_CH].yuv_addr.u_addr = isp_yuv_size_addr[ROT_CH].yuv_addr.v_addr;
			isp_yuv_size_addr[ROT_CH].yuv_addr.v_addr = tmp_addr;
		}
		//printk("%s, line: %d\n", __FUNCTION__, __LINE__);
		bsp_isp_set_yuv_addr(&isp_yuv_size_addr[ROT_CH].yuv_addr, ROT_CH, ISP_SRC0);
	}
}



static int sunxi_isp_subdev_s_power(struct v4l2_subdev *sd, int enable)
{
	return 0;
}
static int sunxi_isp_subdev_s_stream(struct v4l2_subdev *sd, int enable)
{
	return 0;
}

static int sunxi_isp_enum_mbus_code(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		      struct v4l2_subdev_mbus_code_enum *code)
{
	return 0;
}

static int sunxi_isp_subdev_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_format *fmt)
{
	return 0;
}

static int sunxi_isp_subdev_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
				   struct v4l2_subdev_format *fmt)
{
	return 0;
}

int sunxi_isp_addr_init(struct v4l2_subdev *sd, u32 val)
{
	bsp_isp_set_dma_load_addr((unsigned long)isp_gbl->isp_load_reg_mm.dma_addr);
	bsp_isp_set_dma_saved_addr((unsigned long)isp_gbl->isp_save_reg_mm.dma_addr);
	return 0;
}
static int sunxi_isp_subdev_cropcap(struct v4l2_subdev *sd,
				       struct v4l2_cropcap *a)
{
	return 0;
}

static const struct v4l2_subdev_core_ops sunxi_isp_core_ops = {
	.s_power = sunxi_isp_subdev_s_power,
	.init = sunxi_isp_addr_init,
	.queryctrl = v4l2_subdev_queryctrl,
	.g_ctrl = v4l2_subdev_g_ctrl,
	.s_ctrl = v4l2_subdev_s_ctrl,
};

static const struct v4l2_subdev_video_ops sunxi_isp_subdev_video_ops = {
	.s_stream = sunxi_isp_subdev_s_stream,
	.cropcap	= sunxi_isp_subdev_cropcap,
};

static const struct v4l2_subdev_pad_ops sunxi_isp_subdev_pad_ops = {
	.enum_mbus_code = sunxi_isp_enum_mbus_code,
	.get_fmt = sunxi_isp_subdev_get_fmt,
	.set_fmt = sunxi_isp_subdev_set_fmt,
};


static struct v4l2_subdev_ops sunxi_isp_subdev_ops = {
	.core = &sunxi_isp_core_ops,
	.video = &sunxi_isp_subdev_video_ops,
	.pad = &sunxi_isp_subdev_pad_ops,
};
/*
static int sunxi_isp_subdev_registered(struct v4l2_subdev *sd)
{
	struct vfe_dev *vfe = v4l2_get_subdevdata(sd);
	int ret = 0;
	return ret;
}

static void sunxi_isp_subdev_unregistered(struct v4l2_subdev *sd)
{
	struct vfe_dev *vfe = v4l2_get_subdevdata(sd);
	return;
}

static const struct v4l2_subdev_internal_ops sunxi_isp_sd_internal_ops = {
	.registered = sunxi_isp_subdev_registered,
	.unregistered = sunxi_isp_subdev_unregistered,
};
*/

static int __sunxi_isp_ctrl(struct isp_dev *isp, struct v4l2_ctrl *ctrl)
{
	int ret = 0;

	if (ctrl->flags & V4L2_CTRL_FLAG_INACTIVE)
		return 0;

	switch (ctrl->id) {
	case V4L2_CID_HFLIP:
		if(ctrl->val== 0)
			sunxi_isp_set_mirror(MAIN_CH, DISABLE);
		else
			sunxi_isp_set_mirror(MAIN_CH, ENABLE);
		isp->hflip = ctrl->val;
		break;
	case V4L2_CID_VFLIP:
		if(ctrl->val== 0)
			sunxi_isp_set_flip(MAIN_CH, DISABLE);
		else
			sunxi_isp_set_flip(MAIN_CH, ENABLE);
		isp->vflip = ctrl->val;
		break;
	case V4L2_CID_HFLIP_THUMB:
			if(ctrl->val == 0)
				sunxi_isp_set_mirror(SUB_CH, DISABLE);
			else
				sunxi_isp_set_mirror(SUB_CH, ENABLE);
		break;
	case V4L2_CID_VFLIP_THUMB:
			if(ctrl->val == 0)
				sunxi_isp_set_flip(SUB_CH, DISABLE);
			else
				sunxi_isp_set_flip(SUB_CH, ENABLE);
		break;
	case V4L2_CID_ROTATE:
		isp->rotate= ctrl->val;
		break;
	}
	return ret;
}

#define ctrl_to_sunxi_isp(ctrl) \
	container_of(ctrl->handler, struct isp_dev, ctrls.handler)

static int sunxi_isp_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct isp_dev *isp = ctrl_to_sunxi_isp(ctrl);
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&isp->slock, flags);
	ret = __sunxi_isp_ctrl(isp, ctrl);
	spin_unlock_irqrestore(&isp->slock, flags);

	return ret;
}

static const struct v4l2_ctrl_ops sunxi_isp_ctrl_ops = {
	.s_ctrl	= sunxi_isp_s_ctrl,
};

static const struct v4l2_ctrl_config isp_ctrls[] = 
{
	{
		.ops = &sunxi_isp_ctrl_ops,
		.id = V4L2_CID_HFLIP_THUMB,
		.name = "Horizontal Flip For Thumb",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &sunxi_isp_ctrl_ops,
		.id = V4L2_CID_VFLIP_THUMB,
		.name = "Vertical Flip For Thumb",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
};

int sunxi_isp_init_subdev(struct isp_dev *isp)
{
	const struct v4l2_ctrl_ops *ops = &sunxi_isp_ctrl_ops;
	struct v4l2_ctrl_handler *handler = &isp->ctrls.handler;
	struct v4l2_subdev *sd = &isp->subdev;
	struct sunxi_isp_ctrls *ctrls = &isp->ctrls;
	enum enable_flag *flip_en_glb = &isp->flip_en_glb[0];
	int i;
	mutex_init(&isp->subdev_lock);

	v4l2_subdev_init(sd, &sunxi_isp_subdev_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	snprintf(sd->name, sizeof(sd->name), "sunxi_isp.%u", isp->isp_sel);

	v4l2_ctrl_handler_init(handler, 3 + ARRAY_SIZE(isp_ctrls));

	ctrls->rotate = v4l2_ctrl_new_std(handler, ops, V4L2_CID_ROTATE, 0, 270, 90, 0);
	ctrls->hflip = v4l2_ctrl_new_std(handler,ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
	ctrls->vflip = v4l2_ctrl_new_std(handler, ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
	
	for (i = 0; i < ARRAY_SIZE(isp_ctrls); i ++)
		v4l2_ctrl_new_custom(handler, &isp_ctrls[i], NULL);

	if (handler->error) {
		return handler->error;
	}

	sd->ctrl_handler = handler;
	//sd->internal_ops = &sunxi_isp_sd_internal_ops;
	v4l2_set_subdevdata(sd, isp);

	flip_en_glb[MAIN_CH] = 0;
	flip_en_glb[SUB_CH] = 0;
	return 0;
}
    

static void isp_release(struct device *dev)
{
	vfe_print("isp_device_release\n");
};

static int isp_resource_alloc(struct isp_dev *isp)
{
	int ret = 0; 
	isp->isp_load_reg_mm.size = ISP_LOAD_REG_SIZE;
	isp->isp_save_reg_mm.size = ISP_SAVED_REG_SIZE;

	os_mem_alloc(&isp->isp_load_reg_mm);
	os_mem_alloc(&isp->isp_save_reg_mm);

	if(isp->isp_load_reg_mm.phy_addr != NULL) {
		if(!isp->isp_load_reg_mm.vir_addr)
		{
			vfe_err("isp load regs va requset failed!\n");
			return -ENOMEM;
		}
	} else {
		vfe_err("isp load regs pa requset failed!\n");
		return -ENOMEM;
	}
  
	if(isp->isp_save_reg_mm.phy_addr != NULL) {
		if(!isp->isp_save_reg_mm.vir_addr)
		{
			vfe_err("isp save regs va requset failed!\n");
			return -ENOMEM;
		}
	} else {
		vfe_err("isp save regs pa requset failed!\n");
		return -ENOMEM;
	}
	return ret;

}
static void isp_resource_release(struct isp_dev *isp)
{
	os_mem_free(&isp->isp_load_reg_mm);
	os_mem_free(&isp->isp_save_reg_mm);
}

static int isp_probe(struct platform_device *pdev)
{
	struct isp_dev *isp = NULL;
	struct resource *res = NULL;
	struct isp_platform_data *pdata = NULL;
	int ret = 0;
	if(pdev->dev.platform_data == NULL)
	{
		return -ENODEV;
	}
	pdata = pdev->dev.platform_data;
	vfe_print("isp probe start isp_sel = %d!\n",pdata->isp_sel);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		return -ENODEV;
	}
	isp = kzalloc(sizeof(struct isp_dev), GFP_KERNEL);
	if (!isp) {
		ret = -ENOMEM;
		goto ekzalloc;
	}
	isp_gbl = isp;
	isp->use_cnt = 0;
	isp->rotation_en = 0;
	sunxi_isp_init_subdev(isp);
	isp->ioarea = request_mem_region(res->start, resource_size(res), res->name);
	if (!isp->ioarea) {
		goto freedev;
	}
	isp->isp_sel = pdata->isp_sel;
	
	spin_lock_init(&isp->slock);
	init_waitqueue_head(&isp->wait);
	
	isp->base = ioremap(res->start, resource_size(res));
	if (!isp->base) {
		ret = -EIO;
		goto out_map;
	}
	if(isp_resource_alloc(isp) < 0)
	{
		ret = -EIO;
		goto ereqirq;
	}
	bsp_isp_init_platform(SUNXI_PLATFORM_ID);
	bsp_isp_set_base_addr((unsigned long)isp->base);
	bsp_isp_set_map_load_addr((unsigned long)isp->isp_load_reg_mm.vir_addr);
	bsp_isp_set_map_saved_addr((unsigned long)isp->isp_save_reg_mm.vir_addr);
	memset((unsigned int*)isp->isp_load_reg_mm.vir_addr,0,ISP_LOAD_REG_SIZE);
	memset((unsigned int*)isp->isp_save_reg_mm.vir_addr,0,ISP_SAVED_REG_SIZE);
	platform_set_drvdata(pdev, isp);
	vfe_print("isp probe end isp_sel = %d!\n",pdata->isp_sel);
	return 0;

ereqirq:
	iounmap(isp->base);
out_map:
	release_resource(isp->ioarea);
	kfree(isp->ioarea);
freedev:
	kfree(isp);
ekzalloc:
	vfe_print("isp probe err!\n");
	return ret;
}


static int isp_remove(struct platform_device *pdev)
{
	struct isp_dev *isp = platform_get_drvdata(pdev);
	platform_set_drvdata(pdev, NULL);
	isp_resource_release(isp) ;
	if(isp->ioarea) {
        release_resource(isp->ioarea);
        kfree(isp->ioarea);
	}
	if(isp->base)
		iounmap(isp->base);
	kfree(isp);
	return 0;
}

static struct resource isp0_resource[] = 
{
	[0] = {
		.name  = "isp",
		.start  = ISP_REGS_BASE,
		.end    = ISP_REGS_BASE + ISP_REG_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct isp_platform_data isp0_pdata[] = {
	{
		.isp_sel  = 0,
	},
};

static struct platform_device isp_device[] = {
	[0] = {
		.name  = ISP_MODULE_NAME,
		.id = 0,
		.num_resources = ARRAY_SIZE(isp0_resource),
		.resource = isp0_resource,
		.dev = {
			.platform_data  = isp0_pdata,
			.release        = isp_release,
		},
	},
};

static struct platform_driver isp_platform_driver = {
	.probe    = isp_probe,
	.remove   = isp_remove,
	.driver = {
		.name   = ISP_MODULE_NAME,
		.owner  = THIS_MODULE,
	}
};

int sunxi_isp_register_subdev(struct v4l2_device *v4l2_dev, struct v4l2_subdev *sd)
{
	return v4l2_device_register_subdev(v4l2_dev, sd);
}

void sunxi_isp_unregister_subdev(struct v4l2_subdev *sd)
{
	v4l2_device_unregister_subdev(sd);
	v4l2_ctrl_handler_free(sd->ctrl_handler);
	v4l2_set_subdevdata(sd, NULL);
}

int sunxi_isp_get_subdev(struct v4l2_subdev **sd, int sel)
{
	*sd = &isp_gbl->subdev;
	return (isp_gbl->use_cnt++);
}
int sunxi_isp_put_subdev(struct v4l2_subdev **sd, int sel)
{
	*sd = NULL;
	return (isp_gbl->use_cnt--);
}

int sunxi_isp_platform_register(void)
{
	int ret,i;
	for(i=0; i<ARRAY_SIZE(isp_device); i++) 
	{
		ret = platform_device_register(&isp_device[i]);
		if (ret)
			vfe_err("isp device %d register failed\n",i);
	}
	ret = platform_driver_register(&isp_platform_driver);
	if (ret) {
		vfe_err("platform driver register failed\n");
		return ret;
	}
	vfe_print("sunxi_isp_platform_register end\n");
	return 0;
}

void  sunxi_isp_platform_unregister(void)
{
	int i;
	vfe_print("sunxi_isp_platform_unregister start\n");
	for(i=0; i<ARRAY_SIZE(isp_device); i++)
	{
		platform_device_unregister(&isp_device[i]);
	}
	platform_driver_unregister(&isp_platform_driver);
	vfe_print("sunxi_isp_platform_unregister end\n");
}



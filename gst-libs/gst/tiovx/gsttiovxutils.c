/*
 * Copyright (c) [2021] Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter
 * owns or controls to make, have made, use, import, offer to sell and sell
 * ("Utilize") this software subject to the terms herein.  With respect to
 * the foregoing patent license, such license is granted  solely to the extent
 * that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include
 * this software, other than combinations with devices manufactured by or
 * for TI (“TI Devices”).  No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce
 * this license (including the above copyright notice and the disclaimer
 * and (if applicable) source code license limitations below) in the
 * documentation and/or other materials provided with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted
 * provided that the following conditions are met:
 *
 * *	No reverse engineering, decompilation, or disassembly of this software
 *      is permitted with respect to any software provided in binary form.
 *
 * *	Any redistribution and use are licensed by TI for use only with TI
 * Devices.
 *
 * *	Nothing shall obligate TI to provide you with source code for the
 *      software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution
 * of the source code are permitted provided that the following conditions are
 * met:
 *
 * *	Any redistribution and use of the source code, including any resulting
 *      derivative works, are licensed by TI for use only with TI Devices.
 *
 * *	Any redistribution and use of any object code compiled from the source
 *      code and any resulting derivative works, are licensed by TI for use
 *      only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its
 * suppliers may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI’S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI’S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gsttiovxutils.h"

/* Convert VX Image Format to GST Image Format */
GstVideoFormat
vx_format_to_gst_format (const vx_df_image format)
{
  GstVideoFormat gst_format = GST_VIDEO_FORMAT_UNKNOWN;

  switch (format) {
    case VX_DF_IMAGE_RGB:
      gst_format = GST_VIDEO_FORMAT_RGB;
      break;
    case VX_DF_IMAGE_RGBX:
      gst_format = GST_VIDEO_FORMAT_RGBx;
      break;
    case VX_DF_IMAGE_NV12:
      gst_format = GST_VIDEO_FORMAT_NV12;
      break;
    case VX_DF_IMAGE_NV21:
      gst_format = GST_VIDEO_FORMAT_NV21;
      break;
    case VX_DF_IMAGE_UYVY:
      gst_format = GST_VIDEO_FORMAT_UYVY;
      break;
    case VX_DF_IMAGE_YUYV:
      gst_format = GST_VIDEO_FORMAT_YUY2;
      break;
    case VX_DF_IMAGE_IYUV:
      gst_format = GST_VIDEO_FORMAT_I420;
      break;
    case VX_DF_IMAGE_YUV4:
      gst_format = GST_VIDEO_FORMAT_Y444;
      break;
    default:
      gst_format = -1;
      break;
  }

  return gst_format;
}

/* Convert GST Image Format to VX Image Format */
vx_df_image
gst_format_to_vx_format (const GstVideoFormat gst_format)
{
  vx_df_image vx_format = VX_DF_IMAGE_VIRT;

  switch (gst_format) {
    case GST_VIDEO_FORMAT_RGB:
      vx_format = VX_DF_IMAGE_RGB;
      break;
    case GST_VIDEO_FORMAT_RGBx:
      vx_format = VX_DF_IMAGE_RGBX;
      break;
    case GST_VIDEO_FORMAT_NV12:
      vx_format = VX_DF_IMAGE_NV12;
      break;
    case GST_VIDEO_FORMAT_NV21:
      vx_format = VX_DF_IMAGE_NV21;
      break;
    case GST_VIDEO_FORMAT_UYVY:
      vx_format = VX_DF_IMAGE_UYVY;
      break;
    case GST_VIDEO_FORMAT_YUY2:
      vx_format = VX_DF_IMAGE_YUYV;
      break;
    case GST_VIDEO_FORMAT_I420:
      vx_format = VX_DF_IMAGE_IYUV;
      break;
    case GST_VIDEO_FORMAT_Y444:
      vx_format = VX_DF_IMAGE_YUV4;
      break;
    default:
      vx_format = -1;
      break;
  }

  return vx_format;
}

/* Convert TIOVX target ID to target name
   Based on: */
const char *
target_id_to_target_name (gint target_id)
{
  const char *target_name = NULL;

  switch (target_id) {
    case TIVX_CPU_ID_DSP1:
      target_name = TIVX_TARGET_DSP1;
      break;
    case TIVX_CPU_ID_DSP2:
      target_name = TIVX_TARGET_DSP2;
      break;
    default:
      target_name = NULL;
      break;
  }

  return target_name;
}

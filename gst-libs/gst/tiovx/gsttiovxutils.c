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

#include "gsttiovxbufferpoolutils.h"
#include "gsttiovxbufferutils.h"
#include "gsttiovxutils.h"

#include <TI/tivx_ext_raw_image.h>

#include "gsttiovx.h"
#include "gsttiovxallocator.h"
#include "gsttiovxbufferpool.h"
#include "gsttiovximagebufferpool.h"
#include "gsttiovxmeta.h"
#include "gsttiovxrawimagebufferpool.h"
#include "gsttiovxrawimagemeta.h"
#include "gsttiovxtensorbufferpool.h"
#include "gsttiovxtensormeta.h"

GST_DEBUG_CATEGORY (gst_tiovx_performance);

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
    case VX_DF_IMAGE_U8:
      gst_format = GST_VIDEO_FORMAT_GRAY8;
      break;
    case VX_DF_IMAGE_U16:
      gst_format = GST_VIDEO_FORMAT_GRAY16_LE;
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
    case GST_VIDEO_FORMAT_GRAY8:
      vx_format = VX_DF_IMAGE_U8;
      break;
    case GST_VIDEO_FORMAT_GRAY16_LE:
      vx_format = VX_DF_IMAGE_U16;
      break;
    default:
      vx_format = -1;
      break;
  }

  return vx_format;
}

/* Transfers handles between vx_references */
vx_status
gst_tiovx_transfer_handle (GstDebugCategory * category, vx_reference src,
    vx_reference dest)
{
  vx_status status = VX_SUCCESS;
  uint32_t num_entries = 0;
  vx_size src_num_addr = 0;
  vx_size dest_num_addr = 0;
  void *addr[MODULE_MAX_NUM_ADDRS] = { NULL };
  uint32_t bufsize[MODULE_MAX_NUM_ADDRS] = { 0 };
  vx_enum src_type = VX_TYPE_INVALID;
  vx_enum dest_type = VX_TYPE_INVALID;

  g_return_val_if_fail (category, VX_FAILURE);
  g_return_val_if_fail (VX_SUCCESS ==
      vxGetStatus ((vx_reference) src), VX_FAILURE);
  g_return_val_if_fail (VX_SUCCESS ==
      vxGetStatus ((vx_reference) dest), VX_FAILURE);

  src_type = gst_tiovx_get_exemplar_type (&src);
  dest_type = gst_tiovx_get_exemplar_type (&dest);

  g_return_val_if_fail (src_type == dest_type, VX_FAILURE);

  if (VX_TYPE_IMAGE == src_type) {
    status =
        vxQueryImage ((vx_image) dest, VX_IMAGE_PLANES, &dest_num_addr,
        sizeof (dest_num_addr));
    if (VX_SUCCESS != status) {
      GST_CAT_ERROR (category,
          "Get number of planes in dest image failed %" G_GINT32_FORMAT,
          status);
      return status;
    }

    status =
        vxQueryImage ((vx_image) src, VX_IMAGE_PLANES, &src_num_addr,
        sizeof (src_num_addr));
    if (VX_SUCCESS != status) {
      GST_CAT_ERROR (category,
          "Get number of planes in src image failed %" G_GINT32_FORMAT, status);
      return status;
    }
  } else if (VX_TYPE_TENSOR == src_type) {
    /* Tensors have 1 single memory block */
    dest_num_addr = MODULE_MAX_NUM_TENSORS;
    src_num_addr = MODULE_MAX_NUM_TENSORS;
  } else if (TIVX_TYPE_RAW_IMAGE == src_type) {
    vx_uint32 dest_num_exp = 0, src_num_exp = 0;

    status =
        tivxQueryRawImage ((tivx_raw_image) dest, TIVX_RAW_IMAGE_NUM_EXPOSURES,
        &dest_num_exp, sizeof (dest_num_exp));
    if (VX_SUCCESS != status) {
      GST_CAT_ERROR (category,
          "Get number of exposures in dest image failed %" G_GINT32_FORMAT,
          status);
      return status;
    }
    dest_num_addr = dest_num_exp;

    status =
        tivxQueryRawImage ((tivx_raw_image) src, TIVX_RAW_IMAGE_NUM_EXPOSURES,
        &src_num_exp, sizeof (src_num_exp));
    if (VX_SUCCESS != status) {
      GST_CAT_ERROR (category,
          "Get number of exposures in src image failed %" G_GINT32_FORMAT,
          status);
      return status;
    }
    src_num_addr = src_num_exp;

  } else {
    GST_CAT_ERROR (category, "Type %d not supported", src_type);
    return VX_FAILURE;
  }

  if (src_num_addr != dest_num_addr) {
    GST_CAT_ERROR (category,
        "Incompatible number of planes in src and dest images. src: %ld and dest: %ld",
        src_num_addr, dest_num_addr);
    return VX_FAILURE;
  }

  status =
      tivxReferenceExportHandle (src, addr, bufsize, src_num_addr,
      &num_entries);
  if (VX_SUCCESS != status) {
    GST_CAT_ERROR (category, "Export handle failed %" G_GINT32_FORMAT, status);
    return status;
  }

  GST_CAT_LOG (category, "Number of planes to transfer: %ld", src_num_addr);

  if (src_num_addr != num_entries) {
    GST_CAT_ERROR (category,
        "Incompatible number of planes and handles entries. planes: %ld and entries: %d",
        src_num_addr, num_entries);
    return VX_FAILURE;
  }

  status =
      tivxReferenceImportHandle (dest, (const void **) addr, bufsize,
      dest_num_addr);
  if (VX_SUCCESS != status) {
    GST_CAT_ERROR (category, "Import handle failed %" G_GINT32_FORMAT, status);
    return status;
  }

  return status;
}

/* Gets exemplar type */
vx_enum
gst_tiovx_get_exemplar_type (vx_reference * exemplar)
{
  vx_enum type = VX_TYPE_INVALID;
  vx_status status = VX_FAILURE;

  g_return_val_if_fail (exemplar, -1);
  g_return_val_if_fail (VX_SUCCESS == vxGetStatus (*exemplar), -1);

  status =
      vxQueryReference ((vx_reference) * exemplar, (vx_enum) VX_REFERENCE_TYPE,
      &type, sizeof (vx_enum));
  if (VX_SUCCESS != status) {
    return VX_TYPE_INVALID;
  }

  return type;
}

/* Get bit depth from a tensor data type */
vx_uint32
gst_tiovx_tensor_get_tensor_bit_depth (vx_enum data_type)
{
  vx_uint32 bit_depth = 0;

  switch (data_type) {
    case VX_TYPE_UINT8:
      bit_depth = sizeof (vx_uint8);
      break;
    case VX_TYPE_INT8:
      bit_depth = sizeof (vx_int8);
      break;
    case VX_TYPE_UINT16:
      bit_depth = sizeof (vx_uint16);
      break;
    case VX_TYPE_INT16:
      bit_depth = sizeof (vx_int16);
      break;
    case VX_TYPE_UINT32:
      bit_depth = sizeof (vx_uint32);
      break;
    case VX_TYPE_INT32:
      bit_depth = sizeof (vx_int32);
      break;
    case VX_TYPE_FLOAT32:
      bit_depth = sizeof (vx_float32);
      break;
    default:
      bit_depth = -1;
      break;
  }

  return bit_depth;
}

/* Get bits per pixel from bayer video */
guint
gst_tiovx_bayer_get_bits_per_pixel (const gchar * bayer_format)
{
  guint bpp = 0;

  if (0 == g_strcmp0 ("bggr", bayer_format) ||
      0 == g_strcmp0 ("gbrg", bayer_format) ||
      0 == g_strcmp0 ("grbg", bayer_format)
      || 0 == g_strcmp0 ("rggb", bayer_format)) {
    bpp = 1;
  } else if (0 == g_strcmp0 ("bggr16", bayer_format) ||
      0 == g_strcmp0 ("gbrg16", bayer_format) ||
      0 == g_strcmp0 ("grbg16", bayer_format) ||
      0 == g_strcmp0 ("rggb16", bayer_format)) {
    bpp = 2;
  } else {
    bpp = -1;
  }

  return bpp;
}

/**
 * This function clears the memory pointers from the exemplars.
 * The actual memory will be cleared by the allocator, this avoid a double
 * free of the memory
 * */
vx_status
gst_tiovx_empty_exemplar (vx_reference ref)
{
  vx_status status = VX_SUCCESS;
  void *addr[MODULE_MAX_NUM_ADDRS] = { NULL };
  vx_uint32 sizes[MODULE_MAX_NUM_ADDRS];
  uint32_t num_addrs;

  g_return_val_if_fail (VX_SUCCESS == vxGetStatus (ref), VX_FAILURE);

  tivxReferenceExportHandle (ref,
      addr, sizes, MODULE_MAX_NUM_ADDRS, &num_addrs);

  for (int i = 0; i < num_addrs; i++) {
    addr[i] = NULL;
  }

  status = tivxReferenceImportHandle (ref,
      (const void **) addr, (const uint32_t *) sizes, num_addrs);

  return status;
}

/* Gets size from exemplar */
gsize
gst_tiovx_get_size_from_exemplar (vx_reference exemplar)
{
  gsize size = 0;
  vx_enum type = VX_TYPE_INVALID;

  g_return_val_if_fail (VX_SUCCESS == vxGetStatus (exemplar), 0);

  type = gst_tiovx_get_exemplar_type (&exemplar);

  if (VX_TYPE_IMAGE == type) {
    vx_size img_size = 0;

    vxQueryImage ((vx_image) exemplar, VX_IMAGE_SIZE, &img_size,
        sizeof (img_size));

    size = img_size;
  } else if (VX_TYPE_TENSOR == type) {
    void *dim_addr[MODULE_MAX_NUM_TENSORS] = { NULL };
    vx_uint32 dim_sizes[MODULE_MAX_NUM_TENSORS] = { 0 };
    vx_uint32 num_dims = 0;

    /* Check memory size */
    tivxReferenceExportHandle ((vx_reference) exemplar,
        dim_addr, dim_sizes, MODULE_MAX_NUM_TENSORS, &num_dims);

    /* TI indicated tensors have 1 single block of memory */
    size = dim_sizes[0];
  } else if (TIVX_TYPE_RAW_IMAGE == type) {
    void *exposure_addr[MODULE_MAX_NUM_EXPOSURES] = { NULL };
    vx_uint32 exposure_sizes[MODULE_MAX_NUM_EXPOSURES];
    guint num_exposures = 0;
    vx_size img_size = 0;
    guint exposure_idx = 0;

    tivxReferenceExportHandle ((vx_reference) exemplar,
        exposure_addr, exposure_sizes, MODULE_MAX_NUM_EXPOSURES,
        &num_exposures);

    for (exposure_idx = 0; exposure_idx < num_exposures; exposure_idx++) {
      img_size += exposure_sizes[exposure_idx];
    }

    size = img_size;
  }

  return size;
}

/* Initializes debug categories */
void
gst_tiovx_init_debug (void)
{
  gst_tiovx_init_buffer_utils_debug ();
}

const gchar *
tivx_raw_format_to_gst_format (const enum tivx_raw_image_pixel_container_e
    format)
{
  const gchar *gst_format = NULL;

  /* TODO Add support to distinguish between different bayer formats  */
  switch (format) {
    case TIVX_RAW_IMAGE_16_BIT:
      gst_format = "bggr16";
      break;
    case TIVX_RAW_IMAGE_8_BIT:
      gst_format = "bggr";
      break;
    default:
      break;
  }

  return gst_format;
}

enum tivx_raw_image_pixel_container_e
gst_format_to_tivx_raw_format (const gchar * gst_format)
{
  enum tivx_raw_image_pixel_container_e tivx_format = -1;

  /* TODO Add support to distinguish between different bayer formats  */
  if (g_str_equal (gst_format, "bggr") ||
      g_str_equal (gst_format, "gbrg") ||
      g_str_equal (gst_format, "grbg") || g_str_equal (gst_format, "rggb")) {
    tivx_format = TIVX_RAW_IMAGE_8_BIT;
  } else if (g_str_equal (gst_format, "bggr16") ||
      g_str_equal (gst_format, "gbrg16") || g_str_equal (gst_format, "grbg16")
      || g_str_equal (gst_format, "rggb16")) {
    tivx_format = TIVX_RAW_IMAGE_16_BIT;
  }

  return tivx_format;
}

vx_status
add_graph_parameter_by_node_index (GstDebugCategory * debug_category,
    GObject * gobj, vx_graph graph, vx_node node, vx_uint32 parameter_index,
    vx_uint32 node_parameter_index,
    vx_graph_parameter_queue_params_t * parameters_list,
    vx_reference * refs_list, guint refs_list_size)
{
  vx_parameter parameter = NULL;
  vx_status status = VX_FAILURE;

  g_return_val_if_fail (parameter_index >= 0, VX_FAILURE);
  g_return_val_if_fail (refs_list_size >= MIN_NUM_CHANNELS, VX_FAILURE);
  g_return_val_if_fail (parameters_list, VX_FAILURE);
  g_return_val_if_fail (refs_list, VX_FAILURE);
  g_return_val_if_fail (VX_SUCCESS ==
      vxGetStatus ((vx_reference) graph), VX_FAILURE);
  g_return_val_if_fail (VX_SUCCESS ==
      vxGetStatus ((vx_reference) node), VX_FAILURE);

  parameter = vxGetParameterByIndex (node, node_parameter_index);
  status = vxAddParameterToGraph (graph, parameter);
  if (VX_SUCCESS != status) {
    GST_CAT_ERROR_OBJECT (debug_category, gobj,
        "Add parameter to graph failed  %" G_GINT32_FORMAT, status);
    goto exit;
  }

  parameters_list[parameter_index].graph_parameter_index = parameter_index;
  parameters_list[parameter_index].refs_list_size = refs_list_size;
  parameters_list[parameter_index].refs_list = refs_list;

exit:
  if (VX_SUCCESS != vxReleaseParameter (&parameter)) {
    GST_CAT_ERROR_OBJECT (debug_category, gobj, "Release parameter failed");
  }

  return status;
}

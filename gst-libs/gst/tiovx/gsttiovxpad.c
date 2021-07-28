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

#include "gsttiovxpad.h"

#include "gsttiovxbufferpool.h"
#include "gsttiovxmeta.h"
#include "gsttiovxutils.h"

static const gsize kcopy_all_size = -1;

#define MIN_BUFFER_POOL_SIZE 2
#define MAX_BUFFER_POOL_SIZE 16
#define DEFAULT_BUFFER_POOL_SIZE MIN_BUFFER_POOL_SIZE

enum
{
  PROP_BUFFER_POOL_SIZE = 1,
};

/**
 * SECTION:gsttiovxpad
 * @short_description: GStreamer pad for GstTIOVX based elements
 *
 * This class implements a GStreamer standard buffer pool for GstTIOVX
 * based elements.
 */

GST_DEBUG_CATEGORY_STATIC (gst_tiovx_pad_debug_category);
#define GST_CAT_DEFAULT gst_tiovx_pad_debug_category

struct _GstTIOVXPad
{
  GstPad base;

  GstTIOVXBufferPool *buffer_pool;

  vx_reference exemplar;
  guint pool_size;
};

G_DEFINE_TYPE_WITH_CODE (GstTIOVXPad, gst_tiovx_pad,
    GST_TYPE_PAD,
    GST_DEBUG_CATEGORY_INIT (gst_tiovx_pad_debug_category,
        "tiovxpad", 0, "debug category for TIOVX pad class"));

/* prototypes */
static void gst_tiovx_pad_finalize (GObject * object);
static gboolean gst_tiovx_pad_configure_pool (GstTIOVXPad * pad, GstCaps * caps,
    GstVideoInfo * info);
static void
gst_tiovx_pad_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void
gst_tiovx_pad_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void
gst_tiovx_pad_class_init (GstTIOVXPadClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gst_tiovx_pad_set_property;
  object_class->get_property = gst_tiovx_pad_get_property;

  g_object_class_install_property (object_class, PROP_BUFFER_POOL_SIZE,
      g_param_spec_uint ("pool-size", "Buffer pool size",
          "Size of the buffer pool",
          MIN_BUFFER_POOL_SIZE, MAX_BUFFER_POOL_SIZE, DEFAULT_BUFFER_POOL_SIZE,
          G_PARAM_READWRITE));

  object_class->finalize = gst_tiovx_pad_finalize;
}

static void
gst_tiovx_pad_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTIOVXPad *self = GST_TIOVX_PAD (object);

  GST_LOG_OBJECT (self, "set_property");

  GST_OBJECT_LOCK (self);
  switch (prop_id) {
    case PROP_BUFFER_POOL_SIZE:
      self->pool_size = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (self);
}

static void
gst_tiovx_pad_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstTIOVXPad *self = GST_TIOVX_PAD (object);

  GST_LOG_OBJECT (self, "get_property");

  GST_OBJECT_LOCK (self);
  switch (prop_id) {
    case PROP_BUFFER_POOL_SIZE:
      g_value_set_uint (value, self->pool_size);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (self);
}


static void
gst_tiovx_pad_init (GstTIOVXPad * this)
{
  this->buffer_pool = NULL;
  this->exemplar = NULL;
  this->pool_size = DEFAULT_BUFFER_POOL_SIZE;
}

void
gst_tiovx_pad_set_exemplar (GstTIOVXPad * pad, const vx_reference exemplar)
{
  GstTIOVXPad *tiovx_pad = GST_TIOVX_PAD (pad);

  g_return_if_fail (pad);
  g_return_if_fail (exemplar);

  tiovx_pad->exemplar = exemplar;
}

gboolean
gst_tiovx_pad_peer_query_allocation (GstTIOVXPad * tiovx_pad, GstCaps * caps)
{
  GstQuery *query = NULL;
  gint npool = 0;
  gboolean ret = FALSE;
  GstPad *peer = NULL;

  g_return_val_if_fail (tiovx_pad, FALSE);
  g_return_val_if_fail (caps, FALSE);

  query = gst_query_new_allocation (caps, TRUE);

  peer = gst_pad_get_peer (GST_PAD (tiovx_pad));
  ret = gst_pad_query (peer, query);

  if (!ret) {
    GST_INFO_OBJECT (tiovx_pad, "Unable to query pad peer");
  }
  gst_object_unref (peer);

  /* Always remove the current pool, we will either create a new one or get it from downstream */
  if (NULL != tiovx_pad->buffer_pool) {
    gst_object_unref (tiovx_pad->buffer_pool);
    tiovx_pad->buffer_pool = NULL;
  }

  /* Look for the first TIOVX buffer if present */
  for (npool = 0; npool < gst_query_get_n_allocation_pools (query); ++npool) {
    GstBufferPool *pool;

    gst_query_parse_nth_allocation_pool (query, npool, &pool, NULL, NULL, NULL);

    if (GST_TIOVX_IS_BUFFER_POOL (pool)) {
      tiovx_pad->buffer_pool = GST_TIOVX_BUFFER_POOL (pool);
      gst_buffer_pool_set_active (GST_BUFFER_POOL (pool), TRUE);
      break;
    } else {
      gst_object_unref (pool);
    }
  }

  if (NULL == tiovx_pad->buffer_pool) {
    GstVideoInfo info;

    tiovx_pad->buffer_pool = g_object_new (GST_TIOVX_TYPE_BUFFER_POOL, NULL);

    ret = gst_tiovx_pad_configure_pool (tiovx_pad, caps, &info);

    if (!ret) {
      GST_ERROR_OBJECT (tiovx_pad, "Unable to configure pool");
      gst_object_unref (tiovx_pad->buffer_pool);
      tiovx_pad->buffer_pool = NULL;
      goto unref_query;
    }
  }

  ret = TRUE;

unref_query:
  gst_query_unref (query);

  return ret;
}

static gboolean
gst_tiovx_pad_process_allocation_query (GstTIOVXPad * pad, GstQuery * query)
{
  GstTIOVXPad *tiovx_pad = GST_TIOVX_PAD (pad);
  GstCaps *caps = NULL;
  GstVideoInfo info;
  gboolean ret = FALSE;

  g_return_val_if_fail (pad, FALSE);
  g_return_val_if_fail (query, FALSE);

  if (NULL == tiovx_pad->exemplar) {
    GST_ERROR_OBJECT (pad,
        "Cannot process allocation query without an exemplar");
    goto out;
  }

  if (NULL != tiovx_pad->buffer_pool) {
    GST_DEBUG_OBJECT (pad, "Freeing current pool");
    gst_object_unref (tiovx_pad->buffer_pool);
    tiovx_pad->buffer_pool = NULL;
  }

  gst_query_parse_allocation (query, &caps, NULL);
  if (!caps) {
    GST_ERROR_OBJECT (pad, "Unable to parse caps from query");
    ret = FALSE;
    goto out;
  }

  tiovx_pad->buffer_pool = g_object_new (GST_TIOVX_TYPE_BUFFER_POOL, NULL);

  ret = gst_tiovx_pad_configure_pool (tiovx_pad, caps, &info);
  if (!ret) {
    GST_ERROR_OBJECT (pad, "Unable to configure pool");
    gst_object_unref (tiovx_pad->buffer_pool);
    tiovx_pad->buffer_pool = NULL;
    goto out;
  }

  gst_query_add_allocation_pool (query,
      GST_BUFFER_POOL (tiovx_pad->buffer_pool), GST_VIDEO_INFO_SIZE (&info),
      tiovx_pad->pool_size, tiovx_pad->pool_size);

  ret = TRUE;

out:
  return ret;
}

gboolean
gst_tiovx_pad_query (GstPad * pad, GstObject * parent, GstQuery * query)
{
  GstTIOVXPad *tiovx_pad = GST_TIOVX_PAD (pad);
  gboolean ret = FALSE;

  g_return_val_if_fail (pad, ret);
  g_return_val_if_fail (query, ret);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_ALLOCATION:
      GST_DEBUG_OBJECT (pad, "Received allocation query");
      ret = gst_tiovx_pad_process_allocation_query (tiovx_pad, query);
      break;
    default:
      GST_DEBUG_OBJECT (pad, "Received non-allocation query");
      ret = gst_pad_query_default (pad, parent, query);
      break;
  }

  return ret;
}

static GstBuffer *
gst_tiovx_pad_copy_buffer (GstTIOVXPad * pad, GstTIOVXBufferPool * pool,
    GstBuffer * in_buffer)
{
  GstBuffer *out_buffer = NULL;
  GstBufferCopyFlags flags =
      GST_BUFFER_COPY_FLAGS | GST_BUFFER_COPY_TIMESTAMPS | GST_BUFFER_COPY_META
      | GST_BUFFER_COPY_DEEP;
  GstFlowReturn flow_return = GST_FLOW_ERROR;
  gboolean ret = FALSE;

  g_return_val_if_fail (pad, NULL);
  g_return_val_if_fail (pool, NULL);
  g_return_val_if_fail (in_buffer, NULL);

  flow_return =
      gst_buffer_pool_acquire_buffer (GST_BUFFER_POOL (pad->buffer_pool),
      &out_buffer, NULL);
  if (GST_FLOW_OK != flow_return) {
    GST_ERROR_OBJECT (pad, "Unable to acquire buffer from internal pool");
    goto out;
  }

  ret = gst_buffer_copy_into (out_buffer, in_buffer, flags, 0, kcopy_all_size);
  if (!ret) {
    GST_ERROR_OBJECT (pad,
        "Error copying from in buffer: %" GST_PTR_FORMAT " to out buffer: %"
        GST_PTR_FORMAT, in_buffer, out_buffer);
    gst_buffer_unref (out_buffer);
    out_buffer = NULL;
    goto out;
  }

  /* The in_buffer is no longer needed, it has been coopied to our TIOVX buffer */
  gst_buffer_unref (in_buffer);

out:
  return out_buffer;
}

GstFlowReturn
gst_tiovx_pad_chain (GstPad * pad, GstObject * parent, GstBuffer ** buffer)
{
  GstTIOVXPad *tiovx_pad = GST_TIOVX_PAD (pad);
  GstFlowReturn ret = GST_FLOW_ERROR;

  g_return_val_if_fail (pad, ret);
  g_return_val_if_fail (buffer, ret);
  g_return_val_if_fail (*buffer, ret);

  GST_INFO_OBJECT (pad, "Received a buffer for chaining");

  if ((*buffer)->pool != GST_BUFFER_POOL (tiovx_pad->buffer_pool)) {
    if (GST_TIOVX_IS_BUFFER_POOL ((*buffer)->pool)) {
      GST_INFO_OBJECT (pad,
          "Buffer's and Pad's buffer pools are different, replacing the Pad's");
      gst_object_unref (tiovx_pad->buffer_pool);

      tiovx_pad->buffer_pool = GST_TIOVX_BUFFER_POOL ((*buffer)->pool);
      gst_object_ref (tiovx_pad->buffer_pool);
    } else {
      GST_INFO_OBJECT (pad,
          "Buffer doesn't come from TIOVX, copying the buffer");

      *buffer =
          gst_tiovx_pad_copy_buffer (tiovx_pad, tiovx_pad->buffer_pool,
          *buffer);
    }
  }

  ret = GST_FLOW_OK;

  return ret;
}

GstFlowReturn
gst_tiovx_pad_acquire_buffer (GstTIOVXPad * pad, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  GstFlowReturn flow_return = GST_FLOW_ERROR;
  GstTIOVXMeta *meta = NULL;
  vx_object_array array = NULL;
  vx_reference image = NULL;

  g_return_val_if_fail (pad, flow_return);
  g_return_val_if_fail (buffer, flow_return);

  flow_return =
      gst_buffer_pool_acquire_buffer (GST_BUFFER_POOL (pad->buffer_pool),
      buffer, params);
  if (GST_FLOW_OK != flow_return) {
    GST_ERROR_OBJECT (pad, "Unable to acquire buffer from pool: %d",
        flow_return);
    goto exit;
  }

  /* Ensure that the exemplar & the meta have the same data */
  meta =
      (GstTIOVXMeta *) gst_buffer_get_meta (*buffer, GST_TIOVX_META_API_TYPE);

  array = meta->array;

  /* Currently, we support only 1 vx_image per array */
  image = vxGetObjectArrayItem (array, 0);

  gst_tiovx_transfer_handle (GST_OBJECT (pad), image, pad->exemplar);

  vxReleaseReference (&image);
exit:
  return flow_return;
}

static gboolean
gst_tiovx_pad_configure_pool (GstTIOVXPad * pad, GstCaps * caps,
    GstVideoInfo * info)
{
  GstTIOVXPad *tiovx_pad = GST_TIOVX_PAD (pad);
  GstStructure *config = NULL;
  gboolean ret = FALSE;

  g_return_val_if_fail (pad, ret);
  g_return_val_if_fail (caps, ret);
  g_return_val_if_fail (info, ret);

  if (!gst_video_info_from_caps (info, caps)) {
    GST_ERROR_OBJECT (pad, "Unable to get video info from caps");
    return FALSE;
  }

  config =
      gst_buffer_pool_get_config (GST_BUFFER_POOL (tiovx_pad->buffer_pool));

  gst_buffer_pool_config_set_exemplar (config, tiovx_pad->exemplar);
  gst_buffer_pool_config_set_params (config, caps, GST_VIDEO_INFO_SIZE (info),
      tiovx_pad->pool_size, tiovx_pad->pool_size);

  if (!gst_buffer_pool_set_config (GST_BUFFER_POOL (tiovx_pad->buffer_pool),
          config)) {
    GST_ERROR_OBJECT (pad, "Unable to set pool configuration");
    goto out;
  }

  gst_buffer_pool_set_active (GST_BUFFER_POOL (pad->buffer_pool), TRUE);

  ret = TRUE;

out:
  return ret;
}

static void
gst_tiovx_pad_finalize (GObject * object)
{
  GstTIOVXPad *tiovx_pad = GST_TIOVX_PAD (object);

  if (NULL != tiovx_pad->buffer_pool) {
    gst_object_unref (tiovx_pad->buffer_pool);
  }

  G_OBJECT_CLASS (gst_tiovx_pad_parent_class)->finalize (object);
}

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
 * *	Any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *	Nothing shall obligate TI to provide you with source code for the
 *      software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution
 * of the source code are permitted provided that the following conditions are met:
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

#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#include <gst/check/gstcheck.h>

#define NUMBER_OF_STATE_CHANGES 5

GstElement *test_create_pipeline (const gchar * pipe_desc);
GstElement *test_create_pipeline_fail (const gchar * pipe_desc);
void test_states_change (const gchar * pipe_desc);
void test_fail_properties_configuration (const gchar * pipe_desc);
void test_states_change_success (const gchar * pipe_desc);

void
test_states_change_fail (const gchar * pipe_desc);
static void
test_states_change_base (const gchar * pipe_desc, GstStateChangeReturn *state_change);

GstElement *
test_create_pipeline (const gchar * pipe_desc)
{
  GstElement *pipeline = NULL;
  GError *error = NULL;

  GST_INFO ("testing pipeline %s", pipe_desc);

  pipeline = gst_parse_launch (pipe_desc, &error);

  /* Check for errors creating pipeline */
  fail_if (error != NULL, error);
  fail_if (pipeline == NULL, error);

  return pipeline;
}

GstElement *
test_create_pipeline_fail (const gchar * pipe_desc)
{
  GstElement *pipeline = NULL;
  GError *error = NULL;

  GST_INFO ("testing pipeline %s", pipe_desc);

  pipeline = gst_parse_launch (pipe_desc, &error);

  /* Check for errors creating pipeline */
  fail_if (error == NULL, error);

  return pipeline;
}

static void
test_states_change_base (const gchar * pipe_desc, GstStateChangeReturn *state_change)
{
    GstElement *pipeline = NULL;
    gint i = 0;

    pipeline = test_create_pipeline (pipe_desc);

    for (i = 0; i < NUMBER_OF_STATE_CHANGES; i++) {
      fail_unless_equals_int (gst_element_set_state (pipeline, GST_STATE_PLAYING),
          state_change[0]);
      fail_unless_equals_int (gst_element_get_state (pipeline, NULL, NULL, -1),
          state_change[1]);
      fail_unless_equals_int (gst_element_set_state (pipeline, GST_STATE_NULL),
          state_change[2]);
    }
    gst_object_unref (pipeline);
}

void
test_states_change (const gchar * pipe_desc)
{
    GstStateChangeReturn state_change[] = {GST_STATE_CHANGE_ASYNC, GST_STATE_CHANGE_SUCCESS, GST_STATE_CHANGE_SUCCESS};

    test_states_change_base(pipe_desc, state_change);
}

void
test_states_change_fail (const gchar * pipe_desc)
{
    GstStateChangeReturn state_change[] = {GST_STATE_CHANGE_FAILURE, GST_STATE_CHANGE_FAILURE, GST_STATE_CHANGE_FAILURE};

    test_states_change_base(pipe_desc, state_change);
}

void
test_states_change_success (const gchar * pipe_desc)
{
    GstStateChangeReturn state_change[] = {GST_STATE_CHANGE_SUCCESS, GST_STATE_CHANGE_SUCCESS, GST_STATE_CHANGE_SUCCESS};

    test_states_change_base(pipe_desc, state_change);
}

#endif

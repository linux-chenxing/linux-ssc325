/*
 *	uvc_v4l2.c  --  USB Video Class Gadget driver
 *
 *	Copyright (C) 2009-2010
 *	    Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/videodev2.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>

#include <media/v4l2-dev.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>

#include "f_uvc.h"
#include "u_uvc.h"
#include "uvc.h"
#include "uvc_queue.h"
#include "uvc_video.h"
#include "uvc_v4l2.h"

/* --------------------------------------------------------------------------
 * Requests handling
 */

static int
uvc_send_response(struct uvc_device *uvc, struct uvc_request_data *data)
{
	struct usb_composite_dev *cdev = uvc->func.config->cdev;
	struct usb_request *req = uvc->control_req;

	if (data->length < 0)
		return usb_ep_set_halt(cdev->gadget->ep0);

	req->length = min_t(unsigned int, uvc->event_length, data->length);
	req->zero = data->length < uvc->event_length;

	memcpy(req->buf, data->data, req->length);

	return usb_ep_queue(cdev->gadget->ep0, req, GFP_KERNEL);
}

/* --------------------------------------------------------------------------
 * V4L2 ioctls
 */

struct uvc_format
{
	u8 bpp;
	u32 fcc;
};

static struct uvc_format uvc_formats[] = {
	{ 16, V4L2_PIX_FMT_YUYV  },
	{ 12, V4L2_PIX_FMT_NV12  },
	{ 0,  V4L2_PIX_FMT_MJPEG },
	{ 0,  V4L2_PIX_FMT_H264  },
	{ 0,  V4L2_PIX_FMT_H265  },
};

static int
uvc_v4l2_querycap(struct file *file, void *fh, struct v4l2_capability *cap)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_device *uvc = stream->dev;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
#endif
	struct usb_composite_dev *cdev = uvc->func.config->cdev;

	strlcpy(cap->driver, "g_uvc", sizeof(cap->driver));
	strlcpy(cap->card, cdev->gadget->name, sizeof(cap->card));
	strlcpy(cap->bus_info, dev_name(&cdev->gadget->dev),
		sizeof(cap->bus_info));


	return 0;
}

static int
uvc_v4l2_get_format(struct file *file, void *fh, struct v4l2_format *fmt)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_video *video = &stream->video;
#else
    struct uvc_device *uvc = video_get_drvdata(vdev);
    struct uvc_video *video = &uvc->video;
#endif

	fmt->fmt.pix.pixelformat = video->fcc;
	fmt->fmt.pix.width = video->width;
	fmt->fmt.pix.height = video->height;
	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = video->bpp * video->width / 8;
	fmt->fmt.pix.sizeimage = video->imagesize;
	fmt->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
	fmt->fmt.pix.priv = 0;

	return 0;
}

static int
uvc_v4l2_set_format(struct file *file, void *fh, struct v4l2_format *fmt)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_device *uvc = stream->dev;
	struct uvc_video *video = &stream->video;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
	struct uvc_video *video = &uvc->video;
#endif
	struct uvc_format *format;
	unsigned int imagesize;
	unsigned int bpl;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(uvc_formats); ++i) {
		format = &uvc_formats[i];
		if (format->fcc == fmt->fmt.pix.pixelformat)
			break;
	}

	if (i == ARRAY_SIZE(uvc_formats)) {
		uvcg_info(&uvc->func, "Unsupported format 0x%08x.\n",
			  fmt->fmt.pix.pixelformat);
		return -EINVAL;
	}

	bpl = format->bpp * fmt->fmt.pix.width / 8;
	imagesize = bpl ? bpl * fmt->fmt.pix.height : fmt->fmt.pix.sizeimage;

	video->fcc = format->fcc;
	video->bpp = format->bpp;
	video->width = fmt->fmt.pix.width;
	video->height = fmt->fmt.pix.height;
	video->imagesize = imagesize;
	video->max_payload_size = imagesize; // for bulk mode

	fmt->fmt.pix.field = V4L2_FIELD_NONE;
	fmt->fmt.pix.bytesperline = bpl;
	fmt->fmt.pix.sizeimage = imagesize;
	fmt->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
	fmt->fmt.pix.priv = 0;

	return 0;
}

static int
uvc_v4l2_reqbufs(struct file *file, void *fh, struct v4l2_requestbuffers *b)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_video *video = &stream->video;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
	struct uvc_video *video = &uvc->video;
#endif

	if (b->type != video->queue.queue.type)
		return -EINVAL;

	return uvcg_alloc_buffers(&video->queue, b);
}

static int
uvc_v4l2_querybuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	//struct uvc_device *uvc = stream->dev;
	struct uvc_video *video = &stream->video;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
	struct uvc_video *video = &uvc->video;
#endif

	return uvcg_query_buffer(&video->queue, b);
}

static int
uvc_v4l2_qbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	//struct uvc_device *uvc = stream->dev;
	struct uvc_video *video = &stream->video;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
	struct uvc_video *video = &uvc->video;
#endif

	int ret;
#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	video->queue.reserved = b->reserved;
	b->reserved = 0;
#endif
	ret = uvcg_queue_buffer(&video->queue, b);
	if (ret < 0)
		return ret;

	schedule_work(&video->pump);

	return ret;
}

static int
uvc_v4l2_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	//struct uvc_device *uvc = stream->dev;
	struct uvc_video *video = &stream->video;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
	struct uvc_video *video = &uvc->video;
#endif

	return uvcg_dequeue_buffer(&video->queue, b, file->f_flags & O_NONBLOCK);
}

static int
uvc_v4l2_streamon(struct file *file, void *fh, enum v4l2_buf_type type)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_device *uvc = stream->dev;
	struct uvc_video *video = &stream->video;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
	struct uvc_video *video = &uvc->video;
#endif

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	struct f_uvc_opts *opts = fi_to_f_uvc_opts(uvc->func.fi);
#endif
	int ret;

	if (type != video->queue.queue.type)
		return -EINVAL;

	/* Enable UVC video. */
	ret = uvcg_video_enable(video, 1);
	if (ret < 0)
		return ret;

#if defined(CONFIG_SS_GADGET) ||defined(CONFIG_SS_GADGET_MODULE)
	if (opts->bulk_streaming_ep)
	#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
		stream->state = UVC_STATE_STREAMING;
	#else
		uvc->state = UVC_STATE_STREAMING;
	#endif
	else
#endif
	{
	/*
	 * Complete the alternate setting selection setup phase now that
	 * userspace is ready to provide video frames.
	 */
		uvc_function_setup_continue(uvc);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
		stream->state = UVC_STATE_STREAMING;
#else
		uvc->state = UVC_STATE_STREAMING;
#endif
	}

	return 0;
}

static int
uvc_v4l2_streamoff(struct file *file, void *fh, enum v4l2_buf_type type)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	//struct uvc_device *uvc = stream->dev;
	struct uvc_video *video = &stream->video;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
	struct uvc_video *video = &uvc->video;
#endif

	if (type != video->queue.queue.type)
		return -EINVAL;

	return uvcg_video_enable(video, 0);
}

static int
uvc_v4l2_subscribe_event(struct v4l2_fh *fh,
			const struct v4l2_event_subscription *sub)
{
	if (sub->type < UVC_EVENT_FIRST || sub->type > UVC_EVENT_LAST)
		return -EINVAL;

	return v4l2_event_subscribe(fh, sub, 2, NULL);
}

static int
uvc_v4l2_unsubscribe_event(struct v4l2_fh *fh,
			const struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

static long
uvc_v4l2_ioctl_default(struct file *file, void *fh, bool valid_prio,
			unsigned int cmd, void *arg)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_device *uvc = stream->dev;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
#endif

	switch (cmd) {
	case UVCIOC_SEND_RESPONSE:
		return uvc_send_response(uvc, arg);
	default:
		return -ENOIOCTLCMD;
	}
}

const struct v4l2_ioctl_ops uvc_v4l2_ioctl_ops = {
	.vidioc_querycap = uvc_v4l2_querycap,
	.vidioc_g_fmt_vid_out = uvc_v4l2_get_format,
	.vidioc_s_fmt_vid_out = uvc_v4l2_set_format,
	.vidioc_reqbufs = uvc_v4l2_reqbufs,
	.vidioc_querybuf = uvc_v4l2_querybuf,
	.vidioc_qbuf = uvc_v4l2_qbuf,
	.vidioc_dqbuf = uvc_v4l2_dqbuf,
	.vidioc_streamon = uvc_v4l2_streamon,
	.vidioc_streamoff = uvc_v4l2_streamoff,
	.vidioc_subscribe_event = uvc_v4l2_subscribe_event,
	.vidioc_unsubscribe_event = uvc_v4l2_unsubscribe_event,
	.vidioc_default = uvc_v4l2_ioctl_default,
};

/* --------------------------------------------------------------------------
 * V4L2
 */

static int
uvc_v4l2_open(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_device *uvc = stream->dev;
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);
#endif
	struct uvc_file_handle *handle;

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (handle == NULL)
		return -ENOMEM;

	v4l2_fh_init(&handle->vfh, vdev);
	v4l2_fh_add(&handle->vfh);

#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	handle->device = &stream->video;
	stream->active = 1;
#else
	handle->device = &uvc->video;
#endif
	file->private_data = &handle->vfh;

	uvc_function_connect(uvc);
	return 0;
}

static int
uvc_v4l2_release(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	struct uvc_device *uvc = stream->dev;
#else
    struct uvc_device *uvc = video_get_drvdata(vdev);
#endif
	struct uvc_file_handle *handle = to_uvc_file_handle(file->private_data);
	struct uvc_video *video = handle->device;

	uvc_function_disconnect(uvc);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	stream->active = 0;
#endif

	mutex_lock(&video->mutex);
	uvcg_video_enable(video, 0);
	uvcg_free_buffers(&video->queue);
	mutex_unlock(&video->mutex);

	file->private_data = NULL;
	v4l2_fh_del(&handle->vfh);
	v4l2_fh_exit(&handle->vfh);
	kfree(handle);

	return 0;
}

static int
uvc_v4l2_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	//struct uvc_device *uvc = stream->dev;

	return uvcg_queue_mmap(&stream->video.queue, vma);
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);

	return uvcg_queue_mmap(&uvc->video.queue, vma);
#endif
}

static unsigned int
uvc_v4l2_poll(struct file *file, poll_table *wait)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	//struct uvc_device *uvc = stream->dev;

	return uvcg_queue_poll(&stream->video.queue, file, wait);
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);

	return uvcg_queue_poll(&uvc->video.queue, file, wait);
#endif
}

#ifndef CONFIG_MMU
static unsigned long uvcg_v4l2_get_unmapped_area(struct file *file,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags)
{
	struct video_device *vdev = video_devdata(file);
#ifdef CONFIG_SS_GADGET_UVC_MULTI_STREAM
	struct uvc_streaming *stream = video_get_drvdata(vdev);
	//struct uvc_device *uvc = stream->dev;

	return uvcg_queue_get_unmapped_area(&stream->video.queue, pgoff);
#else
	struct uvc_device *uvc = video_get_drvdata(vdev);

	return uvcg_queue_get_unmapped_area(&uvc->video.queue, pgoff);
#endif
}
#endif

struct v4l2_file_operations uvc_v4l2_fops = {
	.owner		= THIS_MODULE,
	.open		= uvc_v4l2_open,
	.release	= uvc_v4l2_release,
	.unlocked_ioctl	= video_ioctl2,
	.mmap		= uvc_v4l2_mmap,
	.poll		= uvc_v4l2_poll,
#ifndef CONFIG_MMU
	.get_unmapped_area = uvcg_v4l2_get_unmapped_area,
#endif
};


// SPDX-License-Identifier: MPL-2.0
// Copyright (c) Yuxuan Shui <yshuiv7@gmail.com>
#include <Imlib2.h>
#include <stdio.h>
#include <xcb/render.h>
#include <xcb/xcb.h>

#include "log.h"
#include "x.h"
#include "utils/image.h"

xcb_pixmap_t load_image_as_pixmap(struct x_connection *c, const char *path, int *width, int *height) {
	Imlib_Image image = imlib_load_image(path);
	if (!image) {
		log_error("Failed to load image: %s", path);
		return XCB_NONE;
	}

	imlib_context_set_image(image);
	int w = imlib_image_get_width();
	int h = imlib_image_get_height();
	*width = w;
	*height = h;

	uint32_t *data = imlib_image_get_data_for_reading_only();

	xcb_pixmap_t pixmap = x_create_pixmap(c, 32, (uint16_t)w, (uint16_t)h);
	if (pixmap == XCB_NONE) {
		log_error("Failed to create pixmap for image");
		imlib_free_image();
		return XCB_NONE;
	}

	xcb_gcontext_t gc = xcb_generate_id(c->c);
	xcb_create_gc(c->c, gc, pixmap, 0, NULL);

	// Imlib2 data is ARGB 8888, same as what we usually want for XRender 32-bit
	xcb_put_image(c->c, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, gc, (uint16_t)w, (uint16_t)h, 0, 0, 0, 32,
	              (uint32_t)(w * h * 4), (const uint8_t *)data);

	xcb_free_gc(c->c, gc);
	imlib_free_image();

	return pixmap;
}

xcb_pixmap_t blur_pixmap(struct x_connection *c, xcb_pixmap_t src, int *width, int *height,
                         int radius) {
	if (src == XCB_NONE || radius <= 0) {
		return XCB_NONE;
	}

	imlib_context_set_display(c->dpy);
	imlib_context_set_visual(DefaultVisual(c->dpy, c->screen));
	imlib_context_set_colormap(DefaultColormap(c->dpy, c->screen));
	imlib_context_set_drawable(src);

	// Get geometry
	xcb_get_geometry_reply_t *r =
	    xcb_get_geometry_reply(c->c, xcb_get_geometry(c->c, src), NULL);
	if (!r) {
		return XCB_NONE;
	}
	uint16_t w = r->width;
	uint16_t h = r->height;
	*width = w;
	*height = h;
	free(r);

	Imlib_Image image = imlib_create_image_from_drawable(0, 0, 0, w, h, 1);
	if (!image) {
		log_error("Failed to create Imlib2 image from root pixmap");
		return XCB_NONE;
	}

	imlib_context_set_image(image);
	imlib_image_blur(radius);

	uint32_t *data = imlib_image_get_data_for_reading_only();
	xcb_pixmap_t pixmap = x_create_pixmap(c, 32, w, h);
	if (pixmap != XCB_NONE) {
		xcb_gcontext_t gc = xcb_generate_id(c->c);
		xcb_create_gc(c->c, gc, pixmap, 0, NULL);
		xcb_put_image(c->c, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, gc, w, h, 0, 0, 0, 32,
		              (uint32_t)(w * h * 4), (const uint8_t *)data);
		xcb_free_gc(c->c, gc);
	}

	imlib_free_image();
	return pixmap;
}

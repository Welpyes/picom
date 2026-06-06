// SPDX-License-Identifier: MPL-2.0
// Copyright (c) Yuxuan Shui <yshuiv7@gmail.com>
#pragma once
#include <xcb/xcb.h>
#include "x.h"

xcb_pixmap_t load_image_as_pixmap(struct x_connection *c, const char *path, int *width, int *height);
xcb_pixmap_t blur_pixmap(struct x_connection *c, xcb_pixmap_t src, int *width, int *height,
                         int radius);

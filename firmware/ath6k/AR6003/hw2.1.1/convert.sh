#!/bin/sh

arm-none-linux-gnueabi-objcopy -Ibinary $1 -Oihex $1.ihex

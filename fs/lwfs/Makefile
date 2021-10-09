#
# Makefile for the linux LwFS filesystem routines.
#

obj-$(CONFIG_LWFS_FS) += lwfs.o

lwfs-y := storage.o super.o

ifneq ($(CONFIG_MMU),y)
lwfs-$(CONFIG_LWFS_ON_MTD) += mmap-nommu.o
endif


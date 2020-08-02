
MODULE := disk

$(MODULE)_COBJS = part.o part_dos.o part_efi.o

include $(TOP_DIR)/modules-inc.mk
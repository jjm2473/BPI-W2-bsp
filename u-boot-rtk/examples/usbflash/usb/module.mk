
MODULE := usb
$(MODULE)_COBJS = host/ehci-hcd.o host/ehci-rtk.o host/host-rtk.o host/xhci.o host/xhci-mem.o host/xhci-ring.o host/xhci-plat.o phy/phy-rtk-usb2.o phy/phy-rtk-usb3.o rtd1295/usb.o rtd1295/usb-phy.o common/usb.o common/usb_hub.o common/usb_storage.o usb.o
include $(TOP_DIR)/modules-inc.mk
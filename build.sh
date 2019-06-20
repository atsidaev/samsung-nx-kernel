#!/bin/sh

DEST_NX300=../..
DEST_NX2000=../../project/NX2000

case "$1" in
	defconfig)
    if [ $2 = nx300 ]
    then
	    echo "load NX300 kernel configuration"
	    make ARCH=arm drime4_defconfig
    elif [ $2 = nx2000 ]
    then
	    echo "load NX2000 kernel configuration"
	    make ARCH=arm drime4_nx2000_defconfig
    else
	    echo "We only support nx300 or nx2000"
    fi
	;;
	
	menuconfig)
	echo "menuconfig"
	make ARCH=arm menuconfig
	;;
		
	uImage)
	echo "build uImage"
    if [ $2 = nx300 ]
    then
        DEST=$DEST_NX300
    elif [ $2 = nx2000 ]
    then
        DEST=$DEST_NX2000
    else
        echo "We only support nx300 or nx2000"
        exit
    fi
	make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- uImage 2>make.log
	cp -f arch/arm/boot/uImage $DEST/binary/
	;;
	
	modules)
	echo "build modules"
    if [ $2 = nx300 ]
    then
        DEST=$DEST_NX300
    elif [ $2 = nx2000 ]
    then
        DEST=$DEST_NX2000
    else
        echo "We only support nx300 or nx2000"
        exit
    fi 
	make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- modules
    #cp -f drivers/mmc/host/dw_mmc_sdcard-pltfm.ko $DEST/image/rootfs/usr/lib/
    #cp -f drivers/mmc/host/dw_mmc_sdio-pltfm.ko $DEST/image/rootfs/usr/lib/
	cp -f drivers/net/wireless/ath/ath.ko $DEST/image/rootfs/lib/modules/
	cp -f drivers/net/wireless/ath/ath6kl/ath6kl_sdio.ko $DEST/image/rootfs/lib/modules/
	cp -f net/wireless_ath/cfg80211.ko $DEST/image/rootfs/lib/modules/
    
    #cp -f drivers/mmc/host/dw_mmc_sdcard-pltfm.ko $DEST/binary/
    #cp -f drivers/mmc/host/dw_mmc_sdio-pltfm.ko $DEST/binary/
	cp -f drivers/net/wireless/ath/ath.ko $DEST/binary/
	cp -f drivers/net/wireless/ath/ath6kl/ath6kl_sdio.ko $DEST/binary/
	cp -f net/wireless_ath/cfg80211.ko $DEST/binary/
	;;
		
	distclean)
	echo "distclean"
	make ARCH=arm KDIR=$PWD distclean
	;;

	clean)
	echo "clean"
	make ARCH=arm clean
	;;
esac

# insmod /lib/modules/cfg80211.ko
# insmod /lib/modules/ath6kl_sdio.ko ath6kl_p2p=1
# /usr/sbin/wpa_supplicant -Dnl80211 -iwlan0 -c/data/misc/wifi/wpa_supplicant.conf -u -B -P/var/run/station_wpa_supplicant.pid
# connmand -W nl80211
# dbus-send --system --dest=net.connman --print-reply / net.connman.Manager.EnableTechnology string:"wifi"

# wpa_cli -p/var/run/wifi -iwlan0 scan
# wpa_cli -p/var/run/wifi -iwlan0 scan_results

# iwconfig wlan0 essid SAMSUNG-PINK
# ifconfig wlan0 192.168.0.11

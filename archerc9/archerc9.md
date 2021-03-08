OpenWRT BCM ...And the entire flash usage looks like this -

CFE	trx	gz'd lzma	lzma'd kernel	SquashFS	JFFS2 filesystem	NVRAM

1M   FFFFF   (100000)
16M  FFFFFF  (1000000)
128M FFFFFFF (10000000)

factory-to-ddwrt:

....fwup-ptn par
tition-table bas
e 0x00800 size 0
x00800...fwup-pt
n soft-version b
ase 0x01000 size
 0x00019...fwup-
ptn support-list
 base 0x01019 si
ze 0x00282...fwu
p-ptn os-image b
ase 0x0129b size
 0x199000...fwup
-ptn file-system
 base 0x19a29b s
ize 0xab0d9a...f
wup-ptn extra-pa
ra base 0xc4b035
 size 0x0000b...


image partition table 1014, size 2048 bytes, 0x800
all image base offset to 1014

fw partiontion table 1814
# os-image 
0x0129b+1014 
x199000
dd if=factory-to-ddwrt.bin skip=$((0x0129b+0x1014)) count=$((0x199000)) bs=1 of=os-images-hex.bin

# fs-image

0x19a29b +1014
0xab0d9a
dd if=factory-to-ddwrt.bin skip=$((0x19a29b+0x1014)) count=$((0xab0d9a)) bs=1 of=fs-images-hex.bin

dd if=factory-to-ddwrt.bin skip=6164 count=2048 bs=1 of=partition-table.bin

# extra-para for the double boot configuration (factory boot and uboot)


tp-link fw:

...K.0.....board
type=0x0665.boar
dnum=001.boardre
v=0x1102.boardfl
ags=0x00000110.b
oardflags2=0x000
00000.sromrev=8.
clkfreq=1000,800
.xtalfreq=25000.
sdram_config=0x0
14B.sdram_refres
h=0x0c30.et0phya
ddr=30.et0mdcpor
t=0.et0macaddr=0
0:90:4C:11:20:01

SupportList:
{product_name:ArcherC9,product_ver:5.0.0,special_id:45550000}
{product_name:ArcherC9,product_ver:5.0.0,special_id:55530000}
{product_name:ArcherC9,product_ver:5.0.0,special_id:4A500000}


tp-link partion table

total=20,  flash=16M
0=        factory-boot, 0x00000000, 0x00040000, 0, 0, 0, 1, #
1=            fs-uboot, 0x00040000, 0x00040000, 0, 0, 0, 0, #
2=            os-image, 0x00080000, 0x00200000, 0, 0, 0, 0, #
3=         file-system, 0x00280000, 0x00c80000, 0, 0, 0, 0, #
4=         default-mac, 0x00f00000, 0x00000200, 0, 2, 2, 3, #
5=                 pin, 0x00f00200, 0x00000100, 0, 2, 2, 3, #
6=           device-id, 0x00f00300, 0x00000100, 0, 2, 2, 3, #
7=        product-info, 0x00f10000, 0x00006000, 2, 2, 2, 1, productinfo.bin
8=        soft-version, 0x00f16000, 0x00001000, 3, 2, 2, 0, #
9=          extra-para, 0x00f17000, 0x00001000, 6, 2, 2, 0, #
10=       support-list, 0x00f18000, 0x00008000, 0, 2, 2, 0, supportlist.bin
11=            profile, 0x00f20000, 0x00003000, 0, 2, 2, 0, profile.xml
12=     default-config, 0x00f23000, 0x0000d000, 0, 2, 2, 0, defaultconfig.xml
13=        user-config, 0x00f30000, 0x00040000, 0, 2, 2, 3, #
14=             qos-db, 0x00f70000, 0x00040000, 0, 0, 0, 0, database.bin
15=    partition-table, 0x00fb0000, 0x00010000, 1, 3, 1, 0, partition.conf //fb0000 - 100000*2 = db0000
16=        certificate, 0x00fc0000, 0x00010000, 0, 0, 0, 3, #
17=       merge-config, 0x00fd0000, 0x00010000, 0, 2, 2, 0, mergeconfig.xml
18=           radio_bk, 0x00fe0000, 0x00010000, 0, 0, 0, 3, #
19=              radio, 0x00ff0000, 0x00010000, 0, 0, 0, 3, #

[分区参数注释]
注释=#total 分区数目  #flash flash大小，2M/4M/8M/16M/32M
#第一列 分区序号
#第二列 分区名称
#第三列 分区BASE
#第四列 分区SIZE
#第五列 分区内容来源，0-OUTPUT(外部输入)/1-PTNTBL(分区文件表)/2-PDINFO(产品信息)/3-SOFTVER(软件信息)/4-PROFILE(启动参数)/5-SIGNATURE(数字签名)
#第六列 FLASH文件头部偏移字节数
#第七列 UP文件头部偏移字节数
#第八列 指示UP和FLASH文件生成方式，0-缺省方式，同时写入UP和FLASH;1-只写入FLASH;2-只写入UP;3-都不写入
#第九列 文件名称, #表示空,其他表示文件名称


Boot partition size = 1048576(0x100000)              cat /proc/mtd
Creating 3 MTD partitions on "nflash":
0x000000000000-0x000000100000 : "boot"   1M          mtd0 00100000 20000 boot          /dev/mtdblock0
0x000000100000-0x000000200000 : "nvram"  1M          mtd1 00100000 20000 nvram         /dev/mtdblock1
0x000002200000-0x000004200000 : "backup" 32M         mtd2 02000000 20000 backup        /dev/mtdblock2 (bkup of rootfs, block3)

Creating 3 MTD partitions on "brcmnand":
0x000000200000-0x000002200000 : "rootfs"  32M        mtd3 02000000 20000 rootfs        /dev/mtdblock3
0x000004200000-0x000007c00000 : "storage" 58M        mtd4 03a00000 20000 storage       /dev/mtdblock4 (empty, mounted on /storage)
0x000007c00000-0x000008000000 : "data"    4M         mtd5 00400000 20000 data          /dev/mtdblock5 (appears to be empty)

dd if=/dev/mtdblock1 of=mnt/sda1/archerc9/mtdblock1.bin bs=1

weid:mtd jd$ xxd mtdblock3.bin | grep  'partition' 
00295be0: c7ff 7375 7061 7274 6974 696f 6e5f 636f  ..supartition_co
002b7ec0: 0b96 ea97 7061 7274 6974 696f 6e2d 7461  ....partition-ta

00295be0: c7ff 7375 7061 7274 6974 696f 6e5f 636f  ..supartition_co
00295bf0: 6e66 6967 8519 02e0 4400 0000 1dfb f798  nfig....D.......



weid:mtd jd$ xxd  mtdblock5.bin | grep -v ffff 
00080000: 8519 02e0 4305 0000 4f01 eb03 1100 0000  ....C...O.......
00080010: 0a00 0000 a481 0000 0000 0000 0080 0000  ................
00080020: d021 bb5a d021 bb5a d021 bb5a 0373 0000  .!.Z.!.Z.!.Z.s..
00080030: ff04 0000 fd0c 0000 0600 0000 f055 8cb4  .............U..
00080040: 4f69 1075 785e 9457 4b6f e336 10be fb57  Oi.ux^.WKo.6...W


## dd-wrt archerc9 v5 thread

https://forum.dd-wrt.com/phpBB2/viewtopic.php?t=315680&postdays=0&postorder=asc&start=0

## dd-wrt archerc9 v4 thread
https://forum.dd-wrt.com/phpBB2/viewtopic.php?t=305884&postdays=0&postorder=asc&start=135

The earlier procedure has been expanded to also fix up the jffs2 corruption bug.
You need two files. The first file is attached to this message.

Fair Warning: This procedure can really brick your unit if the flash command is typed wrong. This procedure can overwrite critical data if the wrong file is loaded.

This procedure requires the use of the serial port on the circuit board.

This binary file (C9-V3-Fix-R5.bin) can be used as part of the procedure to recover an Archer C9 V3 from a bad DD-WRT flash. It has been tried against build 34311 and 34080. It should work (but has not been tested) against the other bad dd-wrt builds from December 2017.

It also can recover from the corruption after enabling jffs2 in DD-WRT in versions 33525 and 33772. It should work (but has not been tested) against other builds where jffs2 has been enabled and corrupted the flash.

The bad dd-wrt load and or jffs2 bug has overwritten the MAC address of the unit. The C9-V3-Fix-R5.bin file has a MAC address of AA-BB-CC-CC-BB-AA. You should change the MAC address in the file to the one that is on the back of your unit.

Using a hex edititor (for example, HxD under windows) change the hex values located at offset 8,9,A,B,C,D in the file. If the back of your unit is labled MAC:00-12-10-A0-33-2F, change the hex value at 8 to 00, at 9 to 12, at A to 10, B to A0, C to 33 and D to 2F. If using HxD, perform your editing on the left, hex side of the display, not the ASCII side. Save the changes back to the file.

Grab stock firmware from TP-LINK's website and rename it to ArcherC9v3_tp_recovery.bin.

The general instructions are:
1. Connect the serial port to the main board.
2. Connect a PC to a LAN port on the C9v3.
3. Set the PC Ethernet interface to 192.168.0.66 with a subnet mask of 255.255.255.0
4. Run TFTP server on the PC. Point the TFTP server to the directory with the C9-V3-Fix-R5.bin and ArcherC9v3_tp_recovery.bin files.
5. At the CFE prompt of the C9v3, cut and paste this command:
flash -offset=15728640 -noheader 192.168.0.66:C9-V3-Fix-R5.bin flash0
(Make sure that the -offset value is correct before hitting enter.)
(If the response to the flash command is "device not found" remove the 0 from flash0 and try again.)

If all goes well, you will see this upon completion:
CFE> flash -offset=15728640 -noheader 192.168.0.66:C9-V3-Fix-R5.bin flash0
Reading 192.168.0.66:C9-V3-Fix-R5.bin: Done. 917504 bytes read
Programming...done. 917504 bytes written
*** command status = 0
CFE>

6. When finished, reboot or reset. With some of the corruptions, the serial console shows the unit booting into DD-WRT. With others, the unit repeatedly trys to boot and has a kernal panic. Considering how messed up the firmware was, immediately do a TFTP recovery and load TP-LINK firmware onto the unit.
7. Download the stock firmware for V3 from TP-Link. Unzip the .bin from the file. Place it in the folder that the TFTP server points to. Rename the file to ArcherC9v3_tp_recovery.bin.
8. Power off the unit. Hold down the reset button and power the unit back up. When you see on the serial console that the unit is in recovery mode, release the reset button.
9. The recovery software will load and the unit will reboot with TP-LINK firmware.
10. Change the PC ethernet port back to automatically get an IP address and DNS server.
11. At this point DD-WRT can be reloaded using the factory-to-ddwrt.bin file from the TP-LINK gui.

https://forum.dd-wrt.com/phpBB2/viewtopic.php?t=305884&postdays=0&postorder=asc&start=30

By the way, TP-Link provides "GPL code" which even includes binary toolchain for building:

http://static.tp-link.com/resources/gpl/GPL_C9v5_EU_20170110.tar.gz

Code:

1. This package contains all GPL code used by TP-LINK Routers/APs with Linux OS.
2. All components have been built successfully on Redhat Enterprise Linux 5.0 Workstation.
3. Compiling components in this package on platforms other than Redhat Enterprise Linux 5.0 Workstation may have unexpected results.
4. Recommended using "root" or "sudo" command to build the code.
5. After building code, you can get linux kernel image, then you can make rootfs with mksquashfs tool, but you can't upgrade these images to your router through web management page.

Build Instructions
1. All build targets are in ~/GPL_Code/build/Makefile, you should enter this directory to build components.
2. Pre-built toolchain is avaliable in this package.
3. Toolchain (include uclibc) source code is avaliable in toolchain_src.
4. Prepare Pre-built toolchain, tools and filesystem: 
make linux_build_prep

5. Build core components:(includes linux kernel, kernel modules and some applications)
make linux_build_all

6. Install binary and libs
make install

7. Build root fs:
make image

Note: please build the GPL codes following the order(1-7), otherwise it has risk to compile error.
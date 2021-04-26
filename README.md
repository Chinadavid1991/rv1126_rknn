# RV1126


## 编译

可在 Rockchip RV1109/RV1126 SDK 的 buildroot 编译此程序。

1. 把程序源码放置到 SDK 的 external/ 目录。
2. 在 SDK 的 buildroot/package/rockchip 目录下创建 demo_h264_rkmedia  目录。
3. 在 buildroot/package/rockchip/demo_h264_rkmedia  添加文件 Config.in。

```SHELL
config BR2_PACKAGE_DEMO_H264_RKMEDIA
	bool "demo_h264_rkmedia: test rkmedia H264 encode"
	select BR2_PACKAGE_RKMEDIA
	select BR2_PACKAGE_ISP2_IPC
```

4. 在 buildroot/package/rockchip/demo 添加文件 demo.mk。

```makefile
DEMO_SITE = $(TOPDIR)/../external/demo
DEMO_SITE_METHOD = local

DEMO_H264_RKMEDIA_DEPENDENCIES = rkmedia

$(eval $(cmake-package))
```

5. 修改 buildroot/package/rockchip/Config.in，添加：

```shell
source "package/rockchip/demo_h264_rkmedia/Config.in"
```

6. 在 buildroot 目录执行 `make menuconfig` 将此程序选上；或者直接在 defconfig 文件（ 如 buildroot/configs/rockchip_rv1126_rv1109_defconfig）将此 package 选上：

```shell
BR2_PACKAGE_DEMO_H264_RKMEDIA=y
```

6. 在 buildroot 目录执行 `make demo ` 编译程序，最终可执行文件 demo 被安装到 buildroot/output 的目标目录下。
7. 如果在 external/demo_h264_rkmedia 中修改完代码需要重新编译，可执行：

```shell
source envsetup.sh
cd buildroot
make demo-dirclean
make demo-rebuild
```



## 运行

demo最终被安装到系统的 /usr/bin 目录，重新打包根文件系统固件并烧录可在设备运行程序。

也可以直接从 buildroot/output 中将 demo_h264_rkmedia 复制出来，使用 adb 或者其他方法把 demo_h264_rkmedia 放置到设备上。

与运行 rkmedia 的 examples 程序相同，运行 demo_h264_rkmedia 也需要将 ispserver 先运行起来。如果未运行 ispserver 可执行：

```shell
ispserver -no-sync-db &
```


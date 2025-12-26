# Overview
`netbird_connection_watcher` is an application that monitors connection status of a netbird client daemon service running on a `mips`- based embedded Linux devices and sends it to host PC so it can be observed in a browser on a local host:

![netbird connection watcher demo GIF](./images/netbird-connection-watcher-demo.gif)

## Prerequisites
- OpenWRT device with a network connection with the host machine and a running netbird client
- `build-essential` and `make` - standard development tools
- `mipsel-linux-musl-gcc` - cross-compiler toolchain
    - if not available from the package repository, obtain the source directly, build it locally and add it to PATH:
    ```
    cd
    wget https://musl.cc/mipsel-linux-musl-cross.tgz
    tar xf mipsel-linux-musl-cross.tgz
    export PATH=$PWD/mipsel-linux-musl-cross/bin:$PATH
    ```
- `ssh` installed both on host machine and target device

## Build and run
*For simplicty's sake, the binary you obtain from cross-compiling will be directly copied to the device in `tmp` folder instead of creating an .ipk package that would be integrated into the image.*

From the root of the project directory, simply run `make` which will create the binary inside the build directory. Now copy the binary to the device with:
```
scp -O build/netbird_connection_watcher  root@<target-device-ip>:/tmp/
``` 

Now, ssh into the device and run:
```
ssh -L 8080:127.0.0.1:8080 root@<target-device-ip>
```
to bridge the embedded Linux device and host PC by enabling local port forwarding. This way, the PC acts as some kind of a proxy to the router and visiting `http://localhost:8080` in the PC's browser will expose the output from the target device's app.

Now open another ssh session in a separate terminal and run:
```
cd /var/
./netbird_connection_watcher
```

Finally, on your host PC, open a new tab in the browser and type:
```
http://127.0.0.1:8080/
```
to observe the netbird connection status of the target embedded Linux device.

# Creating an .ipk package (ADVANCED USAGE)
There are several ways of creating a package for OpenWRT-based devices. I have used a derivative of OpenWRT SDK which is why this guide will be a general description of how to cross compile and include `netbird-connection-watcher` package into the final image. To create such a package that fits nicely into the OpenWRT SDK, make sure your file structure looks like this:

```
.
├── files
│   └── netbird-connection-watcher.init
├── Makefile
└── src
    └── main.c
```

Keep the source code (`main.c`) same as it's laid out in this repository and copy the following files into your build toolchain:

`netbird-connection-watcher.init`:
```
#!/bin/sh /etc/rc.common

USE_PROCD=1

# Start value of netbird-connection-watcher should be greater than the start value of netbird by at least 1 
START=100
STOP=10

NAME=netbird-connection-watcher

start_service() 
{
    	# wait until netbird is actually responding
    	while ! /usr/bin/netbird status >/dev/null 2>&1; do
        	sleep 2
    	done
	procd_open_instance
	procd_set_param command /usr/bin/netbird-connection-watcher
	procd_set_param respawn
	procd_close_instance
}
```

and
`Makefile`:
```
include $(TOPDIR)/rules.mk

PKG_NAME:=netbird-connection-watcher
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/netbird-connection-watcher
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  TITLE:=NetBird connection watcher
  DEPENDS:=+netbird
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/netbird-connection-watcher \
		$(PKG_BUILD_DIR)/main.c
endef

define Package/netbird-connection-watcher/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netbird-connection-watcher \
		$(1)/usr/bin/netbird-connection-watcher

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/netbird-connection-watcher.init \
		$(1)/etc/init.d/netbird-connection-watcher
endef

$(eval $(call BuildPackage,netbird-connection-watcher))
```
Once you have these files in your SDK, update the package feeds and install the `netbird-connection-watcher` package by running the following in your SDK directory:
```
./scripts/feeds update -a
./scripts/feeds install netbird-connection-watcher
```
which registers the `netbird-connection-watcher` package into the build system. Then, run:
```
make package/netbird-connection-watcher/compile V=s
```
to actually build the package called `netbird-connection-watcher_1.0-1_<target_architecture>.ipk`. The resulting package is now placed inside the SDK's build directory for the specific target architecture you have build the package for. 

You can now copy this package into the build system's root, include it in the menuconfig (with the source location of the package's configuration parameter being defined in the Makefile of the package (`Network` -> `VPN` -> `netbird-connection-watcher`)).

Finally, from the root directory of the build system run:
```
make
``` 
which will produce the final image with the `netbird-connection-watcher` package included into the build.

*NOTE: you might now see the `network-connection-watcher` process being ran and active when you check the running processes on your device because there the procd init system quietly gives up on bringing this process up if the app cannot execute `netbird status -d` or bind to port 8080. In this case, the latter is the reason for this behavior.The app doesn't have a bulletproof check of the netbird client actually being up and running and being connected to Netbird's central Management Server since the idea of creating a package like this was to demonstrate how to create custom package that you can include in the final image.*

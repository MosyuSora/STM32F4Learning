# 让100ASK_IMX6ULL支持adb

把adbd_files下的文件，全部复制到开发板根目录：

```shell
cd adbd_files
cp * / -rf

dos2unix /lib/udev/rules.d/61-usb-adbd.rules
dos2unix /usr/bin/usb_config
dos2unix /etc/init.d/S99adbd

chmod +x /usr/bin/adb
chmod +x /usr/bin/adbd
chmod +x /usr/bin/usb_config
chmod +x  /etc/init.d/S99adbd
```



Ubuntu上安装adb：

```shell
sudo apt install adb
sudo chmod a+x /usr/bin/adb
sudo chmod a+s /usr/bin/adb
sudo adb kill-server
```



然后就可以在Ubuntu上使用adb操作开发板了：

```shell
adb devices
adb push 1.txt /root      # 把Ubuntu的文件放到开发板的/root目录
adb pull /root/1.txt 2.tx # 把开发板的/root/1.txt下载并改名为2.txt
adb shell                 # 登录开发板
```


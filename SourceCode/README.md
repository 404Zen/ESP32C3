# ESP32C3 Clock

## 更新说明

| 版本号 | 说明                         | 备注                                  |
| ------ | ---------------------------- | ------------------------------------- |
| 0.0.3  | 1.添加AP名称显示与LAN IP显示 | 1.NTP同步程序需要优化，后续版本优化。 |
| 0.0.2  | 1.Fix bug #3、#4             |                                       |
| 0.0.1  | 1.初始版本                   |                                       |



## 使用说明

1. 使用 smartconfig 对设备进行配网。

2. 申请天气接口的 app_key 

   请到和风天气申请API，以及获取自己城市相应的location id, 具体操作参考开发文档。

   https://dev.qweather.com/

3. 使用控制台输入 location_id 和 key

   `weather_param location_id key_xxxxxxxxxxxxxxxxxxxx`
   这个命令参数比较长，可以在外部编辑器编辑完成后一次性粘贴进去。



##	需要先解锁GPIO11才能控制背光

[Link](https://wiki.luatos.com/chips/esp32c3/board.html#spi-flash)

ESP32C3的GPIO11(VDD_SPI)默认功能是给flash供电，这个开发板（能不能烧看原理图）VDD直接接3.3，所以可以将此IO用作GPIO，以下是操作流程，注意以下的操作只能执行一次，更改后不能复原（因为是设置熔丝位，不是寄存器，一次性操作）。
1、使用python的pip安装esptool。pip install esptool
2、将开发板插入电脑
3、打开命令行窗口输入espefuse.py -p 端口 burn_efuse VDD_SPI_AS_GPIO 1
4、看提示，输入'BURN'



## https gzip decode

[Link](https://yuanze.wang/posts/esp32-unzip-gzip-http-response/)



## bug list

- #1.https在读取response的时候，极小概率会出不来读数据的while循环，导致后面无法获取到新的天气数据。可能与网络环境有关，需要添加超时异常处理，未处理。ret = 0xFFFFFFB0.

- #2.console初始化可能导致wifi相关的功能异常，如无法连接到网络，smartconfig失效等。可能与当前网络环境有关，未复现。

- ~~#3.新设备通过 console 首次写入weather parameters 会导致重启，但是数据会保存到flash中，重启后可以正常使用...(原因是新设备在没有读取到参数时不会创建对应的事件组和任务，导致 assert failed. 待修复)。~~

- ~~#4.新设备配网完成后，没有进行对时.(smartconfig任务在会清除事件标志。)~~


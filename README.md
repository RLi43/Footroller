# Footroller
HCI device using feet motion factor. 

esp32 - WiFi - Python

## 19/6/2记录

* adc是时延的主要原因 一个100ms太久，故而换了模拟开关来读取8个电阻。响应比较快，而且本来对压力的精度要求就不是很高
* 重写了python的接收，还是写成类比较方便。由于使用者的脚大小不一样，把8个电阻降到3个区域（上，左/右，下）的维度判断

## 苦逼的前言
因为想连单片机和各种智能设备，用了dfrobot的esp。esp原装没货买了ble版本（客服跟我说要两三个月的！一个月不到就上了！），虽然本来对ble就不是很熟，但是买都买了。。。

鼓捣几天觉得实在不行，ble的api在不同平台上都要重新开发。。。windows提供的sample又各种bug（或许我这里环境不太对。。）。所以，最后还是用WiFi吧。。[TCP](https://blog.csdn.net/Naisu_kun/article/details/87125845)

然后C#的socket不知道为什么收不到esp发起的连接。行吧，既然要通用，就用python吧。[pyautogui](https://github.com/asweigart/pyautogui)

但也有一些问题待解决：
* 一个没有输入设备的外设怎么设置连接……
  * 设备端开一个蓝牙advertisement告诉它吗…… 听起来解决方案一点都不简洁……
  * 还是留个串口让用户自己初始化一下吧！
* python界面 唔 要学

## 整体构思

* MCU ===》 PC
  * 运动数据 （《==先做这个
* PC ===》 MCU
  * *休眠/节电
* ？
  * MCU怎么知道WiFi信息……
    * 增加交互设备/方式 - 串口通信 - 顺便可以调一下参数
    * 复杂的ble广告[BLE Scanner](<https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/>)

## 思考记录

传感器灵敏度

###### 传输的选择

让单片机作运算再传数据

* 性能

还是单片机只传数据，接收端处理。

* 一些无用的数据（无变化数据的重复
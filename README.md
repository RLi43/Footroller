# Footroller
HCI device using feet motion factor. 
esp32 - WiFi - Python

## 苦逼的前言
因为想连单片机和各种智能设备，用了dfrobot出品的esp。esp原装没货买了ble版本（客服跟我说要两三个月的！一个月不到就上了！），虽然本来对ble就不是很熟，但是买都买了。。。

鼓捣几天觉得实在不行，ble的api在不同平台上都要重新开发。。。windows提供的sample又各种bug（或许我这里环境不太对。。）。所以，最后还是用WiFi吧。。[TCP](https://blog.csdn.net/Naisu_kun/article/details/87125845)

然后C#的socket不知道为什么收不到esp发起的连接。行吧，既然要通用，就用python吧。[pyautogui](https://github.com/asweigart/pyautogui)

但也有一些问题待解决：
* 一个没有输入设备的外设怎么设置连接……
  * 设备端开一个蓝牙advertisement告诉它吗…… 听起来解决方案一点都不简洁……
* python界面 唔 要学

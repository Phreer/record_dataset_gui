# 人体运动数据采集界面程序
本应用以 Qt5 为平台, 用于采集人体手势数据. 调用的外设有普通摄像头, Intel Realsense F200, 基于 Android 的智能手表, Myo.

## 环境
- Windows 10 64bit
- Qt Creator 4.5.2
- Qt5
- Intel Realsense DCM F200 1.3.27.52404
- Intel Realsense SDK
- Android Studio 2.3
- Android SDK API Level 25

Realsense SDK 可以在[这个地址](https://pan.baidu.com/s/1ufNhnLiya_17Mp17_p514w)下载.

OpenCV 可以在[这个地址](https://pan.baidu.com/s/11nCLgG5aUTlNQXdQmWc2DA)下载.

关于 OpenCV 和 Qt 的配置可以参考[这篇文章](https://phreer.github.io/2018/03/28/setup_opencv_and_basic_usage.html)

## 关于 Myo 的配置
Myo SDK 同样包含了 lib 文件和 include 目录, 配置方法与 OpenCV 相似. 唯一不太一样的地方是需要**把动态链接文件放到生成可执行文件的目录下**.
Myo 和 PC 连接需要安装连接程序, 可以通过[这里](https://pan.baidu.com/s/1HyzBLp25o5lQCWQOWfDi4A)下载.

## 关于 smart watch
我们所使用的 smart watch 基于 Android 平台, 因此采用 WiFi 进行通信. 基本的思路类似于 FTP, 使用两个套接字, 一个用于发送控制命令(开始记录, 结束记录, 传输数据等), PC 作为 Client, smart watch 为 Server. 另一个用于接收数据, PC 为 Server 接受 smart watch 的请求, 该部分使用一个单独的线程一直运行.

## 其他说明
- 网络编程部分使用 winsock2 api, 可能需要安装 Visual Studio 才能使用, 我不太确定, 我使用的是 VS2015. 其实 Qt 本身提供了网络编程 api, 或许更容易使用, 但因为我之前是在 VS 上面写的, 所以用了 winsock.
- 对应的 Android 端程序请见[这个项目](https://github.com/Phreer/getAccelearationRound).

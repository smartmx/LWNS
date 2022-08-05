# LWNS v1.61 Release

结合例程和《LWNS开发指南.pdf》学习使用LWNS。

注意根据不同的功能进行修改宏定义。

## ch5xx_LWNS_Template为LWNS例程

只要在RF_Init函数中将rf配置为相同参数，

启用加密的话，lwns_sec.c中的秘钥lwns_sec_key也配置相同，

即为同一组设备，适合用来学习使用，也可以直接用于开发。

## 配网

LWNS只要RF参数一致即在同一个网络中，无需其他配网操作。
可以自行添加配网流程，例如：

1. 蓝牙配网
2. USB/UART配网
3. 未配网设备广播，网关扫描后配网

基于LWNS编写的应用层程序是通用的，不同芯片只需移植好LWNS即可使用相同的LWNS应用层程序。

## 技术支持

有问题给以发送邮件至`mx@wch.cn`，或者联系沁恒官方技术支持。

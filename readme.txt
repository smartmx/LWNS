一定要结合《LWNS开发指南.pdf》一起学习。
有问题给以发送邮件至 mx@wch.cn ，或者联系沁恒官方技术支持。
注意根据不同的功能进行修改宏定义。

ch57x_LWNS_Template为LWNS无配网功能例程，
只需要在RF_Init函数中将rf配置为相同参数，
启用加密的话，lwns_sec.c中的秘钥lwns_sec_key也配置相同，
即为同一组设备，适合用来学习使用。


ch57x_LWNS_Prov_Template为LWNS具有配网功能例程，
1.支持未配网设备通过配网设备进行配网
2.支持手机蓝牙对设备进行配网，通过rwprofile进行参数设定
初次启动默认RF_Init配置为rf_config_params.c中的lwns_rf_params全局变量，
秘钥为lwns_sec.c中的秘钥lwns_sec_key。
通过配网可以修改rf参数和秘钥，保存到flash中。
同时具有蓝牙连接功能，可用于开发参考。




1、Src_150420: 初次创建

2、Src_150430：完成GPS数据解析, CDMA短信发送。

3、Src_150505：完成debug命令添加，CDMA模块本机号，接收号的设置。设备类型和设备编号的设置等。

4、Src_150518：测试代码，不发送CDMA信息
               1、按照运行流程，以5分钟发送为例，3分钟待机，2分钟系统工作（GPS和MCU），1分钟通讯模块工作
               2、添加了运行时间指令，发送:AA 55 0A CC 33运行时间清零，开始测试；
                  发送AA 55 0B CC 33 读取系统已经运行的时间。
                  注意只能在系统唤醒期间操作。
                  
5、Src_150911：中间调试版本，主要测试各个模块的工作情况。

6、Src_151019：增加北斗接收机号码设置功能；
               增加设备公司编码功能，由之前的16位ID号变为目前公司编码16位，设备编码16位；
               
7、Src_151030：完成全流程设计。
               因目前版本不需要对设备进行远程设置，则系统进入待机后可将CDMA模块直接掉电。
               目前进入待机整机功耗小于1.7ma，一天待机功耗在40mah左右。目前使用6000mah@8.4V，
               则待机功耗对于整机的整体功耗影响较小。
               
8、Src_151117：完成基本要求。下一版本需要更改通讯协议。

9、Src_160113：没改管脚前最后一版
               
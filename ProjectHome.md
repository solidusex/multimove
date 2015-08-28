> 由于工作原因，我经常要在多台电脑上联调程序，比如Client-Server或者多级中心等等，导致桌面上键鼠混杂凌乱不堪，所以我抽空开发了这个小软件，本程序通过网络使用一套键鼠控制多台机器，并且能共享剪贴板(暂时仅支持文本复制)。希望能给大家带来方便。

  * MultiMoveClient.exe是主控端，此程序运行在装有键鼠的PC上，通过MultiMoveClientLoader.exe启动。
  * MultiMoveServer.exe是被控端，此程序运行在没有键鼠的PC上。
  * 本程序使用VS2010编译，需要安装vc10的运行时库，我在Downloads栏上传了一份。
# MediaPlayer

是一个基于SDL2，FFmpeg3.2的跨平台视频播放器，若要自行编译SDL和FFmpeg请自行编译，然后替换相关*.so、*.dll、*.lib即可,具体参照${Project}/app/CMakeLists.txt

##如何导出？
* Android Studio 2.2
* * 根目录即为AndroidStudio工程，直接导入即可
* Visual Studio 系列
* * 用CMake3.4.1以上版本，源目录为${Project}/app，导出即为VS工程
* * 打开VS后，将调试的工作目录更改为${Project}/app/src/main/winLibs/${x86/x64}/bin，方便调试
* * 发布程序，需要将main.exe和工作目录下的所有*.dll放至同一目录，然后发布
* Linux系列
* * 敬请期待

##有问题反馈
在使用中有任何问题，欢迎反馈给我，可以用以下联系方式跟我交流

* xiangwencheng@outlook.com
* QQ: 289728630
* 或直接GitHub上新建Issue.
* 问题描述越细致越好

##关于作者
    向文成，感谢张钰雯提供的SDL2.so和FFmpeg相关*.so
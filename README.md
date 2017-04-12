# MediaPlayer

是一个基于SDL2，FFmpeg3.2的跨平台视频播放器，若要自行编译SDL和FFmpeg请参考其他文献，然后替换相关.so,.dll,.lib即可,具体参照${Project}/app/CMakeLists.txt

## 如何导出？

* Android
* * 安装Android Studio2.2.3
* * 根目录即为AndroidStudio工程，使用AndroidStudio2.2+直接导入即可

* Winodws
* * 安装VisualStudio2010+，CMake3.4.1+
* * 用CMake3.4.1以上版本，源目录为${Project}/app，导出工程（x86、x64可选，x86好像是默认的）
* * 用VisualStudio打开生成的工程后，将调试的工作目录更改为${Project}/app/src/main/winLibs/${x86/x64}/bin已便调试时能够找到相关*.dll
* * 发布程序需要将生成的exe和工作目录下的所有*.dll放至同一目录，然后发布

* Linux系列
* * 敬请期待（还是使用CMake,CMakeLists.txt中添加支持）

## 有问题反馈
在使用中有任何问题，欢迎反馈给我，可以用以下联系方式跟我交流

* xiangwencheng@outlook.com
* QQ: 289728630
* 或直接GitHub上新建Issue.
* 问题描述越细致越好

## 关于作者
* 向文成
* 感谢张钰雯提供的SDL2.so和FFmpeg相关*.so

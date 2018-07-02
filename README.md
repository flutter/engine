Flutter Engine
==============

这个fork将记录分析Flutter Engine源码的原理.
剖析Engine从启动那一刻,到加载运行dart文件,并在屏幕上渲染控件的整个过程.

Engine是整个Flutter UI框架的核心实现,它基于skia 2D图形引擎,引入了Blink的RenderText库.
再加上DartVM,共同构成了Flutter这个框架.

从Flutter这个项目创建的过程来看,似乎是Chromium浏览器项目的成员,似乎对日益庞大的Chromium,感到并不是很满意.
开始一个重复造轮子的过程,他们从Chromium中的Blink,去除了CSS解析,HTMLParser这些模块,只保留了核心的渲染的实现.
尝试构建了一个高性能的2D渲染UI库.

但是Chromium这些人可能也太过厉害了,竟然抛弃了C++实现,引入了新的Dart语言.
按照他们选择Dart语言的情况来看,仅仅是因为Dart语言团队就在他们身边,他们可以随时沟通.
有时候任性,也是一种实力的体现.

不过据八卦来看,可能不仅仅是这样.
最近Google和甲骨文因为Java的官司太多. Flutter UI框架,也被钦定为新的Fuchsia OS的图形框架.
全部从头造轮子,确实符合Google一贯的作风,但是推行新的OS,阻力太多.

就目前Flutter UI框架而言,因为是采用AOT编译,和目前的React Native支持动态JS不同,
没办法做到很好的热更新,每次App有新的改动,必须重新发布版本,上架.很明显已经不符合Hybird App的趋势了.

所以这个Fork,也有一个小小的目标,希望能在深入理解整个Flutter UI框架的时候,
能将Dart对UI控件的实现,翻译成C++,然后基于C++版本,再通过V8/JavascriptCore引擎绑定,暴露JS版本.
提供一个支持热更新的Hybird App的Flutter-JS框架.

===============
[![Build Status](https://travis-ci.org/flutter/engine.svg)](https://travis-ci.org/flutter/engine)

Flutter is a new way to build high-performance, cross-platform mobile apps.
Flutter is optimized for today's, and tomorrow's, mobile devices. We are
focused on low-latency input and high frame rates on Android and iOS.

The Flutter Engine is a portable runtime for hosting
[Flutter](https://flutter.io) applications.  It implements Flutter's core
libraries, including animation and graphics, file and network I/O,
accessibility support, plugin architecture, and a Dart runtime and compile
toolchain. Most developers will interact with Flutter via the [Flutter
Framework](https://github.com/flutter/flutter), which provides a modern,
reactive framework, and a rich set of platform, layout and foundation widgets.


_Flutter is still under development and we continue to add 
features._ However, it is ready for use by early adopters who are willing to deal
with the odd wrinkle or two along the way.  We hope you try it out and send
us [feedback](mailto:flutter-dev@googlegroups.com).

 - For information about using Flutter to build apps, please see
   the [getting started guide](https://flutter.io/getting-started/).

 - For information about contributing to the Flutter framework, please see
   [the main Flutter repository](https://github.com/flutter/flutter/blob/master/CONTRIBUTING.md).

 - For information about contributing code to the engine itself, please see
   [CONTRIBUTING.md](CONTRIBUTING.md).
   
 - For information about the engine's architecture, please see
   [the wiki](https://github.com/flutter/engine/wiki).

Community
---------

Join us in our [Gitter chat room](https://gitter.im/flutter/flutter) or join our mailing list,
[flutter-dev@googlegroups.com](https://groups.google.com/forum/#!forum/flutter-dev).

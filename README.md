
本库包含通用机器智能系统用到的一些基础项目。

<hr/>
通用机器智能系统（General Machine Intelligent System,GMIS）,目标是成为一个通用的机器人大脑，它允许用户用自己的母语，以类似自然语言的方式，编程控制任何具有现代操作系统的机器（包括不限于：个人PC，手机，智能家电，智能汽车、工业机器或军事武器等等）产生逻辑行为，并在适当时候实现机器人自主编程。它将有助于诸如ASIMO这样的硬件机器人实用化。

GMIS将以众筹+开源的方式发展，任何人可以通过为此项目做出的贡献按比例分享知识产权，详情请访问：[gmis.github.io](http://gmis.github.io/)

目前GMIS已经得到一个原型，试用请[下载](https://github.com/GMIS/Download)。

这里是原型的演示视频:[(中文)](http://pan.baidu.com/s/1mglwzmo)，它演示了在GMIS的逻辑控制下自动玩一款在线德克萨斯牌游戏。

目前GMIS原型还谈不上什么用户体验，仅用于验证理论。所以强烈建议试用原型之前，请耐心看完此视频，并阅读附带的说明手册，你也可以单独[下载手册了解](https://github.com/GMIS/Download)。

所有用户可用的资源,比如可供GMIS使用的外部物体，都可以在这里[下载](https://github.com/GMIS/Download)。

<hr/>
整个GMIS被分解成几个部分,每个部分又包含几个编译工程，放在不同仓库里，它们的关系如下:<br />

__GMIS库概览__
* [GMIS_BASE](https://github.com/GMIS/GMIS_BASE) 本仓库存放GMIS所需的基库工程<br />
> TheorySpace （lib）： GMIS的根基类<br />
> BaseSpace  (lib)：实现一些抽象运算<br />
> UserSpace_Win (lib)：依赖操作系统的某些实现，这里特指windows<br />
> UserSpace_boot (lib)：同UserSpace_Win，但基于boost，理论上可以跨平台同用<br />

* [GMIS_SPACE](https://github.com/GMIS/GMIS_SPACE) 本仓库存放有关虚拟空间的项目
> SpacePort (exe)： 虚拟空间入口程序<br />
> VirtualSpace (lib)： 虚拟空间基库<br />
> VisualSpace (lib)： 可见空间的基库（GUI）<br />

* [GMIS_OBJECT](https://github.com/GMIS/GMIS_OBJECT) 本仓库存放有关外部物体的项目
> Executer (exe...)：  外部物体的调用程序，分各种版本<br />
> Objects...(Dll...)： 各种外部物体Project<br />

* [GMIS_BRAIN](https://github.com/GMIS/GMIS_BRAIN) 本仓库存放基于各平台的大脑实例项目
> Brain (lib)： 实现一个大脑的基本框架<br />
> GMIS_WIN (exe)：基于WINDOWS个人PC的大脑实例<br />
   
* [GMIS_SHARE](https://github.com/GMIS/GMIS_SHARE) 本仓库存放与GMIS非直接相关的项目
> WIN3Party (lib) 收集到的其他第三方代码集合<br />

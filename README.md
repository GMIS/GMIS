__GMIS_BASE__

This repository including some basic projects used for GMIS.

<hr/>
General machine intelligence system (Hereafter "GMIS") is aimed to became a general robot brain which allows user using themselves mother tongue with similar natural language talking way to program control any machines that have had a modern operating system(including not limited to: PC, phone, smart appliances, smart car, and industrial machine or military weapons etc) to produce logic behavior, and in appropriate time to achieve robot independent programming. It will contribute to the practicability of hardware robot like ASIMO.

GMIS plans to develop by crowd+ open source mode, anyone can make contribution to GMIS and share the profit , please visit：[gmis.github.io](http://gmis.github.io/)

GMIS has currently  got a prototype: [download](https://github.com/GMIS/Download)。

Here is the prototype demo video:[(45 minutes)](https://player.vimeo.com/video/113364683 )，It demonstrates how to automatically play a online Texas card game under the GMIS logic control.

The GMIS  prototype,  which is far from user experience, is only used to verify the theory. Therefore we strongly recommend that users watch the video and read the manual before using prototype.

<hr/>
GMIS as a whole is broken down into several parts, each part contains several projects stored in different repository, their relationship is as follows:<br />

__GMIS Library Overview__<br />

* [GMIS_BASE](https://github.com/GMIS/GMIS_BASE) This repository stored some basic projects used for GMIS<br />
> TheorySpace （lib）： The root classes of GMIS<br />
> BaseSpace  (lib)：Implementing some abstract operations<br />
> UserSpace_Win (lib)：Some implement rely on the operating system, here especially Windows<br />
> UserSpace_boot (lib)：Same with  UserSpace_Win, but based on boost，in theory, it can be used across platforms<br />

* [GMIS_SPACE](https://github.com/GMIS/GMIS_SPACE) This repository stored some projects of virtual space
> SpacePort (exe)： Virtual space entry application<br />
> VirtualSpace (lib)： base lib of virtual space <br />
> VisualSpace (lib)： base lib of visual space（GUI）<br />

* [GMIS_OBJECT](https://github.com/GMIS/GMIS_OBJECT) This repository stored projects of external object
> Executer (exe...)：  Caller of the external object, there are various versions based on specified os<br />
> Objects...(Dll...)： various external object projects<br />

* [GMIS_BRAIN](https://github.com/GMIS/GMIS_BRAIN) The repository stored  brain  instance  projects based on  specified platform
> Brain (lib)： a basic brain framework<br />
> GMIS_WIN (exe)：The brain instance based on Windos OS<br />
   
* [GMIS_SHARE](https://github.com/GMIS/GMIS_SHARE) This repository stored some projects that are not directly related to GMIS 
> WIN3Party (lib) The collections collected from other third-party code<br />

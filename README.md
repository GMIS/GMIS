
*The first general robot operating system with Turing-complete  in the world.*

__Introduce__
<hr/>


General machine intelligence system (Hereafter “GMIS”) is aimed to became a general robot brain which allows user using themselves mother tongue to  programmatically control any machines (including not limited to: PC, phone, smart appliances, smart car, and industrial machine or military weapons etc) to meet their individualized needs. Currently, it helps robot like ASIMO more practicability, finally, through robot autonomously programming, it will be able to achieve strong AI.

We have got a prototype. Here is the prototype demo video: [(45 minutes)](https://player.vimeo.com/video/113364683 )，It demonstrates how to automatically play a online Texas card game under the GMIS logic control.

The GMIS  prototype,  which is far from good user experience, is only used to verify the theory. We plan to develop GMIS by crowd+ open source mode, anyone can make contribution to GMIS and share the profit , for details please visit：[gmis.github.io](http://gmis.github.io/)


__Source Code Overview__<br />

<hr/>
GMIS as a whole is broken down into several parts, their relationship is as follows:<br />


* \GMIS_BASE_LIB includes some basic projects of GMIS<br />
> AbstractSpace （lib）： some abstract root classes of GMIS.<br />
> PhysicSpace  (lib)：some implement of abstract classes rely on  operating system.<br />
> VisualSpace (lib)：a very lightweight GUI lib based on Windows system for test purpose.<br />

* \GMIS_ROBOT includes stored brain instance projects based on specified platform.
> Brain (lib)： a basic robot brain framework.<br />
> GMIS_WIN (exe)：the brain instance based on Windows system.<br />

* \GMIS_WORD includes some projects for virtual space  where robot will connect each other.
> VirtualSpace (lib)： base lib for SpacePort. <br />
> SpacePort (exe)： virtual space entry application.<br />



* \GMIS_OBJECT includes projects of external object and their excuter. 
> Executer(exe...)：  caller of external object, there are various versions based on specified OS.<br />
> Objects(Dll...)： various external object projects.<br />

   
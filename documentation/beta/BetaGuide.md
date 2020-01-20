#Reality 3 for Poser Beta#
Welcome to the beta test of Reality 3 for Poser. 

**Please read this document carefully. If you don't do that you might lose data and possibly entire scenes that you have spent hours in creating. Questions and reports that have answers in this document will be ignored.**

Here are a few notes about how to use this beta and on how to work in a beta-test environment. Please forgive me for putting a few dry concept at the beginning, it's important that the way this beta test is conducted is clear for everybody, in order to avoid misunderstandings.

Some companies started doing "open betas" in the past few years, therefore producing the perception that beta testing is a sort of informal way of getting a "sneak peek" at a certain program. That is not the case with Prêt-à-3D and Reality. We take beta testing very seriously and we use it to obtain the highest quality for the software that we release. 

I am personally very, very grateful that you have decided to help us. Without testing "in the field" it is not possible to obtain a clear picture of how the software performs in the real world. 

We always work to make your effort worth the time and energy invested and that will be the case for this beta program too. 

##A few simple rules##
So, here are the things to keep in mind during this testing phase.

- This is a **closed** beta program. This means that under **no circumstance** any detail about Reality 3 for Poser beta can be discussed outside the group of testers. No posting in the forums, no discussion with other people, no showing of images generated with Reality 3 for Poser. In fact it will be best if you don't even mention that you are a beta testers. 
- If you find bugs in Reality you are kindly asked to report the bug using the BTS (Bug Tracking Software). Please do not send me emails about it. Solving bugs relyes on the ability to track them. Email is inadequate at this task. The BTS is the right tool for the job. See below for more details.
- Please keep in mind that this is beta software. It will crash and it will chew up your data. Please, please, please do not use Reality for Poser with valuable Poser scenes. Use new scenes that you can trash without causing valueable loss of data. 

OK, now that I scared the bejusus out of you let me give you the good news. The software is actually in pretty good shape, or so I hope, but there might be a few things to be adjusted in the storage system and that might require to erase the stored Reality data. This will be stabilized pretty soon and I will let you know right away when Reality will be usable for normal production.

##Installation##

###Installing LuxRender
Without LuxRender Reality is not going to be of much use. You must install Luxrender first, and you can find it at `http://luxRender.net`. Use version 1.2RC1 or the official version 1.2 if it's available. Windows users need to decide if they want to install the OpenCL version or not. That depends on the GPU that you have. Please consult the manufacturer of your GPU if you are in doubt. Mac users don't have to worry about this because OpenCL is supported directly by the OS and so there is one version of LuxRender.

Once LuxRender is installed please try to launch it just to verify that it runs on your machine. Once it runs simply close it. If there are problems at this stage examine your system and check if you have the right version of Lux. 

###Installing Reality
At the current time I don't have an automated installer for Reality, but the installation of the program is simple. Please note that, for simplicity, I use forward slashes even when describing files for Windows. So a path will be something like `Runtime/Geometries/Reality` even under Windows. You will need to replace "/" with "\".

- Reality is distributed with zipfiles. 
- There is one one zipfile for Mac OS that contains a Universal version of the program, both 32 and 64 bits.
- For Windows there is one zipfile for the 32-bit version and one for the 64-bit version. Select the one that applies to your system. If you use the 32-bit version of Poser you must use the 32-bit version of Reality and so on.
- To install Reality simply unzip the zipfile and a directory named "Reality" should be produced.
- Make sure that Poser is not running.
- Move the Reality directory to be inside your installation of Poser in the subdirectory `Runtime/Python/addons`
- Assuming that you have Poser installed in the default location you will end up with either one of the following directories, in the case of Poser Pro:
	- `/Applications/Smith Micro/Poser Pro 2012/Runtime/Python/addons/Reality` (Mac)
	- `C:/Program Files/Smith Micro/Poser Pro 2012/Runtime/Python/addons/Reality` (Windows)
- There a re couple of Reality-specific add-ons. These are lights are props that will be expanded in future drops. They are jusy standard Runtime Poser object so you can add them to your Runtime as you do for other propducts.
- Start Poser and check under the Render menu is you have a new option called "Reality Render editor". If it's there then Reality is working as expected. If it isn't there then enter a bug report.

###Video tutorial
At this point you can start a new scene and use the video at the [video tutorial prepared for the beta](http://www.preta3d.com/videos/private/R3BetaIntro.mov) test to see how to produce a render with Reality.

#What we are going to test
Since this is the very first test outside the development environment the scope of the test is very limited. There are several things that are not working and it's superfluos, at this stage, to report them. What we need to verify is based on a few questions:

- Does Reality run on your machine?
- Does Reality recognize the installed LuxRender once you point out where it is?
- Can you render a scene with Reality and Lux?
- Can you set material properties?
- Can you use the supplied lights?

Please report any problem in the BTS. On the other hand, if all tests are passed on your machine please send me a short email letting me know. That is all that we are going to check for this first test run. 

Please note that the conversion of skin is not completely accurate and some texture packs might result with unnatural skin colors. No need to report those yet, work on fine tuning the conversion in under way.

##Data storage
While using Reality you might wonder where its data is saved, since the program never asks for saving your data before exiting. At any time all the data is kept under the Poser side of Reality and it will be saved in your Poser scene if you decide to save it to disk.

##Reality's memory
You will nerice that Reality remembers the position and size of its window. This means that the next time that you will be runnign Reality it will come up exactly in the same place and with the same window size of when you last closed it. It remembers also the last tab that you used. 

##Documentation
Reality has benn always very well documented bu thte guide for version 3 is not ready yet and that means that you will have to lear the program the hard way. I apolofize for this. The RUG 2.1 (Reality User's Guide) is included and can be opened form the help tab, by clicking on the button labelled "Reality User's Guide."

That manual is for the previous version and the it refers to Studio. Reagrdless, a lot of concepts apply to Reality 3 for Poser as well. As soon as I can I will produce a new guide updated for this version.

# How to report problems#

The BTS is on the web at the URL: `http://preta3d.com/mantisbts/`
You will need to create an account for it. Once you create the account I will get notified and enable it for the Reality 3 for Poser project. Please note that the BTS handles other projects as well. If you already have a BTS account please send me an email with the name of that account and I will enable it for this project. 
The BTS requires full names for every reporter.

Once you have access you will see a page with a few links at the top. To enter a report click on "Report Issue". The fields should be self-explanatory. If you need help please do a Google search for "Mantis BTS." There is plenty of documentation online. Mantis is the name of the BTS program that we use.

Once you found a problem in the program it is important to report it in a way that can help me to reproduce it, so that I can solve it. To do this you need to provide step-by-step instructions that lead to experiencing the bug as you found it. Here is an example:

- Start a new scene
- Add the standard V4 figure
- Apply the skin texture XYZ
- Call Reality
- Crash with error <error message here>

This is an example of how not to report an error:

- Every time I use the default Reality lights the program hangs.

The problems in the above description are: "default lights" is a vague term. Are those mesh lights? The sun? spots? It is important to be precise. The program "hangs" doesn't tell us when it happens, what was the action that preceeded the crash/hang. Step-by-step is the mantra here. You have to think of it as guiding someone to reproduce your crash... using the telephone. 
It's also important that you specify the OS that you use. There are presets for this under the "Select Profile" dropdown menu. Let me know if your configuration is missing.

##Log files
Reality creates a few logfiles to help with the debugging. In your Documents folder look for the following:

- Reality_plugin_Log.txt
- Reality_IPC_GUI_log.txt
- Reality_IPC_Server_log.txt

The first one is of main interest, it contains the bulk of debug and error messages. If something goes wrong look at the bottom of that file to see if there are error messages. 
The other two show are low-level debug information about the communication system of Reality. Reality works on a message-passing system that communicates between the Reality window and the side of the application that runs inside Poser. We call those two sides the GUI-side (Graphic User Interface) and the host-app side. At any time the GUI sends and receives information from the host-app side. If something goes wrong I might ask you to send me those log files. Once Poser and Reality are not running it's perfectly safe to delete those files. They will be recreated with the next run of Reality.

The two IPC (Inter-Process Communication) files will be removed as soon as we establish that Reality is running without problems for everybody.

#Expiration
This beta will stop working on March 3rd. At that time you will receive a new beta.

##Known problems
The following issues are known and should not be reported:

- Starting a new scene in Poser (Cmd-N/Ctrl-N) when Reality runs crashes Reality
- Several UI elements are misaligned
- Occasionally some messages appear in the "mainWacros" window. Please ignore them for now.
- SSS doesn't appear correctly in the material preview. If you want to see a better preview disable SSS temporarily and enable it back before rendering.
- Displacement doesn't appear in the material preview

#Notes for users of Reality 2#
If you used Reality 2 you will notice a few UI changes. First of all the Reality window is not modal (non-blocking) so you can work on your Poser scene while keeping Reality opened all the time and switch back and forth between the two programs. The Reality if a full-fledged application, not a plugin, so it can be found in your OS' task manager.

You will noticed tha the output tab is gone. It's actually transformend into the "Render" tab, which is now the first of the group. The Render tab also contains the Render button. A few options are not present at this time in Reality 3. Given what's happening with SLG (SLG4), and the new SLGRender system of Lux, it makes no sense to develop for that platform at this time. Reality 3 will be updated to support SLGRender as soon as it makes it in the official distribution of Lux.

You will also notice that ACSEL is not present. That is going to be the status for the shipping version. ACSEL, and ACSEL share will be introduced in a service release later on. 

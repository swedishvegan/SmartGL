# SmartGL
OpenGL made easy.

#### Demo video: https://youtu.be/zDuNxg4LJ18 (sorry for low-quality recording)
#### For an example of how my engine is used, please read through the demo.cpp file for the demo video's source code.
#### Contact me: mas83674@uga.edu

### About SmartGL
> What the hell is this?

SmartGL is at its core two things: (1) a collection of object-oriented wrappers around OpenGL objects designed to boost productivity and ease of programming while working with OpenGL, and (2) a graphics engine designed primarily with the goal of making video games. No one likes having to write acres of OpenGL API calls for the most simple tasks or carefully look up OpenGL functions one at a time in the spec. My object-oriented wrappers were made to preserve all the fun and power of OpenGL programming while eliminating all the boring and tedious stuff. Some features include:
- Classes for every type of commonly used texture with support for (pretty much) every data type.
- Easy interface for loading shaders and creating programs.
- Classes that provide an easy interface for setting a program's uniforms and keeping them all bundled in one place. In particular, uniform buffers with an std140 layout can be updated automatically without the user having to perform meticulous alignment calculations. This one was truly a game-changer for me.
- My own math library. You can think of it as a slightly worse, less extensive version of the popular GLM library. But it's single-header and still gets the job done. (If you want to have nightmares tonight, go look at the source code for "util/GL-math.hpp"!)

The graphics engine is what I spent the bulk of my time on, and includes the following features:
- Loading of models from pretty much any format.
- Support for animated and PBR models.
- Custom model format that minimizes loading times.
- Custom PBR shaders that are generated on the fly for any type of model that is being rendered.
- Integration of background skyboxes into a PBR scene.
- Overhead and point-light shadows.
- Integration of Bullet physics.
- A relatively easy-to-use interface.
> Ok cool, but like, why would I ever use this? Why not just use Unity or something?

Great question, my imaginary friend! Honestly, no reason -- you're most likely better off using Unity. This was just kind of a passion project for me. I personally like having full control over the GPU and I take pride in knowing that the games I develop are running off of code that is entirely my own. But if your goal is to maximize your game development productivity, probably go with a more well-established game engine. But just in case you would ever want to use a small, compact, quirky game engine made by a passionate developer, you now have the option to do so.
> How can I learn more about the engine and how to use it?

I want to add Doxygen comments to all my code eventually, but for now, there is no documentation. Your best bet is to look through the "demo.cpp" file and read the comments/ try to understand the code. It's pretty simple and probably covers most common use cases. You can also look at the source code for each file; I organized the headers so that the class declarations are always at the top of the file for reference. If you have any additional pressing questions, you can email me (see top).
> How would I go about including my library/ building the demo?

Including the library is easy enough, but you'll need to also include all of its dependencies. This is hard, and you can expect to spend a couple days getting it up and running. See the bottom for a list of dependencies. In particular it is quite a hassle to get the Bullet physics library up and running, so if you want to use my code without physics integration, simply define "SmartGL_NO_PHYSICS" before including SmartGL. Also note that SmartGL does not internally use any window system or OpenGL function loading library, so you'll have to provide libraries for these utilities yourself (I use the combination GLEW/GLFW).
> Not that I care, but can you tell me a little bit about yourself?

Well that's a little rude, but sure. At the time of writing this, I'm a 20 year old college student going into his junior year. I've been coding and making games as a hobby since middle school, and now I'm a math/CS major. I spend a lot of my free time programming and it allows me to express myself on a level that I would never be able to in everyday life. This project of mine is a very important part of my life and I spent a solid eight months toiling over it. I'm posting it here in the hopes that someone may find it useful one day or at the very least appreciate the work that I've put into it. I also love playing guitar, listening to music, hiking, and probably other stuff that I'm forgetting.

> Okay cool. Also, why are you having a conversation with an imaginary person? Don't you have a life?

No comment.

### List of dependencies:
- The stb_image library (single-header).
- Bullet physics (only required if physics models are used).
- Assimp (required to convert models into SmartGL's custom format, but you can write a separate program to do this so that you don't need Assimp to build your main game's executable).

---
title: Raylib over OpenGL for a Gravity Sim??
date: 2026-02-11
language: 'en'
tags: ['Blog', 'C++', 'Raylib']
draft: false
description: Building a Gravity Simulator using Raylib instead of OpenGL
---

A Gravity Simulator. A simple application that allows you to throw bodies of celestial size at each other and see how they orbit each other.
Let's go build one!!

So in order to build one, we need to pick an engine that allows us to draw graphics on the screen. Let's weigh out our options here for a moment shall we?

- **Unity:** Be real, we've all seen this engine thanks to so many video games that we play in today's age
- **OpenGL:** For extreme masochists that want full control over everything that they touch
- **Raylib:** Man I just wanna do something and not suffer with the installation process or fight with the machine for anything

This post isn't about how or why Raylib is better than other Libraries/Applications that make graphical applications, but more on how I felt like Raylib was the right choice for me to build a very basic gravity simulator without losing years off my life.

# Initial Thoughts

I wanna have the power of the Solar System in my hands. Can I do it in C++ instead of using learning C# just to use Unity?

I had two options sitting in front of me, one being Raylib and the other being OpenGL. I had heard the term OpenGL a lot when playing video games and all I know about it is that it's a library that allows me to access the GPU and gives me full control over the pipeline.
Sure, why not? Lemme go through the rite of passage and write this.
My first blocker was the installation, cause what do you mean I need to install OpenGL, then download a zip file from another website to install GLAD and then include that in the main file manually???

By the time I actually figured out what to do, the Sun had risen and I had just spent the entire night staring at the screen losing my mind.

At this point, I just thought about learning something I've never heard before in my life, and I decided to open the Raylib website, and let me tell you that I felt like I had ascended into a different plane.

# Enter Raylib
Looking at the examples of Raylib felt like someone took the graphics and infrastructure part, and stuffed it away so that you just worry about the camera, objects and their interactions.

All I had to do was write 10 lines of code and I already had a working window that I could later use to render stuff.

```
int main() {
    InitWindow(800, 450, "I have created a universe container");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Behold!!! pixels", 260, 200, 20, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
}
```
Look at that. 10 Lines, and there's a screen that has coloured pixels on the screen. From here all I had to do was replace text with a few spheres, add a 3D Camera and giving them velocity. The rest was math and that's about it.

I spent close to two hours just going through their [examples page](https://raylib.com/examples.html) in order to see how hard it actually is, and let me tell you, those two hours I spent sitting and rewriting the examples made me understand that raylib is pretty simple to use.
After some time of just sitting with the examples and the cheatsheet, I had a window, a render loop and shapes on my screen. This was so much more progress than OpenGL and Unity combined (I didn't use Unity at all).

# The Documentation
The Documentation needs an entire section for itself. That's all I want to say about it. It's so well written and so developer friendly. You don't have to hunt down other programs and their documentation to understand anything.

For everyone who get distracted easily, the entire examples page is completely interactive, so you can learn how the Raylib API works while playing games in the browser itself.

The examples are so clean and polished, it's a simple 50 line code snippet in order to make a 2D platformer. One file, no bloat, no fluff. You can run them instantly, understand it very easily, and my personal favourite, ~steal~ get inspired by techniques shamelessly.

This low barrier for experimentation makes it so easy to get into simple graphics development. It manages to hit the right middle ground for people who wanna do very simple graphics while having control. You don't fight for your machine like how you OpenGL and you don't fight for system resources like how you Unity.
It gave me enough abstraction to move fast, do the math quickly and enough control to stay engaged.

So next time you wanna make a simple graphics related project, have a look at [Raylib](https://raylib.com) and remember that you don't have to fight demons in order perform graphics programming.

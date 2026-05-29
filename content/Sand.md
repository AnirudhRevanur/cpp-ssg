---
title: Psychological warfare on myself so I stop Doomscrolling
date: 2026-05-29
language: 'en'
tags: ['Blog', 'Kotlin', 'Android']
draft: false
description: I break the Geneva Conventions so I don't end up Doomscrolling for hours
---

I broke the Geneva Conventions in order wage a psychological war on myself so I stop doomscrolling and brainrotting. Unlike other Android Apps that bore a hole into my wallet or are so easy to break apart, mine actually adds enough friction for you to get up and deviate yourself.

This is Part 1 on how I punish myself to get back into healthier habits by reinventing the wheel in the form of Android Apps.

# The Problem
Doomscrolling has become one of my favourite pastimes now. I average around 4-5 hours a day just mindlessly scrolling and smoothening out my brain everyday.

I have tried using the in-built timer on the app, but there's a button right there that allows you to snooze the alarm for 5 minutes or 15 minutes. Kinda counter productive if you ask me.

Using the Android Digital Wellbeing was good, it was strict and blocked the app after the set time, but on the other hand it was also pretty simple to snooze. Just a few clicks, reach the Settings page and you're good to snooze.

There are other apps on the Play Store which I have tried, but they were mostly apps that were behind a paywall of some sort with the free versions doing absolutely nothing

Being the developer and problem solver I am, I chose to build my own solution that works in my way.

# It's Different. I Promise
Here's the deal I made myself. You're not gonna go Cold Turkey, but wanting to bypass comes with additional work. If you're willing to put the work, then you get to doomscroll as a reward.

You get 30 minutes for free per day. After that you hit a wall. To keep scrolling, you have to answer my riddles 3. Here's the catch though:

The troll that gives you these riddles 3 lie on your laptop, so you'll have to switch on your laptop, then turn on the API endpoint. No simple button press on your phone to snooze, no quick bypass.

It works because you have to go to a different device, open a terminal, then start the server. Once that is done, you get a handful of math questions based on how long you were doomscrolling that day.

Solve all the answers correctly in a row, and you get **5** full minutes of scrolling before you're stopped again. Oh also! The questions are completely randomized, so you can't just memorize your way through. The questions keep getting progressively harder, so at one point you'll be spending more time answering the questions than you get to scroll.

What made me sure that this works better than a few other solutions is that it forces you to do something else in between, thus throwing you off your balance. My favourite part? It forces you to break the easy dopamine fix by making you move around. My guess is that most people don't want to come back to scrolling cause they got a fix of dopamine and something else to do now that their laptop is on.

# Two Phase Architecture
Unlike other solutions, this one works on a Two-Phase system. We've spoken about how you gotta go switch on your laptop, but how does it work?

We'll talk about the API first. It's a simple API Server that's written in Python with the help of FastAPI. It has a simple file structure looking like this

```
.
|-main.py # The main program tying everything together
|-sessions.py # Keeps track of sessions like how cookies do
|-generator\
  |-level1.py # Make random questions for level 1
  |-level2.py # Make random questions for level 2
  |-level3.py # Make random questions for level 3
  |-level4.py # Make random questions for level 4
  |-level5.py # Make random questions for level 5
  |-__init__.py # Makes this module callable from main.py
```

So since each level has different requirements, I felt it was cleaner to keep the individual functions in different files (Yes I'm a React Developer how could you tell?)

Now when the API endpoint is on, running at port 8000, we can hit a few endpoints being `/sync`, `/generate`, and `/verify`
`/sync` => The phone tells how long a particular app has been used.
`/generate` => It generates a full problem set based on the level (screen time)
`/verify` => With the help of the session token, it verifies if your answers are right or not

The Android App requires a few special permissions which does make it look suspicious, but trust me I am not doing anything sus with it at all.

When you first open the app, you are asked to give it permissions, and after that it shows how long you've spent scrolling on that app. To add salt to the injury, there are also messages that shame you just sitting there on the home page of the app, (These are based on your screen time btw).

Now when you reach 30 minutes, every time you try to open the doomscrolling app, it redirects you to this overlay saying that you've spent enough time. If the laptop is not reachable, then you go switch it on, and then you enter the IP Address of the laptop in the text field available on screen. Once that's done, it displays the questions to you and you can answer them.

# Finishing Thoughts?
I felt like there was a real disconnect in between how solutions look at blocking screen time, and how it really has to be done to make it more effective. But then again, what will work for me won't work for everyone.

If you are really interested in learning more about the app, go here: [Sand-Android](https://github.com/AnirudhRevanur/sand-android), and if you want to see the API endpoint, go here: [Sand-API](https://github.com/AnirudhRevanur/sand-api)

It's free, open source, you can make your own forks and make it work in your favour, or change the colour scheme or add a different kind of friction, and all you need is to use the CI/CD releases to get yourself an apk that can be sideloaded onto your phone with no issues.

And that folks is Sand. It's coarse, rough, irritating and gets in the way when you're trying to doomscroll.

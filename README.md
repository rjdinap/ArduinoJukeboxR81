Some preface: I don't have any nostagia about Jukeboxes. I've never actually seen one working, in person. But I do remember my parents playing records. So when I had the chance to buy a fairly cheap "project" R81 jukebox, I took the plunge. “I want a jukebox”, I thought. I further mused, “It would look cool in my basement that I'm never going to have time to finish because I keep buying projects.” Speaking of projects... this is my first Arduino project. And a bit of an ambitious one at that!

So here's the project: Control the insides of an AMI R81 jukebox with an Arduino. 

We're going to control:
The phono mechanism with the encoder, including toggle shift coil to play the flip side of the record.
The original keypad.
The play counter.
The 'Make any selection' and 'Record playing' lights.
The led display.
A built in Mp3 player that can house thousands of songs.
Front Panel lighting via RGB lights.

It should function mostly similar to a real jukebox. Queue up songs via entering them in the keypad, and it picks them off the queue and plays them in order - it doesn't matter if the player is in mp3 mode or controlling the physical phono mechanism. The mp3 player has the advantage of using 'folders', so you can have 1 folder for Oldies, 1 folder for Rock music, 1 folder for Christmas, etc. And you can change folders via a keypad command.

Compared to the real R81, We have lots of new commands available, such as: 
Turning on random play mode.
Sequential play mode.
Viewing the encoder postion in real time.
Changing the volume via the keypad (mp3 mode).
Changing the front panel lighting via the keypad.

And let's be clear. This isn't a restoration - it's a hack. A wonderful hack that saved this box from going into a landfill, but still a hack. Had I picked up something more substantially beautiful than an R81, I may have put more effort into a restore using real parts. You can read about the full build in the PDF in the repository.

Be sure to read the Readme-Jukebox.pdf file. You can also use the Readme-Jukebox.odt file (OpenOffice format) if you don't trust opening the .pdf

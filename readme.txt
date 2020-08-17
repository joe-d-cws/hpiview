
HPIView Source Notes

This program was compiled with Microsoft Visual C++ 5.0.

It's written in straight C - no C++ required.

However, it does require a knowledge of the Windows API.  An excellent
place to start is "Programming Windows 95" by Charles Petzold.

If you plan to modify the program, use GetMem and FreeMem to allocate and
free memory.  You can use malloc or GlobalAlloc if you want, but using 
GetMem and FreeMem will reduce memory fragmentation.

I started making the program able to decompile COB files, but never
finished it.  The hooks are there, all you really have to do is the
actual decompiling.  See the viewcob.c file for details.

I've also added hooks for viewing 3DO files, but never got any farther.

If you want to make the program view other file types:
1. Add an appropriate constant to viewstuf.h
2. Check for the appropriate extensions in the FindFileType function
   in HPIView.c
3. Write an appropriate viewing function ('ViewXXXFile') and place it in
   its own source file ('ViewXXX.c').  Add this to the project.
4. Call this function from the ViewFile function in HPIView.c
   The ViewFile function will have already decompressed and decoded the
   file for you.  Just pass the parameters to your routine in the same
   way that they're passed to the other viewing routines.

The view function should return zero if it is unable to view the file
for some reason, otherwise return non-zero.

The simplest viewing function is the ViewTextFile function in ViewText.c. 
You should be able to look at that one to figure things out.

If you have any questions, just ask.

And if you DO make changes, I'd appreciate a copy of them.

Joe D
joed@cws.org

# DELIVERY INSTRUCTIONS

--------------------------------------------------------------------------------
## HOW TO DELIVER A NEW RELEASE ?

A delivery is made of two files:
* binary archive
* source archive

It is possible to use CMake for building such a delivery. When you invoke 'cmake', 
you have to specify the version number like this:
    
    cmake -DMAJOR=3 -DMINOR=4 -DPATCH=11 ..

Then, you can use a 'delivery' target with:

    make delivery

You can do the whole stuff on a single line:

    cmake -DMAJOR=3 -DMINOR=4 -DPATCH=11 ..;  make delivery
    
    
This target will both produce the binary and source archives and then upload them on 
the GForge server.

You can have the list of available targets for delivery with:

    make delivery_help

--------------------------------------------------------------------------------
## HOW TO KNOW THE EXISTING RELEASES ?

There is a target for this:

    make delivery_dump

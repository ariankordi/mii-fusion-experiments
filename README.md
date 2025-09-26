# mii-fusion-experiments
This is some code related to Mii characters in the [Fusion programming language](https://github.com/fusionlanguage/fut), which theoretically allows reuse of simple code in many other languages.

My end goal is to hopefully be able to write as much shared logic as I can in Fusion, and eventually be able to work with Mii data and render them in as many environments as possible.

However, this is not that.

These are just some tests whose goal is to prove how much functionality can and can't be implemented in Fusion. What you see below are kind of like the building blocks needed to eventually reimplement something like [FFL](https://github.com/aboood40091/ffl).

### Building...
Yeah, Fusion is kinda funky here. You'll want to [start by downloading a release of fut.](https://github.com/fusionlanguage/fut/releases)

Fusion doesn't have a concept of imports, everything is just built into one file. (!!! which I hope changes). So you glob all of the .fu source into one file in your target language.

You're also not encouraged to build your whole program in Fusion. You can have a `Main()` function and read argv, but you can't currently open files. Fusion just wants you to make library code, and MAYBE some small tests that run in the Main() function.

For each of these samples I'll have some build instructions. All other languages should? be straightforward unless I didn't test it. **One notable exception** is that, you should add your language as a definition, e.g. `-D PY`, `-D C`, `-D CPP`... Because some code targets a specific language and Fusion doesn't let you know this inside of the code yet without adding that def.

If you build for C and it says ***glib.h file not found***, then you'll need to include+link glib like so:
```sh
gcc `pkg-config glib-2.0 --cflags --libs` main.c # after fut -> main.c
# Fish shell: add "eval" at the beginning and replace backticks with ()
```

### Ver3StoreData (2025/04/07)
Sample that decodes and re-encodes 3DS/Wii U format Mii data using a `Ver3CharInfo` class.

```sh
# Build for Python
fut -l py Ver3CharInfo.fu Ver3StoreData.fu  Crc16Ccitt.fu Utf16ToUtf8Converter.fu Main.fu -o main.py
python3 main.py
```

### NxResource (2025/05/11)
Reads a Switch Mii shape resource file (ShapeHigh.dat/ShapeMid.dat, from title ID 0100000000000802/"MiiModel"). Sorry, this is probably borderline impossible to acquire yourself. Definitely don't ask me personally, and I definitely won't provide it to you for testing.

Takes a single shape type/ID and uses an embedded `GLBExporter` class to export it to a model.

```sh
# Build for C
fut -l c -D C NxResource.fu GLBExporter.fu KTX12Header.fu -o NxResource.c
gcc `pkg-config glib-2.0 --cflags --libs` `pkg-config zlib --cflags --libs` -g NxResource.c ShapeMain.c -o ShapeMain
./ShapeMain ShapeHigh.dat # default shape emitted to shape.glb
./ShapeMain ShapeMid.dat faceline9.glb 1 9 # type: faceline (1), id: 9 -> faceline9.glb

# Build for JS.
fut -l js -D JS NxResource.fu GLBExporter.fu KTX12Header.fu -o NxResource.js
node ShapeMain.mjs ShapeMid.dat shape.glb # emits default hair 123
# May complain about not having zlib, idk didn't test works on my machine Lol!
```
Then view it in something like this: https://gltf-viewer.donmccurdy.com and Admire.

(I also tried to implement textures at some point by exporting KTX but uh didn't work before I gave up)

### Mask (2025/05/12)

This is it. The last one of the trifecta needed for a Mii library: Data parsing, resource reading, and mask drawing.

The "Mask" in Mii rendering is the texture that contains most of the facial features. The coordinates of each part are calculated and turned into quads, which are drawn into a texture and multiple mask textures are used for each expression.

So what did I implement- pretty much nothing.

In May I had reimplemented [this one function, CalcMVMatrix](https://github.com/aboood40091/ffl/blob/73fe9fc70c0f96ebea373122e50f6d3acc443180/src/FFLiRawMaskParts.cpp#L72-L86). But I plan to reimplement a larger amount soon from my own decompilation of nn::mii, so expect something NEW here eventually!!!!!


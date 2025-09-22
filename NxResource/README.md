# NxResource/nn::mii shape resource reader and exporter in Fusion

The reason this reads the Switch format in particular, is because there was DWARF info for me to copy in nnmiiResource.h- I mean, because it’s simpler than the FFL resource. 😅
- (Note that this format uses 16 bit half-floats for vertex data, compared to the FFL resource that uses 32 bit floats which are more compatible and less lossy. So I don't actually think it's better, just simpler.)

There’s a "frontend" to this in C and JS. I couldn’t get C++ working due to some build error related to std::format?

After making this, I had three goals for myself. First, is texture support by exporting KTX/2 (and potentially reimplementing simple Tegra swizzle).
* Then, I wanted to come up with an interface to abstract access to all Mii resources.
    * It would also contain stuff that would allow you to get things like the vertex format, etc.
    * Something that inspired me to think about this is that I also made a Wii RFL resource reader toy (granted, using Kaitai) in May.
      - On GitHub Gist (need to update this with the new Kaitai, arrghh): https://gist.github.com/ariankordi/15c713e1208d7a5d534152dc276bab4a
    * Some resources require decompression and some don’t. FFL uses deflate, nn::mii uses zlib.
        * However, since they both use deflate, I also briefly thought about doing deflate in Fusion. [For instance, see how simple it could be.](https://github.com/nayuki/Simple-DEFLATE-decompressor/blob/master/cpp/DeflateDecompress.cpp)
    * This is something that'd be good to have unit tests for. Vertex and texture formats differ between most all resources.
      - Wii/3DS use uint16 attributes. 3DS/Wii U/Switch has swizzling. Wii/DS(?????) does not have indices, the rest do.

Here's a copy-paste from my Apple Notes about that abstraction:

<details>

- [ ] Resource: Add table of vertex attribute formats and add each attribute in a loop
- [ ] (rough draft of resource polymorphism methods)
- [ ] Element/part info (ResourceCommonAttribute)
    - [ ] Offset, Size
    - [ ] CompressionMode = NoCompression, Zlib, Brotli
        - [ ] (ffl window bits: middle = 4 (zlib 12), high/poster = 5 (zlib 13)), ASSERT IF NOT ANY OF THOSE STATING we hAVE NOT SEEN RESOURCE WITH THOSE
    - [ ] CompressedSize(=undef&assert)? StoredSize(=size/compressed)?
    - [ ] Get Shape, Texture
- [ ] Shape attribute/info (ResourceShapeAttribute)
    - [ ] AttributeOffset/Size
    - [ ] IndexOffset/Size (SHOULD they be separate?????)
    - [ ] BoundingBox (rfl/cfl: NaN)
    - [ ] 👉 VertexFormat\[attribute\]???? HOW IS WII GOING TO WORK AAAAAA
- [ ] Methods
    - [ ] Get Shape, ShapeFaceline, ShapeHair
        - [ ] ShapeFaceline: shape + faceline transform
        - [ ] ShapeHair: shape + hair transform
- [ ] Texture attribute/info (ResourceTextureAttribute)
    - [ ] Width, Height, Alignment(?), MipCount
    - [ ] Format (our enum / expand nnmii with CFL and RFL's)
        - [ ] FFL: R8, RG8, RGBA8 / nn mii has compressed
        - [ ] TODO rfl: GXTexFmt enum, dolphin tex decoding / I4 (brow), IA4 (glas), RGB5A3 (eye mouth facetex)
        - [ ] TODO cfl: tiling / L4, A4 (same?), LA4, RGBA4
    - [ ] TileMode (= Linear, Gx2Default, NxOptimal, CtrDmp, GcnWiiArtX (should be no ? )
        - [ ] Get Texture
    - [ ] LittleEndian - Only needed here. Not for info or texture, and impl already knows endian
- [ ] METHODS?
    - [ ] GetShapeLocInfo(header, type, index) + GetTextureLocInfo(header, type, index)
    - [ ] GetShapeAttribute(header, type, index, partData) -> ffl/header, nnmii/struct
        - [ ] Faceline (no type), Hair (no type)
    - [ ] GetTextureAttribute(header, type, index, partData) -> ffl/footer, nnmii/struct
- [ ] TODO???
    - [ ] list all possible texture and vertex formats?
    - [ ] how to read wii primitives...??? index format = RflPrimitiveArray?
    - [ ] shape conversion. how to abstract this?????
        - [ ] wii: total conversion
            - [ ] create index buffer
            - [ ] adjust normal and texcoord
            - [ ] SCALE POS=128 (32768/256), UV=4 (32768/8192)
        - [ ] short/half->float, 10 bit->8888 or float
    - [ ] header in memory, content in file...
        - [ ] 3ds/wii has no separate shape header - make our own (just offsets)?
        - [ ] maybe have a mode that does not store a header copy?

</details>

* I’m not super happy with the way the GLBExporter class is.
    * I wanted to avoid using the dynamic List type and ideally have statically allocated lists for everything, avoid heap allocation.
    * I would also want to see if I could avoid string formatting. This is a tall ask, but the goal of this and no dynamic lists would be to not require GLib/C++ STL. It may be possible by treating the string as bytes and "copying" the bytes, IDK.
    * We should provide vertex data as raw bytes instead of trying to interpret it as float, etc. inside the encoder. (since Fusion can't read floats from bytes natively and I had to make this awkward function for it)
    * (would it make more sense to use an external glTF lib? 🤔 but I love if it’s all native...)

Another copy-paste from my Apple Notes:

<details>

- [ ] fusion GLBExporter cleanup
    - [ ] make attributes abstract
    - [ ] enums: semantics, vertex formats
    - [ ] table of byte sizes, semantic strings
    - [ ] -> later on: do not use List types
        - [ ] 1: add pointers + sizes, copy to final glb later
        - [ ] 2: append to buffer directly: buffer should be offset to a fixed length for max json length, and then json is added before it and copied to file
            - [ ] requires CUSTOM IMPLEMENTATION method to expand buffer (realloc?
        - [ ] 3: commit buffer size (assert if not filled), add attributes + make json, then append buffers after
    - [ ] other gltf exporter references
        - [ ] https://github.com/PicelBoi/pvr-model-extractor/blob/74c7a16cd596d2442bc110513e04035dc4ed38de/GLB/GLBExporter.py
        - [ ] c# but using json library (?????): https://github.com/Cognitics/Inception/blob/5a7de4b623c31834d321bd01916f06a39ff108e8/Assets/Cognitics/glTF.cs#L12

</details>

I remembering wanting to continue, but early on I showed this to Jo who used it to make [Vee](https://github.com/j0lol/vee), and ended up spending all of my time for the rest of May answering their daily texts about questions and progress, eventually forgetting my old goals.

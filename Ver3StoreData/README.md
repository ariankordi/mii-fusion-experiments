# Ver3StoreData decoding sample in Fusion

That's my preferred name for the 3DS/Wii U Mii data format. Second most popular name is FFLStoreData. But, given it has at least 4 different names and the latest one is `nn::mii::Ver3StoreData`, that's what we'll go with.

This includes `Ver3CharInfo` which represents decoded Mii data, and `Ver3StoreData` that decodes into CharInfo, to have a "common" format many can decode to.
* I have this phobia of creating another _container_ format that Nintendo didn't make and would like to avoid it at all costs. If you make a class or struct that's a "lowest common denominator!!!!!", like, that's what I want to avoid. It's a little irrational.
  - But, using _Ver3CharInfo_ (represents CFLiCharInfo, FFLiCharInfo...) sounds fine to me. Then we can have an NX version that only supports Switch features, convert between the two which handles colors as well.

## Challenges I face with this

I was unhappy with how this whole thing worked and didn't continue due to indecisiveness.

### Code Generation

* How can I make this magic bitfield getter/setter code without guesswork? From a struct definition?

Ideally, there should be "one source of truth" for the struct definition, then a way to parse the definition and work with it. I've thought about: plain C headers/using DWARF info, then [Kaitai Struct](https://github.com/kaitai-io/kaitai_struct) and [ImHex Pattern Language](https://github.com/WerWolv/PatternLanguage). All have pros and cons to them, and is a deeper discussion than this.

All of the following have pros and cons, but for this specific example I used [struct-fu (my fork)](https://github.com/ariankordi/struct-fu) to convert the C definition to it (using AI) and use offsets from it - just because it was closest to C structs and easiest to play with.

What I ended up was asking AI to make the snippet below, then running it on [this jsfiddle](https://jsfiddle.net/arian_/dvnym03t/3/) containing the struct-fu struct I want and feeding it back to the AI. That worked, to make this specific snippet.

<details>

```js
/**
 * Generates pure JS encode/decode functions (as strings) that read/write one byte at a time.
 * @param {Object} struct - A struct-fu instance with .fields and .size.
 * @returns {{ decode: string, encode: string }}
 */
function generatePureJS_noDataView(struct) {
  const decodeLines = [];
  const encodeLines = [];

  // Decoder function start:
  decodeLines.push('function decode(buffer) {');
  decodeLines.push('  // Assume buffer is an ArrayBuffer; create a Uint8Array view.');
  decodeLines.push('  const bytes = new Uint8Array(buffer);');
  decodeLines.push('  const result = {};');

  // Encoder function start:
  encodeLines.push('function encode(obj) {');
  encodeLines.push(`  // Create a new buffer with the exact struct size (${struct.size} bytes).`);
  encodeLines.push(`  const buffer = new ArrayBuffer(${struct.size});`);
  encodeLines.push('  const bytes = new Uint8Array(buffer);');

  // Iterate each field in the struct
  for (const [key, field] of Object.entries(struct.fields)) {
    // If field.offset is a plain number (byte-aligned field)
    if (typeof field.offset === 'number') {
      // If we have a multi-byte field, build it byte by byte.
      if (field.size && field.size > 1) {
        // --- Decode multi-byte field:
        decodeLines.push(`  { // decode field: ${key}`);
        decodeLines.push(`    let val = 0;`);
        decodeLines.push(`    for (let i = 0; i < ${field.size}; i++) {`);
        decodeLines.push(`      val |= bytes[${field.offset} + i] << (8 * i);`);
        decodeLines.push(`    }`);
        decodeLines.push(`    result.${key} = val;`);
        decodeLines.push('  }');
        // --- Encode multi-byte field:
        encodeLines.push(`  { // encode field: ${key}`);
        encodeLines.push(`    let val = Number(obj.${key}) || 0;`);
        encodeLines.push(`    for (let i = 0; i < ${field.size}; i++) {`);
        encodeLines.push(`      bytes[${field.offset} + i] = (val >> (8 * i)) & 0xFF;`);
        encodeLines.push(`    }`);
        encodeLines.push('  }');
      } else {
        // Single byte field
        decodeLines.push(`  result.${key} = bytes[${field.offset}];`);
        encodeLines.push(`  bytes[${field.offset}] = Number(obj.${key}) & 0xFF;`);
      }
    }
    // If field.offset is an object, we assume it's a bitfield.
    else if (typeof field.offset === 'object' && field.offset.bytes !== undefined) {
      const byteOffset = field.offset.bytes;
      const bitOffset = field.offset.bits || 0;
      const width = field.width;
      const mask = (1 << width) - 1;

      // --- Decode bitfield: read a 32-bit word one byte at a time.
      decodeLines.push(`  { // decode bitfield: ${key}`);
      decodeLines.push(`    // Read 4 bytes starting at offset ${byteOffset}`);
      decodeLines.push(`    let word = (bytes[${byteOffset}]      |`);
      decodeLines.push(`                (bytes[${byteOffset}+1] << 8)  |`);
      decodeLines.push(`                (bytes[${byteOffset}+2] << 16) |`);
      decodeLines.push(`                (bytes[${byteOffset}+3] << 24)) >>> 0;`);
      decodeLines.push(`    result.${key} = (word >>> ${bitOffset}) & ${mask};`);
      decodeLines.push('  }');

      // --- Encode bitfield: read-modify-write a 32-bit word.
      encodeLines.push(`  { // encode bitfield: ${key}`);
      encodeLines.push(`    // Read the existing 32-bit word at offset ${byteOffset}`);
      encodeLines.push(`    let word = (bytes[${byteOffset}]      |`);
      encodeLines.push(`                (bytes[${byteOffset}+1] << 8)  |`);
      encodeLines.push(`                (bytes[${byteOffset}+2] << 16) |`);
      encodeLines.push(`                (bytes[${byteOffset}+3] << 24)) >>> 0;`);
      encodeLines.push(`    // Clear the target bits`);
      encodeLines.push(`    word = word & ~((${mask}) << ${bitOffset});`);
      encodeLines.push(`    // Set the bits from obj.${key}`);
      encodeLines.push(`    word = word | ((Number(obj.${key}) & ${mask}) << ${bitOffset});`);
      encodeLines.push(`    // Write back the 32-bit word, one byte at a time`);
      encodeLines.push(`    bytes[${byteOffset}]     = word & 0xFF;`);
      encodeLines.push(`    bytes[${byteOffset}+1]   = (word >> 8) & 0xFF;`);
      encodeLines.push(`    bytes[${byteOffset}+2]   = (word >> 16) & 0xFF;`);
      encodeLines.push(`    bytes[${byteOffset}+3]   = (word >> 24) & 0xFF;`);
      encodeLines.push('  }');
    }
  }

  // End functions:
  decodeLines.push('  return result;');
  decodeLines.push('}');

  encodeLines.push('  return bytes;');
  encodeLines.push('}');

  return {
    decode: decodeLines.join('\n'),
    encode: encodeLines.join('\n')
  };
}

// Usage example:
// Suppose FFLiMiiDataCore is your struct-fu instance.
const generated = generatePureJS_noDataView(FFLiMiiDataCore);
console.log("Generated decode function:\n", generated.decode);
console.log("Generated encode function:\n", generated.encode);
````

</details>

### API, Code Size

* Do I want pack/unpack functions, or, getter/setter "accessor" methods?
  * Having getters/setters for all fields is more flexible.
    - This would actually be required for [NxInVer3Pack](https://github.com/ariankordi/ffl_mii_patcher_plugin/tree/main/effsd).
  * However, I want the JS output to be tiny.
  * Solution: inlining. It can either be done manually, or if you don’t call the individual funcs then hopefully a smart inliner will optimize them down anyway. Right?

Code size here was really a make it or break it moment personally. I reeeeeally wanted this to be better than if someone had just written this by hand in JS. I always thought that getters/setters add bloat.

Unfortunately, Fusion at the time enforced that fields were private. This meant that when using `tsc` to transpile to JS, it inserted these "polyfills" to enforce the private fields.
- Piotr Fusik actually removed this by my request in June 2025, but I haven't revisted this since.

If I removed the private fields, then at least _Closure Compiler_ was able to flatten down all of it. But then I thought, how could I elegantly include an intermediate build step to find-replace `#` to `_`? Would I need something like Babel? I didn't go anywhere with this.

### Read bytes or words?

* Currently, the way this reads bitfields is to read just the bytes that contain the bitfield.
  - If you had a function that accessed bitfields in C, and decompiled it with Ghidra, you got output that did exactly this.
    * This makes completely "universal" code you could copy-paste in any language, since it's just reading byte-by-byte, with bit operations.
    * See where I actually did this to port struct encoding/decoding to JS: https://gist.github.com/ariankordi/e6e66b8b03b1424d6e4e489fd9dd83bf
  - This has two pitfalls though: Endianness, and performance.
    * (You actually need to mark the struct as "packed", alignment 1, and sometimes make the bitfields volatile, in order for the compiler to emit single-byte reads)
    * I'll also say performance in this was never ever a big deal to begin with, but prevents this from being "hyperfast" and only being "pretty fast".

#### Reading per-byte: Endianness
* Mii data structs are tightly bit-packed and all that, cool, whatever. But, if you look at the actual C definitions, you'll see that the bitfields are contained within larger types such as u32 and u16.
* For something like Kaitai that doesn't care about which type the bitfields are in, well, this is fine if the bitfields and data are always one endianness.
* But, it gets more complicated if you want to read big and little endian with the same code.
* You see, they wanted to be able to convert between one endian and the other, by swapping each word that the bitfields are in. However, bitfields still follow a different order.
* So, if you look at an (accurate) struct for Ver3StoreData, [such as in Abood's FFL decomp (FFLiMiiDataCore)](https://github.com/aboood40091/ffl/blob/812c3ffeabfac501032a5fc6c289e8402b69dc7c/include/nn/ffl/FFLiMiiDataCore.h#L649-L964), you will see that they reverse the definition order of the bitfields for the other endianness. Oh boy.
* The result is that, you effectively need to read the word that the bitfield is contained within in the _desired endianness_, then read the bitfield itself? in _little-endian_. Reading the specific bytes that the bitfield happened to be contained within, will not work at all for this.

#### Reading per-byte: Performance
* If you read bitfields in this way, by reading individual bytes, it'll actually never get optimized into multi-byte reads.
  - (Or at least clang didn't do this. Maybe it can't assume the alignment?)
* Reading the whole word is more optimal than just the individual bytes, and supports both endiannesses as you read above.

#### Challenges when reading words in Fusion

So what's the problem, why don't we just do that?

* Reading more than a word is not standardized in programming languages.
  - All CPUs have instructions like "load 8 bit", "load 16 bit", "load 32 bit"...
  - For most langs it would've been as simple as calling a function, except for JavaScript. You have to construct this `DataView` class, so it's not stateless anymore.
  - Also removes the ability to copy-paste the output code into any other language that's not Fusion, since accessing bytes and doing bitshifts was universal. But that's not even the main goal.
* Fusion itself doesn't have primitives to read 16-bit or 32-bit numbers.
  - It seems that the author of Fusion, Piotr Fusik, did this in his code by reading each byte and ORing them together.
  - This approach isn't elegant and bloats the code to do inline, it's also endian-specific so there has to be two functions for both.
  - But it will get inlined by the compiler/JIT into either a full load, or a load and swap for the opposite endianness.

#### Proposed solution: custom class

I thought about making a static class that'd have these 16-bit/32-bit read primitives. Then, I realized that state is needed:
* Storing the bytes and offset would be necessary.
  - Fusion actually doesn't let you take a "slice" of an array, so if you pass an array in, you need to also accept the offset, because they could be taking a slice of some bigger file or something.
* We need that DataView for JS.
* If you want to read the opposite endian, having a flag for that here would be a good place.

So, what I have in mind currently is a "Pointer" class that has read primitives on it and gets passed by reference. I currently haven't tried this.

I am worried, though, that having this class and state would hold up how calls of it can be inlined. I also originally wanted no dependencies, or very little.
Hopefully if I do the class, it'll either be very tiny, or able to be easily inlined in JS and C.

### Final conflict: Test Driven Development

I'd been doing unit tests at work for a little while now, and I thought that the ultimate Mii library should absoultely have this. I was really passionate about seeing if I could reach 100% coverage.

* Well, Fusion doesn't have its own testing framework.
  - You could write tests individually in your language, sure...
  - ... but don't you need to make sure the code builds and works in all languages????? Not hitting undefined behavior, or memory errors?

* While Fusion doesn't have its own test framework, [fut actually does have unit tests.](https://github.com/fusionlanguage/fut/tree/master/test)
  - The tests are all single files with one function, and linked to this "Runner.fu". I don't know what to think about this... 😭

More importantly, I was scratching my head at how to get coverage from this - how do you know that your Fusion code is covered? If it builds to another language, you really have to see how much of your test file is covered. But if you're importing the whole library, well, you have to only cover certain... functionality..... I'm confused on how to continue with this.

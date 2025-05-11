import { createInflate } from 'zlib';
import { ZlibInterface, ResourceShapeHeader, Main } from './NxResource.js';
import fs from 'fs/promises';

export class ZlibInflatorImpl extends ZlibInterface {
	constructor() {
		super();
		this._inflate = createInflate();
		this._ended = false;

		/** @type {Buffer|null} */
		this._input = null;

		this._inflate.on('end', () => {
			this._ended = true;
		});
	}

	/** @param {number} windowBits */
	setWindowBits(windowBits) {
		// Not supported in Node.js zlib streaming API, so we ignore it.
	}

	/** @param {Uint8Array} input @param {number} offset @param {number} length */
	setInput(input, offset, length) {
		// Slice creates a view, no copy — just like in Fusion.
		this._input = input.subarray(offset, offset + length);
	}

	/** @param {Uint8Array} output @param {number} offset @param {number} length */
	inflate(output, offset, length) {
		if (!this._input) throw new Error('No input set');
		const slice = this._input;
		this._input = null; // Clear input after use

		let written = 0;
		try {
			const result = this._inflate._processChunk(slice, this._inflate._finishFlushFlag);
			if (result.length > length) throw new Error('Output buffer too small');

			output.set(result, offset);
			written = result.length;
			this._ended = this._inflate._hadEnd || false;
		} catch (err) {
			console.error('inflate failed:', err);
			return -1;
		}
		return written;
	}

	isStreamEnd() {
		return this._ended;
	}

	finalize() {
		this._inflate.close(); // not strictly needed, but clean
	}
}



/**
 * Entrypoint like in your C `main()`
 */
async function main(argv) {
  if (argv.length < 3) {
    console.error(`Usage: node ${argv[1]} <input_file> [output_file]`);
    process.exit(1);
  }

  const inputPath = argv[2];
  const outputPath = argv[3] || 'shape.glb';

  const file = await fs.readFile(inputPath);
  const headerSize = ResourceShapeHeader.SIZE;
  if (file.length < headerSize) {
    throw new Error(`File too small (expected at least ${headerSize} bytes)`);
  }

  const fileSize = new DataView(file.buffer, file.byteOffset, 12).getUint32(8, true);
  console.error(`file size (from header): ${fileSize}`);

  if (file.length < fileSize) {
    throw new Error(`Truncated file (expected ${fileSize}, got ${file.length})`);
  }

  // Create inflator (just like C did)
  const inflator = new ZlibInflatorImpl();

  // Export simulated .glb
  const glbData = Main.exportTest(argv.length, argv, file, inflator);

  if (!glbData) {
    throw new Error('glbData is null.');
  }

  // Write it out
  await fs.writeFile(outputPath, new Uint8Array(glbData));
  console.log(`Wrote output to ${outputPath}`);

  inflator.finalize();
}

main(process.argv).catch(err => {
  console.error(err);
  process.exit(1);
});

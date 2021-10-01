# C64 Tap Compression Tool
 C64 tap file compression support for the [Tapuino with packed file loading] project

This tool finds and compresses all TAP files inside a given folder
 - Orignal TAP files will not deleted
 - Compressed files are called .zap files
 - Tap files are still supported 
 -   - you can mix both tap and zap files together
 - Example savings
   - Zybex.tap (881KB) -> Zybex.zap (89KB)

----
Example usage:  
- TapCompression.exe \[.\\]
  - Compresses tap files found in the tools run path
- TapCompression.exe \[yourTaps\\]  
  - ...relative folder
- TapCompression.exe \[c:\data\c64\yourTaps\\]  
  - ...absolute path

(code is in a mess and lots of old stuff left in, now I'm on github... I'll sort that out in the next week or so)

[Tapuino with packed file loading]: https://github.com/titmouse001/tapuino-with-packed-file-loading

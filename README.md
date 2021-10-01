# C64 Tap Compression Tool
 C64 tap file compression support for the [Tapuino with packed file loading] project  


This tool finds and compresses all TAP files inside a given folder
- compressed files are called .zap files
- compressed files must keep the .zap filename extension for [Tapuino with packed file loading] to recognise them
- orignal tap files will not be deleted
- example savings: Zybex.tap (881KB) -> Zybex.zap (89KB)

 Credit goes Charcole for the [Origianl Idea], nice.
 
----
Example usage:  
- TapCompression.exe \[.\\]
  - Compresses tap files found in the tools run path
- TapCompression.exe \[yourTaps\\]  
  - ...relative folder
- TapCompression.exe \[c:\data\c64\yourTaps\\]  
  - ...absolute path


[origianl idea]:https://github.com/charcole/C64Tape
[Tapuino with packed file loading]: https://github.com/titmouse001/tapuino-with-packed-file-loading

# Example on how to use the LLVM library

`MyClass.cpp` is a C++ file that declares a small class and a `main` function that uses this class.

`main.cpp` is a C++ file that uses the LLVM library to generate a LLVM code performing the same things that `MyClass.cpp`. There are a lot of comments describing the different parts of the LLVM code generation.

To be able to generate the LLVM code using the library on your computer, you must **install** it first. It is maybe already installed, for instance for the MacOS users. You can check that the library has been installed if you have access to the `llvm-config` command.

To compile LLVM code, you will also need `clang`.

You can also use the Docker container, all you need is already installed.

To generate the LLVM code:
- Use `make` to compile `main.cpp`. The `Makefile` uses the `llvm-config` command to retrieve the folders where are stored the LLVM librabry. You may have to add a suffix (the LLVM version) to use it on your computer (for instance `llvm-config-11`).
- Then you can use the created executable to generate the LLVM code and to store it in a file:
  ```bash
  ./main > code.ll
  ```

You can then use `clang` to generate an executable from the LLVM file:
```bash
clang -o code code.ll
```

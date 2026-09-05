# Khthon: a Hellbound VSOP to LLVM Compiler

Khthon is a toy compiler frontend for the VSOP language, a small OOP language designed to be compiled easily. It outputs LLVM IR and can generate an executable.

This project has been developed by Valérian Wislez in the scope of the course *2025-2026 / INFO0085-1 Compilers* by Pascal Fontaine at ULiège. It is now publicly available.

## License

This project is licensed under the **PolyForm Noncommercial License 1.0.0**. 
You are free to view, modify, and distribute this software for non-commercial purposes. 
Commercial use requires a separate commercial license from the author.

## Prerequisites

### LLVM version 11
For now, Khthon only works using LLVM version 11, which we will soon try to relax by rewriting part of the code to use more recent (21+) LLVM version.

In order to use Khthon, you should have LLVM 11 installed on your system but we do not provide an automatic way of installing it here. 

Either you can install it using your package manager, or you can recompile it from source after downloading it on the [release pages](https://github.com/llvm/llvm-project/releases/tag/llvmorg-11.1.0). 

Download the LLVM 11 release file (clang+llvm) for your architecture.

If you recompile it with a recent compiler, there will be some errors during compilation, telling you some includes are not working. You'll have to manually edit them to add the required includes. Follow your compiler's notes and it should go fine, this is mostly a 3 steps edit.

To check everything is working, simply call in your shell the following and verify it returns the 11.1.0 version. From now on, Khthon should compile just fine.

```sh
llvm-config --version
```

Alternatively, you can simply execute `run.sh`, which will download and launch a container which is fitted with everything you need. This is the easiest for now.

## Compiling

Use `make install-tools` to install all the necessary dependencies (for now, there is none, except LLVM version 11)

Use `make all` to compile Khthon in **release mode**.

Use `make debug` to compile Khthon in **debug mode**, thereby adding colored output and logging information.

## Usage

Once Khthon is compiled, you can use it as follows.

To get the lexer output on stdout, use:
```sh
./vsopc -l file/to/compile 
```

To get the abstract syntax tree output on stdout, use:
```sh
./vsopc -p file/to/compile 
```

To get the annotated (typed) abstract syntax tree output on stdout, use:
```sh
./vsopc -c file/to/compile 
```

To get the generated LLVM IR on stdout, use:
```sh
./vsopc -i file/to/compile 
```

To make an executable file out of the generated IR, use:
```sh
./vsopc file/to/compile 
```

You can then execute it (without arguments) using:
```sh
./file/to/compile
```

### VSCode configuration 

This project uses a custom VSOP programming language, which is not recognized by VSCode, and for which there is no known extension.
Hence if you would like code highlighting for the `.vsop` files, we recommend highlighting it as Java, which provides a decent colorization.

You can also color the .out as `diff` when reading the automated tests output.

There exists a nice [extension](https://github.com/colejcummins/llvm-syntax-highlighting) for LLVM highlighting as well.

All in all, add a `.vscode/settings.json` file in which you can copy paste the following:

```json
{
    "files.associations": {
        "*.vsop": "java",
        "*.out": "diff",
        "*.ll": "llvm"
    }
}
```

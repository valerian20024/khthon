# Khthon: a Hellbound VSOP to LLVM Compiler

This project is held by Valérian Wislez in the scope of the course *2025-2026 / INFO0085-1 Compilers* by Pascal Fontaine at ULiège.

# VSCode configuration 

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
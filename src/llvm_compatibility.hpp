#ifndef LLVM_COMPATIBILITY_HPP
#define LLVM_COMPATIBILITY_HPP

// LLVM headers contain a lot of warnings when compiling.
// Omitting them to focus on Khthon's warnings.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" 
#pragma GCC diagnostic ignored "-Wunused-variable" 

#include "llvm/Config/llvm-config.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"

// Headers have changed with time.
// Used to find LLVM target triple.
#if LLVM_VERSION_MAJOR >= 17
    #include "llvm/TargetParser/Host.h"
#else
    #include "llvm/Support/Host.h"
#endif

#pragma GCC diagnostic pop


#endif

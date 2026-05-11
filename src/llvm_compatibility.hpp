#ifndef LLVM_COMPATIBILITY_HPP
#define LLVM_COMPATIBILITY_HPP

/**
 * This header acts as a central coordinator for including anything LLVM.
 * In its current state, it manages to ignore the messy output of compiling
 * LLVM code and makes sure to include the correct headers based on the LLVM
 * version. This is mostly a proof of concept, we do not guarantee to support
 * any other version than LLVM 11.
 * 
 * The code needing to include LLVM headers should only include this very 
 * header to ensure a smooth and consistent behavior, namely generation.hpp.
 */

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
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"

// Header used to find LLVM target triple.
#if LLVM_VERSION_MAJOR >= 17
    #include "llvm/TargetParser/Host.h"
#else
    #include "llvm/Support/Host.h"
#endif

// We could also add a wrapper for llvm::getPtrTy since getInt8PtrTy is 
// deprecated in recent LLVM versions.

#pragma GCC diagnostic pop

#endif

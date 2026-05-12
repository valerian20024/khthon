; ModuleID = 'VSOP Module'
source_filename = "tests/simplest.vsop"
target triple = "x86_64-unknown-linux-gnu"

%Object___VTable = type { %Object* (%Object*, i8*)*, %Object* (%Object*, i1)*, %Object* (%Object*, i32)*, i8* (%Object*)*, i1 (%Object*)*, i32 (%Object*)* }
%Object = type { %Object___VTable* }
%Main___VTable = type { %Object* (%Main*, i8*)*, %Object* (%Main*, i1)*, %Object* (%Main*, i32)*, i8* (%Main*)*, i1 (%Main*)*, i32 (%Main*)*, i32 (%Main*)* }
%Main = type { %Main___VTable* }

@Object___vtable = external constant %Object___VTable
@Main___vtable = internal constant %Main___VTable { %Object* (%Main*, i8*)* @Main__print, %Object* (%Main*, i1)* @Main__printBool, %Object* (%Main*, i32)* @Main__printInt32, i8* (%Main*)* @Main__inputLine, i1 (%Main*)* @Main__inputBool, i32 (%Main*)* @Main__inputInt32, i32 (%Main*)* @Main__main }

declare %Object* @Object__print(%Object*, i8*)

declare %Object* @Object__printBool(%Object*, i1)

declare %Object* @Object__printInt32(%Object*, i32)

declare i8* @Object__inputLine(%Object*)

declare i1 @Object__inputBool(%Object*)

declare i32 @Object__inputInt32(%Object*)

declare %Object* @Object___new()

declare %Object* @Object___init(%Object*)

declare i8* @malloc(i64)

define i32 @Main__main(%Main* %0) {
entry:
  ret i32 0
}

define %Object* @Main__print(%Main* %self, i8* %s) {
entry:
  %0 = bitcast %Main* %self to %Object*
  %call_real = call %Object* @Object__print(%Object* %0, i8* %s)
  ret %Object* %call_real
}

define %Object* @Main__printBool(%Main* %self, i1 %b) {
entry:
  %0 = bitcast %Main* %self to %Object*
  %call_real = call %Object* @Object__printBool(%Object* %0, i1 %b)
  ret %Object* %call_real
}

define %Object* @Main__printInt32(%Main* %self, i32 %i) {
entry:
  %0 = bitcast %Main* %self to %Object*
  %call_real = call %Object* @Object__printInt32(%Object* %0, i32 %i)
  ret %Object* %call_real
}

define i8* @Main__inputLine(%Main* %self) {
entry:
  %0 = bitcast %Main* %self to %Object*
  %call_real = call i8* @Object__inputLine(%Object* %0)
  ret i8* %call_real
}

define i1 @Main__inputBool(%Main* %self) {
entry:
  %0 = bitcast %Main* %self to %Object*
  %call_real = call i1 @Object__inputBool(%Object* %0)
  ret i1 %call_real
}

define i32 @Main__inputInt32(%Main* %self) {
entry:
  %0 = bitcast %Main* %self to %Object*
  %call_real = call i32 @Object__inputInt32(%Object* %0)
  ret i32 %call_real
}

define %Main* @Main___init(%Main* %self) {
entry:
  %vtable_ptr = getelementptr inbounds %Main, %Main* %self, i32 0, i32 0
  store %Main___VTable* @Main___vtable, %Main___VTable** %vtable_ptr, align 8
  ret %Main* %self
}

define %Main* @Main___new() {
entry:
  %raw_mem = call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %obj = bitcast i8* %raw_mem to %Main*
  %0 = call %Main* @Main___init(%Main* %obj)
  ret %Main* %obj
}

define i32 @main() {
entry:
  %main_object = call %Main* @Main___new()
  %ret = call i32 @Main__main(%Main* %main_object)
  ret i32 %ret
}

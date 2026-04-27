; ModuleID = 'test'
source_filename = "test"

%struct.MyClassVTable = type { void (%MyClass*, i32, i32)* }
%MyClass = type { %struct.MyClassVTable*, i32, i1 }

@MyClass_vtable = internal constant %struct.MyClassVTable { void (%MyClass*, i32, i32)* @add }

declare i8* @malloc(i64)

define void @add(%MyClass* %self, i32 %a, i32 %b) {
entry:
  %0 = getelementptr %MyClass, %MyClass* %self, i32 0, i32 1
  %1 = load i32, i32* %0, align 4
  %2 = add i32 %1, %a
  %3 = add i32 %2, %b
  store i32 %3, i32* %0, align 4
  ret void
}

define %MyClass* @new_MyClass() {
entry:
  %0 = getelementptr %MyClass, %MyClass* null, i32 1
  %1 = ptrtoint %MyClass* %0 to i64
  %2 = call i8* @malloc(i64 %1)
  %3 = bitcast i8* %2 to %MyClass*
  %4 = call %MyClass* @init_MyClass(%MyClass* %3)
  ret %MyClass* %4
}

define %MyClass* @init_MyClass(%MyClass* %self) {
entry:
  %0 = getelementptr %MyClass, %MyClass* %self, i32 0, i32 0
  store %struct.MyClassVTable* @MyClass_vtable, %struct.MyClassVTable** %0, align 8
  %1 = getelementptr %MyClass, %MyClass* %self, i32 0, i32 1
  store i32 10, i32* %1, align 4
  %2 = getelementptr %MyClass, %MyClass* %self, i32 0, i32 2
  store i1 true, i1* %2, align 1
  ret %MyClass* %self
}

define i32 @main() {
entry:
  %0 = call %MyClass* @new_MyClass()
  %1 = getelementptr %MyClass, %MyClass* %0, i32 0, i32 0
  %2 = load %struct.MyClassVTable*, %struct.MyClassVTable** %1, align 8
  %3 = getelementptr %struct.MyClassVTable, %struct.MyClassVTable* %2, i32 0, i32 0
  %4 = load void (%MyClass*, i32, i32)*, void (%MyClass*, i32, i32)** %3, align 8
  call void %4(%MyClass* %0, i32 1, i32 2)
  ret i32 0
}

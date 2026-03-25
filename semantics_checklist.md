# 4.1. Classes

4100 | TODO
classes.vsop
class globally visible.

4101 | TODO
classes.vsop
define a type.

4102 | TODO
classes.vsop
Can be used (as a type) before being declared/defined (here it is the same).

4103 | TODO
classes_ERROR.vsop
cannot be redefined.

4104 | TODO
classes_ERROR.vsop
Two fields/methods of a same class cannot have the same name, even with
different type.

4105 | TODO
class.vsop
A field and a method of a same class can have the same name.

# 4.1.1. New

4110 | TODO
classes.vsop
The type of expression new C is C.

# 4.1.2. Inheritance

4120 | TODO
inheritance.vsop
If X extends Y it inherits all its fields and methods

4121 | TODO
inheritance.vsop
transitivity of inheritance

4122 | TODO
inheritance.vsop
child class conformity: child class can be used in any place where parent can
be used

4123 | TODO
inheritance_ERROR.vsop
child class conformity: child class can only be used in any place where parent
can be used (FALSE OR WEIRDLY SAID, TO CHANGE)

4124 | TODO
inheritance_ERROR.vsop
cannot redefine a field of a parent

4125 | TODO
inheritance.vsop
can override parent method

4126 | TODO
inheritance_ERROR.vsop
overriden method should conform to arguments and return type (smae signature)

4127 | TODO
inheritance_ERROR.vsop
no inheritance cycle

# 4.1.3. Fields

4130 | TODO
fields_ERROR.vsop
type of the initializer must conform to declared type

4131 | TODO 
fields_ERROR.vsop
object (self), its fields and methods are not yet in the scope

4132 | TODO 
fields_ERROR.vsop
object (self), its fields and methods are not yet in the scope

4133 | TODO 
fields.vsop
field can be manipulated by method

4134 | TODO 
fields.vsop
field can be manipulated by child method

4135 | TODO
fields_ERROR.vsop
field are protected and can only be manipulated by (child) method

# 4.1.4. Methods

4140 | TODO
methods.vsop
methods have global scope

4141 | TODO
methods_ERROR.vsop
The identifiers used in the formal parameters list must be distinct.

4142 | TODO
methods_ERROR.vsop
type of the body must conform to declared return type

# 4.1.5. Object Class

4150 | TODO
classes.vsop
Object class is default parent

# 4.1.6. Main Class

4160 | TODO
main_class_ERROR.vsop
A program must have a Main class

4161 | TODO
main_class2_ERROR.vsop
A program must have a main method in Main class

4162 | TODO
main_class3_ERROR.vsop
A program must have a main method with no arguments returning in Main class

4163 | TODO
main_class4_ERROR.vsop
A program must have a main method returning int32 in Main class
// ajouter TypesMap::at() qui ne crée pas de nouvelle classe si elle n'existe pas

# 4.2. Expressions

# 4.2.1. Literals

4210 | TODO
literals.vsop
return type: true/false  bool. integer literals  int32. string literals 
string. ()  unit

# 4.2.2. Identifiers

4220 | TODO 
identifier.vsop
name of local variable returns the corresponding type

4221 | TODO 
identifier.vsop
name of method parameters returns the corresponding type

4222 | TODO 
identifier.vsop
name of fields returns the corresponding type

4223 | TODO 
identifier.vsop
self returns the type of the class

4224 | TODO 
identifier.vsop
a field can be hidden by a formal parameter or local variable

4225 | TODO 
identifier.vsop
a formal parameter can be hidden by a local variable

4226 | TODO 
identifier.vsop
a local variable can be hidden by a local variable within its scope

4227 | TODO 
identifier_ERROR.vsop
self cannot be hidden. Error if declared as a field, formal parameter or local variable

4228 | TODO 
identifier_ERROR.vsop // change file name for the same one as 4131
self is only bound in method (not in field initializer. Already tested in 4131)

# 4.2.3. Assignments

4230 | TODO
assignments_ERROR.vsop
type of expr must conform to type of id 

# 4.2.4. Dispatch

4240 | TODO
dispatch.vsop
a.b(p1, ..., pn), a must have a method b.

4241 | TODO
dispatch_ERROR.vsop
a.b(p1, ..., pn), a must have a method b.

4242 | TODO
dispatch_ERROR.vsop
p_i must have the same type as the ith formal parameter

4243 | TODO
dispatch_ERROR.vsop
must have the same number of parameters as the formals

# 4.2.5. Conditionals

4250 | TODO
conditionals_ERROR.vsop
condition expression must be a bool (error if not)

4251 | TODO
conditionals.vsop
then and else branches must have a common ancestor, result is the first common
ancestor or if one branch type is unit, result is unit. (maybe divide the test,
determine what is to be tested exactly)

4252 | TODO
conditionals_ERROR.vsop
then and else branches must have a common ancestor, result is the first common
ancestor or if one branch type is unit, result is unit.

4253 | TODO
conditionals.vsop
without any else it is a shortcut to else (), that is return type is unit

# 4.2.6. Loops

4260 | TODO
conditionals_ERROR.vsop
condition expression must be a bool (error if not)

4261 | TODO
conditionals.vsop
return type is unit

# 4.2.7. Blocks

4270 | TODO
blocks.vsop
result type is the one of the last expression in it

# 4.2.8. Let

4280 | TODO 
let_ERROR.vsop
type of initializer must conform to type

4281 | TODO 
let.vsop
return type is the same as body

4282 | TODO 
let.vsop
scope of id is valid in body

4283 | TODO 
let_ERROR.vsop
scope of id takes its old value when outside the body (may be none  error)

# 4.2.9. Arithmetic, Logic and Comparison Operations

4290 | TODO
arithmetic.vsop
arithmetic operations, <, <= are defined on int32

4291 | TODO
arithmetic_ERROR.vsop
arithmetic operations, <, <= are only defined on int32

4292 | TODO
arithmetic.vsop
= return a bool

4293 | TODO
arithmetic.vsop
= can be used on two values with same primitive type or two objects

4294 | TODO
arithmetic_ERROR.vsop
error if = with two different primitive type or a primitive and a class type

4295 | TODO
arithmetic.vsop
logical operators act on bool

4296 | TODO
arithmetic_ERROR.vsop
logical operators act only on bool

4297 | TODO
arithmetic.vsop
logical operators returns bool


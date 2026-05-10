#include "semantics.hpp"

using namespace std;
using namespace khthon;

namespace khthon {

    bool TypesVisitor::conforms(const Type& given, const Type& expected) const {
        
        // Undefined types should never reach type checking.
        if (given.is_undefined() || expected.is_undefined()) {
            driver_.internal_error("conforms(): encountered an undefined type");
            return false;
        }

        // Primitive types conform only with themselves.
        if (given.is_primitive() || expected.is_primitive())
            return given == expected;

        // Types are custom so we check for subtyping.
        return checker_.is_subtype(given, expected);
    }

    bool TypesVisitor::check_unop_operand(
        const UnaryOperation& operation,
        const Type& t_operand
    ) const {
        
        // If the given operand is one of those expected by the operation.
        for (const auto& expected : operation.valid_operand_types()) {
            if (conforms(t_operand, expected))
                return true;
        }
        return false;
    }

    bool TypesVisitor::check_binop_operands(
        const BinaryOperation& op,
        const Type& type_l,
        const Type& type_r
    ) const {

        // Handling the special case of equality.
        if (op.is_equality())
            return (type_l == type_r) 
                || (type_l.is_custom() && type_r.is_custom());

        // If the given operands are among the pairs expected by the operation.
        for (const auto& [expect_l, expect_r] : op.valid_operand_types()) {
            if (   conforms(type_l, expect_l) 
                && conforms(type_r, expect_r))
                return true;
        }
        return false;
    }

    bool TypesVisitor::check_formals(
        const MethodInfo& method,
        const vector<shared_ptr<Expr>>& args,
        const location& loc
    ) const {

        const auto& formals = method.formals();

        // Check argument count first, as a mismatch makes type checking meaningless.
        if (args.size() != formals.size()) {
            driver_.semantic_error(
                loc,
                "method '" + method.name() + "' expects "
                + to_string(formals.size())
                + " argument(s), got "
                + to_string(args.size())
            );
            return false;
        }

        // Reporting each formal type mismatch.
        bool all_conform = true;
        for (size_t i = 0; i < formals.size(); ++i) {
            const Type& expected = formals[i].type();
            const Type& actual = args[i]->type();

            if (!conforms(actual, expected)) {
                driver_.semantic_error(
                    loc,
                    "argument " + to_string(i + 1)
                    + " of method '" + method.name()
                    + "' expects type '" + expected.to_string()
                    + "', got '" + actual.to_string() + "'"
                );
                all_conform = false;
            }
        }

        return all_conform;
    }

    bool TypesVisitor::check_type_exists(
        const Type& type, 
        const khthon::location& loc
    ) const {
        // Primitive types are always valid.
        if (type.is_primitive() || type.is_unit())
            return true;

        // Custom types must exist in the class table.
        if (type.is_custom() && !checker_.class_exists(type.custom_name())) {
            driver_.semantic_error(
                loc,
                "use of undefined type '" + type.custom_name() + "'"
            );
            return false;
        }

        return true;
    }

    void TypesVisitor::trace(const string& message) const { 
        if (khthon::enable_advanced_logging)
            cout << message << endl;
    }

    void TypesVisitor::visit(ProgramNode& node) {
        trace("TypesVisitor visits ProgramNode");

        for (const auto& c : node.classes())
            c->accept(*this);
    }

    void TypesVisitor::visit(ClassNode& node) {
        trace("TypesVisitor visits ClassNode");
        
        current_class_name_ = node.name();
        
        //todo Have fresh scope for new class
        
        for (const auto& f : node.fields())
            f->accept(*this);

        for (const auto& m : node.methods())
            m->accept(*this);
    }

    void TypesVisitor::visit(MethodNode& node) {
        trace("TypesVisitor visits MethodNode");

        check_type_exists(node.type(), node.location());
        for (const auto& formal : node.formals())
            check_type_exists(formal->type(), formal->location());

        checker_.push_scope();

        // Bind self to the current class type.
        checker_.add_binding("self", Type(current_class_name_));

        // Bind each formal to its declared type.
        for (const auto& formal : node.formals())
            checker_.add_binding(formal->name(), formal->type());

        node.body()->accept(*this);

        const Type& declared = node.type();
        const Type& actual = node.body()->type();

        if (!actual.is_undefined() && !conforms(actual, declared)) {
            driver_.semantic_error(
                node.location(),
                "method '" + node.name() + "' has return type '"
                + declared.to_string()
                + "' but body has type '"
                + actual.to_string() + "'"
            );
        }

        checker_.pop_scope();
    }

    void TypesVisitor::visit(FormalNode& node) {
        trace("TypesVisitor visits FormalNode");

        (void) node;
        return;
    }

    void TypesVisitor::visit(FieldNode& node) {
        trace("TypesVisitor visits FieldNode");

        check_type_exists(node.type(), node.location());

        if (node.has_init())
            node.initializer().value()->accept(*this);
    }

    void TypesVisitor::visit(BlockExpr& node) {
        trace("TypesVisitor visits BlockExpr");

        // Empty blocks yield unit.
        if (node.is_empty()) {
            node.set_type(Type::Unit());
            return;
        }

        for (const auto& e : node.expressions())
            e->accept(*this);
        
        // The type of a block is the type of its last expression.
        const auto& last_expression = node.last_expression();
        node.set_type(last_expression->type());
    }

    void TypesVisitor::visit(StringLiteralExpr& node) { 
        trace("TypesVisitor visits StringLiteralExpr");

        node.set_type(Type::String());
    }

    void TypesVisitor::visit(IntegerLiteralExpr& node) { 
        trace("TypesVisitor visits IntegerLiteralExpr");

        node.set_type(Type::Int32());
    }

    void TypesVisitor::visit(BoolLiteralExpr& node) { 
        trace("TypesVisitor visits BoolLiteralExpr");

        node.set_type(Type::Bool());
    }

    void TypesVisitor::visit(UnitLiteralExpr& node) { 
        trace("TypesVisitor visits UnitLiteralExpr");

        node.set_type(Type::Unit());
    }

    void TypesVisitor::visit(IfExpr& node) { 
        trace("TypesVisitor visits IfExpr");

        node.guardian()->accept(*this);
        node.consequent()->accept(*this);
        
        if (node.alternative().has_value())
            node.alternative().value()->accept(*this);
        
        // Guardian must have type bool.
        if (!conforms(node.guardian()->type(), Type::Bool())) {
            driver_.semantic_error(
                node.location(),
                "condition of 'if' must be of type 'bool', found '"
                + node.guardian()->type().to_string()
                + "'"
            );
        }

        const Type consequent_type = node.consequent()->type();
        const Type alternative_type = node.alternative().has_value()
            ? node.alternative().value()->type()
            : Type::Unit();
        
        // If at least one branch is unit, the node is unit.
        if (consequent_type.is_unit() || alternative_type.is_unit()) {
            node.set_type(Type::Unit());

        // If branches have class types, the node has the type of their ancestor.
        } else if (consequent_type.is_custom() && alternative_type.is_custom()) {
            node.set_type(checker_.ancestor(consequent_type, alternative_type));

        // If branches have non-unit, primitive types.
        } else if (consequent_type == alternative_type) {
            node.set_type(consequent_type);

        } else {
            driver_.semantic_error(
                node.location(),
                "branches of 'if' have incompatible types '"
                + consequent_type.to_string()
                + "' and '"
                + alternative_type.to_string()
                + "'"
            );

            node.set_type(Type::Object());  // error recovery
        }
    }

    void TypesVisitor::visit(AssignExpr& node) {
        trace("TypesVisitor visits AssignExpr");

        // self is not assignable in VSOP.
        if (node.name() == "self") {
            driver_.semantic_error(
                node.location(),
                "cannot assign to 'self'"
            );
            node.set_type(Type::Object());  // error recovery
            return;
        }

        // Resolve the declared type of the target identifier.
        auto target_type = checker_.resolve(node.name(), current_class_name_);
        if (!target_type) {
            driver_.semantic_error(
                node.location(),
                "undefined identifier '" + node.name() + "'"
            );
            node.set_type(Type::Object());  // error recovery
            return;
        }

        node.value()->accept(*this);

        const Type& value_type = node.value()->type();

        if (!conforms(value_type, target_type.value())) {
            driver_.semantic_error(
                node.location(),
                "cannot assign value of type '" + value_type.to_string()
                + "' to identifier '" + node.name()
                + "' of type '" + target_type->to_string() + "'"
            );
        }

        // Assignment yields the type of the assigned value.
        node.set_type(value_type);
    }

    void TypesVisitor::visit(NewExpr& node) {
        trace("TypesVisitor visits NewExpr");

        const string& class_name = node.identifier();

        if (!checker_.class_exists(class_name)) {
            driver_.semantic_error(
                node.location(),
                "unknown class '" + class_name + "' in 'new' expression"
            );
            node.set_type(Type::Object());  // error recovery
            return;
        }

        node.set_type(Type(class_name));
    }

    void TypesVisitor::visit(UnOpExpr& node) { 
        trace("TypesVisitor visits UnOpExpr");

        node.operand()->accept(*this);

        const Type& t_operand = node.operand()->type();

        if(!check_unop_operand(node.operation(), t_operand)) {
            driver_.semantic_error(
                node.location(),
                "operator '" + node.operation().to_string()
                + "' cannot be applied to type '"
                + t_operand.to_string() + "'"
            );

            node.set_type(Type::Object());
            return;
        }

        node.set_type(node.operation().result_type());
    }

    void TypesVisitor::visit(BinOpExpr& node) { 
        trace("TypesVisitor visits BinOpExpr");

        node.left()->accept(*this);
        node.right()->accept(*this);  
        
        const Type& t_left  = node.left()->type();
        const Type& t_right = node.right()->type();

        if (!check_binop_operands(node.operation(), t_left, t_right)) {
            driver_.semantic_error(
                node.location(),
                "operator '" + node.operation().to_string()
                + "' cannot be applied to types '"
                + t_left.to_string() + "' and '"
                + t_right.to_string() + "'"
            );

            node.set_type(Type::Object());
            return;
        }

        node.set_type(node.operation().result_type());
    }

    void TypesVisitor::visit(VariableExpr& node) { 
        trace("TypesVisitor visits VariableExpr");

        auto type = checker_.resolve(node.identifier(), current_class_name_);
        if (!type) {
            driver_.semantic_error(
                node.location(),
                "undefined identifier '" + node.identifier() + "'"
            );
            node.set_type(Type::Object());  // error recovery
            return;
        }

        node.set_type(type.value());
    }

    void TypesVisitor::visit(CallExpr& node) { 
        trace("TypesVisitor visits CallExpr");

        // Visiting the object receving the call.
        node.receiver()->accept(*this);

        // Visiting all the arguments of the call.
        for (const auto& arg : node.args())
            arg->accept(*this);

        const Type& receiver_type = node.receiver()->type();

        // Only class types have methods.
        if (!receiver_type.is_custom()) {
            driver_.semantic_error(
                node.location(),
                "cannot call method '" + node.name()
                + "' on non-class type '"
                + receiver_type.to_string() + "'"
            );
            node.set_type(Type::Object());
            return;
        }

        auto method = checker_.lookup_method(
            node.name(), 
            receiver_type.custom_name()
        );

        if (!method) {
            driver_.semantic_error(
                node.location(),
                "class '" + receiver_type.custom_name()
                + "' has no method '" + node.name() + "'"
            );
            node.set_type(Type::Object());
            return;
        }

        check_formals(method.value(), node.args(), node.location());

        node.set_type(method->return_type());
    }

    void TypesVisitor::visit(SelfExpr& node) { 
        trace("TypesVisitor visits SelfExpr");

        node.set_type(Type(current_class_name_));
    }

    void TypesVisitor::visit(LetExpr& node) { 
        trace("TypesVisitor visits LetExpr");
    
        check_type_exists(node.type(), node.location());

        // Visit initializer in the outer scope, before binding x.
        if (node.has_initializer()) {
            node.initializer().value()->accept(*this);

            const Type& init_type = node.initializer().value()->type();
            const Type& declared_type = node.type();

            if (!conforms(init_type, declared_type)) {
                driver_.semantic_error(
                    node.location(),
                    "initializer of '" + node.name()
                    + "' has type '" + init_type.to_string()
                    + "' but declared type is '" + declared_type.to_string() + "'"
                );
            }
        }

        // Bind x in a new inner scope, only visible to the scope expression.
        checker_.push_scope();
        checker_.add_binding(node.name(), node.type());

        node.scope()->accept(*this);

        checker_.pop_scope();

        // The type of a let expression is the type of its scope.
        node.set_type(node.scope()->type());
    }

    void TypesVisitor::visit(WhileExpr& node) { 
        trace("TypesVisitor visits WhileExpr");
        
        node.condition()->accept(*this);
        node.body()->accept(*this);

        if (!conforms(node.condition()->type(), Type::Bool())) {
            driver_.semantic_error(
                node.location(),
                "condition of 'while' must be of type 'bool', found '"
                + node.condition()->type().to_string()
                + "'"
            );
        }

        if (node.body()->type().is_undefined())
            driver_.internal_error("visiting WhileExpr found an undefined type");

        node.set_type(Type::Unit());
    }
    
} // namespace khthon

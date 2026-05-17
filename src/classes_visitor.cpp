/**
 * This file contains the implementation of the ClassesVisitor.
 */

#include "semantics.hpp"

namespace khthon {

    void ClassesVisitor::visit(const ProgramNode& node) const {
        
        // Inject the built-in Object class
        khthon::location builtin_loc = driver_.default_location();
        ClassInfo object_info("Object", "Object", builtin_loc);

        // Object's built-in methods
        object_info.add_method(MethodInfo(
            "print",
            Type("Object"),
            { FormalInfo("s", Type::String(), builtin_loc) },
            builtin_loc
        ));

        object_info.add_method(MethodInfo(
            "printBool",
            Type("Object"),
            { FormalInfo("b", Type::Bool(), builtin_loc) },
            builtin_loc
        ));

        object_info.add_method(MethodInfo(
            "printInt32",
            Type("Object"),
            { FormalInfo("i", Type::Int32(), builtin_loc) },
            builtin_loc
        ));

        object_info.add_method(MethodInfo(
            "inputLine",
            Type::String(),
            {},  // no formals
            builtin_loc
        ));

        object_info.add_method(MethodInfo(
            "inputBool",
            Type::Bool(),
            {},
            builtin_loc
        ));

        object_info.add_method(MethodInfo(
            "inputInt32",
            Type::Int32(),
            {},
            builtin_loc
        ));

        checker_.add_class(object_info);

        // Reading concrete classes of the program.
        for (const auto& c : node.classes())
            c->accept(*this);
    }

    void ClassesVisitor::visit(const ClassNode& node) const {
        const string& class_name = node.name();

        // Not adding duplicate classes.
        if (checker_.class_exists(class_name)) {
            driver_.semantic_error(
                node.location(), 
                "class '" + class_name + "' is defined more than once"
            );
            return;
        }

        ClassInfo class_info(class_name, node.parent(), node.location());

        // Fields.
        for (const auto& field : node.fields()) {
            FieldInfo field_info(
                field->name(), 
                field->type(), 
                field->location()
            );

            if (!class_info.add_field(std::move(field_info))) {
                driver_.semantic_error(
                    field->location(), 
                    "field '" + field->name() + "' is defined more than once"
                );
            }
        }

        // Methods.
        for (const auto& method : node.methods()) {
            // Formals.
            vector<FormalInfo> formals_infos;
            for (const auto& formal : method->formals()) {
                formals_infos.emplace_back(
                    formal->name(), 
                    formal->type(), 
                    formal->location()
                );
            }
            
            MethodInfo method_info(
                method->name(), 
                method->type(), 
                std::move(formals_infos), 
                method->location()
            );

            if (!class_info.add_method(std::move(method_info))) {
                driver_.semantic_error(
                    method->location(),
                    "method '" + method->name() + "' is defined more than once"
                );
            }
        }

        checker_.add_class(class_info);
    }
} // namespace khthon

#include "ast.hpp"

using namespace std;

namespace Khthon {

    shared_ptr<MethodNode> MethodNode::makeDummy(
        Khthon::location loc, 
        string name
    ) {
        shared_ptr<FormalNode> dummy_formal = make_shared<FormalNode>(
            loc, 
            "DEFAULT_FORMAL", 
            Khthon::Type()
        );

        vector<shared_ptr<FormalNode>> dummy_formals;
        dummy_formals.push_back(dummy_formal);

        shared_ptr<BlockExpr> dummy_body = make_shared<BlockExpr>(
            loc, 
            vector<shared_ptr<Expr>>{}
        );

        return make_shared<MethodNode>(
            loc,
            std::move(name),
            Khthon::Type(),
            dummy_formals,
            dummy_body
        );
    }
}
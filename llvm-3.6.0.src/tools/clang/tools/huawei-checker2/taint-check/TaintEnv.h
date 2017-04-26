
#ifndef TAINT_ENV_H
#define TAINT_ENV_H

#include "taint-check/TaintValue.h"

namespace taint {

class Environment {

public:
    
    Environment(ASTFunction *F, bool is_entry = false) {
        this->F = F;
        const std::vector<ASTVariable *> &variables = F->getVariables();
        for (unsigned i = 0; i < variables.size(); i++) {
            values.push_back(TaintValueV(variables[i], is_entry));
        }
        return_value = TaintValueV(ValueKind::Return);
    }

    Environment(const Environment &env) {
        F = env.F;
        for (auto v : env.values) {
            values.push_back(TaintValueV::copy(v));
        }
        return_value = TaintValueV::copy(env.return_value);
    }

    Environment &operator=(const Environment &env) {
        F = env.F;
        values.clear();
        for (auto v : env.values) {
            values.push_back(TaintValueV::copy(v));
        }
        return_value = TaintValueV::copy(env.return_value);
        return *this;
    }

    friend Environment operator +(const Environment &env1, const Environment &env2);
    friend bool operator ==(const Environment &env1, const Environment &env2);
    friend bool operator !=(const Environment &env1, const Environment &env2);

    template<typename OStream>
    friend OStream &operator <<(OStream &out, const Environment &env);

    ASTFunction *F;
    std::vector<TaintValueV> values;
    TaintValueV return_value;

};


Environment operator +(const Environment &env1, const Environment &env2);
bool operator ==(const Environment &env1, const Environment &env2);
bool operator !=(const Environment &env1, const Environment &env2);

template<typename OStream>
OStream &operator <<(OStream &out, const Environment &env) {
    
    for (auto v : env.values) {
        out << v.getValue() << " ";
    }
    out << env.return_value.getValue() << "\n";
    return out;
}


TaintValueV visitStmt(Stmt *S, Environment *env);

}
#endif

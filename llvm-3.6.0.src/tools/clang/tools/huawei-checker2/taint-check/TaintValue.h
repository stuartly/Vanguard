
#ifndef TAINT_VALUE_H
#define TAINT_VALUE_H

#include "framework/ASTElement.h"

#include "clang/AST/AST.h"
#include "clang/Frontend/ASTUnit.h"

enum TaintKind {Tainted, Untainted, Gamma};
enum ValueKind {Variable, Return, Temp};

namespace taint {

class TaintValue {

public: 
    TaintValue() {
        value = 0;
    }
    
    TaintValue(unsigned v) {
        value = v;
    }
    
    TaintValue(TaintKind kind) {
        if (kind == TaintKind::Untainted) value = 0;
        else if (kind == TaintKind::Tainted) value = UINT_MAX;
        else value = 1;
    }

    TaintValue(ASTVariable *V, bool is_gamma = false) {
        if (V->getID() < V->getFunction()->getParamSize() && is_gamma)
            value = 1 << V->getID();
        else
            value = 0;
    }

    bool isTainted() const {
        return value > 0;
    }
    
    TaintKind getTaintKind() const {
        if (value == 0) return TaintKind::Untainted;
        if (value == UINT_MAX) return TaintKind::Tainted;
        return TaintKind::Untainted;
    }

    unsigned getValue() const {
        return value;
    }

    void update(unsigned value) {
        this->value = value;
    }
    void update(const TaintValue &v) {
        value = v.value;
    }
    
    friend TaintValue operator +(const TaintValue &v1, const TaintValue &v2);
    friend bool operator ==(const TaintValue &v1, const TaintValue &v2);
    friend bool operator !=(const TaintValue &v1, const TaintValue &v2);

private:
    unsigned value;
};


TaintValue operator +(const TaintValue &v1, const TaintValue &v2);
bool operator ==(const TaintValue &v1, const TaintValue &v2);
bool operator !=(const TaintValue &v1, const TaintValue &v2);

class TaintValueV {

public:
    
    TaintValueV() {
        V = nullptr;
        value = std::make_shared<TaintValue>();
        kind = ValueKind::Temp;
    }

    TaintValueV(ASTVariable *V, bool is_gamma = false) {
        this->V = V;
        value = std::make_shared<TaintValue>(V, is_gamma);
        kind = ValueKind::Variable;
    }
    
    TaintValueV(ValueKind kind) {
        V = nullptr;
        value = std::make_shared<TaintValue>();
        this->kind = kind;
    }

    TaintValueV(TaintKind kind) {
        V = nullptr;
        value = std::make_shared<TaintValue>(kind);
        this->kind = ValueKind::Temp;
    }
    
    TaintValueV(unsigned v) {
        V = nullptr;
        value = std::make_shared<TaintValue>(v);
        this->kind = ValueKind::Temp;
    }

    TaintValueV(TaintValue v) {
        V = nullptr;
        value = std::make_shared<TaintValue>(v.getValue());
        this->kind = ValueKind::Temp;
    }

    bool is_tainted() const {
        return value->isTainted();
    }
    
    TaintKind getTaintKind() const {
        return value->getTaintKind();
    }
    
    unsigned getValue() const {
        return value->getValue();
    }

    static TaintValueV copy(const TaintValueV &v) {
        TaintValueV v2 = TaintValueV(v.getValue());
        v2.V = v.V;
        v2.kind = v.kind;
        return v2;
    }

    void update(const TaintValueV &valuev, bool is_pointer = false) {
        if (is_pointer) {
            value = valuev.value;
        }
        else {
            value->update(*valuev.value);
        }
    }
 
    void update(const TaintValue &v, bool is_pointer = false) {
        if (is_pointer) {
            value = std::make_shared<TaintValue>(v.getValue());
        }
        else {
            value->update(v);
        }
    }

    ASTVariable *V;
    ValueKind kind;

    friend TaintValueV operator +(const TaintValueV &v1, const TaintValueV &v2);
    friend bool operator ==(const TaintValueV &v1, const TaintValueV &v2);
    friend bool operator !=(const TaintValueV &v1, const TaintValueV &v2);
    
    friend TaintValue combineValues(TaintValueV value, std::vector<TaintValueV> values);

private:

    std::shared_ptr<TaintValue> value;

    void setValue(unsigned v) {
        value->update(v);
    }

};

TaintValueV operator +(const TaintValueV &v1, const TaintValueV &v2);
bool operator ==(const TaintValueV &v1, const TaintValueV &v2);
bool operator !=(const TaintValueV &v1, const TaintValueV &v2);


TaintValue combineValues(TaintValueV value, std::vector<TaintValueV> values);

}


#endif

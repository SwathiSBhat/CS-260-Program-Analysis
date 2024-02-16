#include<iostream>
#include<string>
#include<vector>
#include<set>
#include<variant>
/*
* Defining set constraint language
* x = set variable 
* c = constructor
* t = term = x | c(t1, t2, ..., tn) 
* e = expression = t | proj(c, x, i)
* s = statement = e1 <= e2 | s1 ^ s2
*/

enum NodeType {
    SET_VAR,
    CONSTRUCTOR,
    PROJECTION,
    LAM
};

/*
* A constructor has a name, arity, a list of arguments and a list of contravariant positions
*/
class Constructor {
    public:
    std::string name;
    int arity;
    std::set<int> contravariant_pos;

    Constructor(const std::string& name, const int& arity, const std::set<int>& contravariant_pos) : 
    name(name), arity(arity), contravariant_pos(contravariant_pos){}

    const std::string Name() const { return name; }
    const int Arity() const { return arity; }
    const bool IsContraPos(int pos) const { return contravariant_pos.count(pos); }

};

/*
* A node in the graph can be a set variable, a constructor or a projection
* Each node has a name and a set of predecessor and successor nodes 
* If the node is a set variable, it can store it's projections
*/
class Node {
    public:
    std::set<Node*> proj_sv_refs, predecessor_nodes, successor_nodes;

    Node(const std::string name) : name(name) {
        type = NodeType::SET_VAR;
    }

    Node(const std::string name, const std::vector<std::variant<std::string, Node*>> args) : name(name), args(args) {
        type = NodeType::CONSTRUCTOR;
    }

    Node(const std::string name, const std::string proj_sv, int proj_idx) : name(name), proj_sv_(proj_sv), proj_idx_(proj_idx) {
        type = NodeType::PROJECTION;
    }

    Node(const std::string name, const std::vector<std::variant<std::string, Node*>> args, const std::string ret_type, const std::vector<std::string> param_types) :
    name(name), args(args), ret_type(ret_type), param_types(param_types) {
        type = NodeType::LAM;
        // If the return type is a pointer, argument at index 1 = return value which is covariant, else all arguments from index 1 are contravariant
        if (ret_type[0] == '&')
            does_ret_val = true;
        else
            does_ret_val = false;
    }

    const std::string Name() const { return name; }
    std::vector<std::variant<std::string, Node*>> CallArgs() { return args; }
    std::variant<std::string, Node*> GetArgAt(int pos) { return args.at(pos); }
    const std::string ProjSV() const { return proj_sv_; }
    const int ProjIdx() const { return proj_idx_; }

    const bool IsSetVar() const { return type == NodeType::SET_VAR; }
    const bool IsConstructor() const { return type == NodeType::CONSTRUCTOR; }
    const bool IsProjection() const { return type == NodeType::PROJECTION; }
    const bool IsLam() const { return type == NodeType::LAM; }

    const bool HasPredecessor(Node* node) {
        return predecessor_nodes.find(node) != predecessor_nodes.end();
    }
    const bool HasSuccessor(Node* node) {
        return successor_nodes.find(node) != successor_nodes.end();
    }

    const bool HasRetVal() const { return does_ret_val; }

    private:
    NodeType type;
    std::string name;
    std::vector<std::variant<std::string, Node*>> args;
    std::string proj_sv_;
    int proj_idx_;
    std::string ret_type;
    std::vector<std::string> param_types;
    bool does_ret_val;
};
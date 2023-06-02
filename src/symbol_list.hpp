#include <map>
#include <string>
#include <vector>
#include <memory>

class BaseAST;

class SymbolList
{
    std::vector<std::map<std::string, void*>> sym_stk;

public:
    void NewEnv()
    {
        sym_stk.push_back(std::map<std::string, void*>());
    }
    void AddSymbol(const std::string &name, void *koopa_item)
    {
        auto &list = sym_stk[sym_stk.size()-1];
        list[name] = koopa_item;
    }
    void * GetSymbol(const std::string &name)
    {
        void *res = nullptr;
        for(size_t i = sym_stk.size()-1; i>=0; i--)
            if(sym_stk[i].count(name) != 0)
            {
                res = sym_stk[i][name];
                break;
            }
        return res;
    }
    void DeleteEnv()
    {
        sym_stk.pop_back();
    }
};

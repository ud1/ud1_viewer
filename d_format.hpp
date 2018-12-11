#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <iostream>
#include <string>
#include <cstdint>
#include <variant>
#include <map>
#include "myutils.hpp"
#include "myutils3d.hpp"
#include <vector>
#include <mutex>
#include <memory>

typedef std::variant<std::monostate, std::string, double, uint32_t, P, V3, M4> var_t;
typedef std::map<std::string, var_t> SObj;

std::string toString(const var_t &val);
inline std::string getStr(const std::string &key, const SObj &sobj)
{
    auto it = sobj.find(key);
    if (it != sobj.end() && std::holds_alternative<std::string>(it->second))
        return std::get<std::string>(it->second);

    return "";
}

inline P getP(const std::string &key, const SObj &sobj)
{
    auto it = sobj.find(key);
    if (it != sobj.end() && std::holds_alternative<P>(it->second))
        return std::get<P>(it->second);

    return P(1, 0);
}

inline V3 getV3(const std::string &key, const SObj &sobj)
{
    auto it = sobj.find(key);
    if (it != sobj.end() && std::holds_alternative<V3>(it->second))
        return std::get<V3>(it->second);

    return V3(0, 0, 0);
}

inline M4 getM4(const std::string &key, const SObj &sobj)
{
    auto it = sobj.find(key);
    if (it != sobj.end() && std::holds_alternative<M4>(it->second))
        return std::get<M4>(it->second);

    return M4(1.0f);
}

inline uint32_t getInt(const std::string &key, const SObj &sobj)
{
    auto it = sobj.find(key);
    if (it != sobj.end() && std::holds_alternative<uint32_t>(it->second))
        return std::get<uint32_t>(it->second);

    return 0;
}

inline double getDouble(const std::string &key, const SObj &sobj)
{
    auto it = sobj.find(key);
    if (it != sobj.end() && std::holds_alternative<double>(it->second))
        return std::get<double>(it->second);

    return 0.0;
}

struct Obj {
    std::string type;
    SObj props;
    std::map<std::string, SObj> subObjs;

    uint32_t getIntProp(const std::string &key) const {
        return getInt(key, props);
    }

    P getPProp(const std::string &key) const {
        return getP(key, props);
    }

    double getDoubleProp(const std::string &key) const {
        return getDouble(key, props);
    }
};

void writeObj(std::ostream &ostream, const Obj &obj);
Obj readObj(std::istream &istream);

struct Frame {
    unsigned tick = 0;
    std::vector<Obj> objs;
    mutable std::mutex mutex;
};

std::ostream &operator << (std::ostream &str, const SObj &obj);
std::ostream &operator << (std::ostream &str, const Obj &obj);


#endif // FORMAT_HPP

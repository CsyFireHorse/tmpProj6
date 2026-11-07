#ifndef ZMC_BASE_OPT_HPP
#define ZMC_BASE_OPT_HPP

namespace zmc {

template <typename Base, typename Member>
class BaseOpt : public Base {
    Member m;
    BaseOpt(Base const& b, Member const& mem) : Base(b), m(mem) {}
};

}  // namespace zmc

#endif
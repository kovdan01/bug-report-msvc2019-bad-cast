#include <msgpack.hpp>

#include <cmath>
#include <iostream>

#define DEBUGOUT std::cerr << "[debug] "
#define TESTOUT  std::cerr << "[test]  "

class TestUnionMemberClass
{
public:
    TestUnionMemberClass() = default;

    TestUnionMemberClass(double f)
    {
        is_double = true;
        value.f = f;
    }
    TestUnionMemberClass(int i)
    {
        is_double = false;
        value.i = i;
    }

    union
    {
        double f;
        int i;
    } value;
    bool is_double;

    template <typename Packer>
    void msgpack_pack(Packer& pk) const
    {
        if (is_double)
        {
            DEBUGOUT << "pack double" << std::endl;
            pk.pack(msgpack::type::tuple<bool, double>(true, value.f));
        }
        else
        {
            DEBUGOUT << "pack int" << std::endl;
            pk.pack(msgpack::type::tuple<bool, int>(false, value.i));
        }
    }

    void msgpack_unpack(msgpack::object o)
    {
        msgpack::type::tuple<bool, msgpack::object> tuple(false, msgpack::object());
        // If you replace the line above with the line in the comment below, the problem will disapper
        // msgpack::type::tuple<bool, msgpack::object> tuple;

        o.convert(tuple);

        is_double = tuple.get<0>();
        if (is_double)
        {
            DEBUGOUT << "double before convert" << std::endl;
            tuple.get<1>().convert<double>(value.f);
            DEBUGOUT << "double after convert" << std::endl;
        }
        else
        {
            DEBUGOUT << "int before convert" << std::endl;
            tuple.get<1>().convert<int>(value.i);
            DEBUGOUT << "int after convert" << std::endl;
        }
    }
};

void test(bool value, const std::string& description = "")
{
    if (value)
        TESTOUT << description << ": OK" << std::endl;
    else
        TESTOUT << description << "Fail!" << std::endl;
}

int main()
{
    // double
    try
    {
        constexpr double kEPS = 0.0001;
        TestUnionMemberClass val1(1.0);
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, val1);
        msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
        TestUnionMemberClass val2 = oh.get().as<TestUnionMemberClass>();
        test(val1.is_double && val2.is_double, "is double");
        test(std::fabs(val1.value.f - 1.0) < kEPS, "double == 1.0");
        test(std::fabs(val1.value.f - val2.value.f) < kEPS, "doubles equal");
    }
    catch (const std::exception& e)
    {
        DEBUGOUT << "Exception while double tests: " << e.what() << std::endl;
    }

    // int
    try
    {
        TestUnionMemberClass val1(1);
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, val1);
        msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
        TestUnionMemberClass val2 = oh.get().as<TestUnionMemberClass>();
        test(!val1.is_double && !val2.is_double, "is int");
        test(val1.value.i == 1, "int == 1");
        test(val1.value.i == val2.value.i, "ints equal");
    }
    catch (const std::exception& e)
    {
        DEBUGOUT << "Exception while int tests: " << e.what() << std::endl;
    }
}

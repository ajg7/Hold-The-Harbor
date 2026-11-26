#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <nlohmann/json.hpp>

TEST_CASE("toolchain: C++23 and nlohmann/json are wired up")
{
    static_assert(__cplusplus >= 202302L || _MSVC_LANG > 202002L, "C++23 required");

    const auto j = nlohmann::json::parse(R"({"name":"grunt","health":3})");
    CHECK(j.at("name") == "grunt");
    CHECK(j.at("health").get<int>() == 3);
}

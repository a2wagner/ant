#include "catch.hpp"

#include "analysis/physics/Physics.h"

#include <cassert>
#include <iostream>


using namespace std;
using namespace ant;
using namespace ant::analysis;

void dotest1();
void dotest2();
void dotest3();

TEST_CASE("OptionsList: Basic", "[analysis]") {
    dotest1();
}

TEST_CASE("OptionsList: Chained", "[analysis]") {
    dotest2();
}

TEST_CASE("OptionsList: Flags", "[analysis]") {
    dotest3();
}


void dotest1() {
    std::shared_ptr<OptionsList> opts;

    REQUIRE_NOTHROW(opts = std::make_shared<OptionsList>());
    REQUIRE_NOTHROW(opts->SetOption("key=val"));

    REQUIRE(opts->Get<string>("key")  == "val");
    REQUIRE(opts->Get<string>("key2") == "");
}

void dotest2() {
    auto opts = std::make_shared<OptionsList>();
    opts->SetOption("key=val");

    std::shared_ptr<OptionsList> opts2;
    REQUIRE_NOTHROW(opts2 = std::make_shared<OptionsList>(opts));

    REQUIRE_NOTHROW(opts2->SetOption("key4=val4"));    // own option
    REQUIRE(opts2->Get<string>("key4") == "val4");

    REQUIRE(opts2->Get<string>("key")  == "val"); // from parent
    REQUIRE(opts2->Get<string>("key2") == "");    // from parent

    REQUIRE_NOTHROW(opts2->SetOption("key2=val2"));     // overwrite parent
    REQUIRE(opts2->Get<string>("key2") == "val2");
}

void dotest3() {
    auto opts = std::make_shared<OptionsList>();
    opts->SetOption("flag1=1");
    opts->SetOption("flag2=on");
    opts->SetOption("flag3=oN");
    opts->SetOption("flag4=truE");
    opts->SetOption("flag5=no");
    opts->SetOption("flag6=YES");
    REQUIRE(opts->Get<bool>("flag1"));
    REQUIRE(opts->Get<bool>("flag2"));
    REQUIRE(opts->Get<bool>("flag3"));
    REQUIRE(opts->Get<bool>("flag4"));
    REQUIRE_FALSE(opts->Get<bool>("flag5"));
    REQUIRE(opts->Get<bool>("flag6"));
}

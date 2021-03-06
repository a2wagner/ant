#include "catch.hpp"

#include "reconstruct/UpdateableManager.h"
#include "reconstruct/Reconstruct_traits.h"

using namespace std;
using namespace ant;
using namespace ant::reconstruct;


void dotest1();
void dotest2();
void dotest3();
void dotest4();
void dotest5();
void dotest6();


TEST_CASE("UpdateableManager: No updates at all", "[reconstruct]") {
    dotest1();
}

TEST_CASE("UpdateableManager: Update exactly at start", "[reconstruct]") {
    dotest2();
}

TEST_CASE("UpdateableManager: Start in between", "[reconstruct]") {
    dotest3();
}

TEST_CASE("UpdateableManager: Multiple updates", "[reconstruct]") {
    dotest4();
}

TEST_CASE("UpdateableManager: Unallowed things", "[reconstruct]") {
    dotest5();
}

TEST_CASE("UpdateableManager: Complex items", "[reconstruct]") {
    dotest6();
}

// implement some testable Updateable item
struct UpdateableItem :  Updateable_traits {

    const vector<list<TID>> ChangePoints;
    mutable vector<vector<TID>> UpdatePoints;


    UpdateableItem(vector<list<TID>> changePoints) :
        ChangePoints(changePoints),
        UpdatePoints(changePoints.size())
    {}

    virtual std::list<Loader_t> GetLoaders() override
    {
        std::list<Loader_t> loaders;

        for(size_t i=0; i<ChangePoints.size();i++) {
            auto loader =
                    [i, this]
                    (const TID& currPoint, TID& nextChangePoint) {
                UpdatePoints[i].push_back(currPoint);
                for(auto tid : ChangePoints[i]) {
                    if(tid > currPoint) {
                        nextChangePoint = tid;
                        break;
                    }
                }
            };

            loaders.emplace_back(loader);
        }

        return loaders;
    }

};

// provide some points for testing
const vector<TID> p = {
    {0x00},
    {0x01},
    {0x10},
    {0x11},
    {0x12},
    {0x13}
};

void dotest1()
{
    // do some mocking with many points...
    for(const auto& startPoint : p) {
        for(const auto& updatePoint : p) {

            // item returns no changepoints at all
            const shared_ptr<UpdateableItem>& item =
                    make_shared<UpdateableItem>(vector<list<TID>>{});

            list< shared_ptr<Updateable_traits> > updateables;
            updateables.push_back(item);

            UpdateableManager manager(startPoint, updateables);
            manager.UpdateParameters(updatePoint);

            // check that the item was never updated
            REQUIRE(item->UpdatePoints.empty());
            REQUIRE(item->UpdatePoints.empty());
        }
    }
}

void dotest2()
{
    const shared_ptr<UpdateableItem>& item =
            make_shared<UpdateableItem>(
                vector<list<TID>>{{p[1], p[3], p[5]}});

    list< shared_ptr<Updateable_traits> > updateables;
    updateables.push_back(item);

    UpdateableManager manager(p[3], updateables);
    manager.UpdateParameters(p[3]);

    REQUIRE(item->UpdatePoints.size() == 1);
    REQUIRE(p[3] == item->UpdatePoints.at(0).at(0));
}

void dotest3() {
    const shared_ptr<UpdateableItem>& item =
            make_shared<UpdateableItem>(
                vector<list<TID>>{{p[1], p[3], p[5]}});

    list< shared_ptr<Updateable_traits> > updateables;
    updateables.push_back(item);

    UpdateableManager manager(p[2], updateables);
    manager.UpdateParameters(p[4]);

    REQUIRE(item->UpdatePoints.size() == 1);
    REQUIRE(item->UpdatePoints.at(0).size() == 2);
    REQUIRE(p[2] == item->UpdatePoints.at(0)[0]);
    REQUIRE(p[3] == item->UpdatePoints.at(0)[1]);
}

void dotest4() {
    const shared_ptr<UpdateableItem>& item =
            make_shared<UpdateableItem>(
                vector<list<TID>>{{p[1], p[3], p[5]}});

    list< shared_ptr<Updateable_traits> > updateables;
    updateables.push_back(item);

    UpdateableManager manager(p[2], updateables);

    manager.UpdateParameters(p[4]);
    manager.UpdateParameters(p[5]);

    REQUIRE(item->UpdatePoints.size() == 1);
    REQUIRE(item->UpdatePoints.at(0).size() == 3);
    REQUIRE(p[2] == item->UpdatePoints.at(0)[0]);
    REQUIRE(p[3] == item->UpdatePoints.at(0)[1]);
    REQUIRE(p[5] == item->UpdatePoints.at(0)[2]);
}

void dotest5() {
    const shared_ptr<UpdateableItem>& item =
            make_shared<UpdateableItem>(
                vector<list<TID>>{{p[1], p[3], p[5]}});

    list< shared_ptr<Updateable_traits> > updateables;
    updateables.push_back(item);

    UpdateableManager manager(p[2], updateables);
    manager.UpdateParameters(p[4]);
    manager.UpdateParameters(p[4]);

    REQUIRE(item->UpdatePoints.size() == 1);
    REQUIRE(item->UpdatePoints.at(0).size() == 2);
    REQUIRE(p[2] == item->UpdatePoints.at(0)[0]);
    REQUIRE(p[3] == item->UpdatePoints.at(0)[1]);
}

void dotest6() {
    const shared_ptr<UpdateableItem>& item =
            make_shared<UpdateableItem>(
                vector<list<TID>>{
                    {p[0], p[1], p[2], p[3], p[4], p[5]},
                    {p[0], p[1], p[3]}
                });

    list< shared_ptr<Updateable_traits> > updateables;
    updateables.push_back(item);

    UpdateableManager manager(p[0], updateables);
    manager.UpdateParameters(p[4]);
    manager.UpdateParameters(p.back());

    REQUIRE(item->UpdatePoints.size() == 2);

    REQUIRE(item->UpdatePoints.at(0).size() == 6);
    REQUIRE(item->UpdatePoints.at(1).size() == 3);
    REQUIRE(p[5] == item->UpdatePoints.at(0)[5]);
    REQUIRE(p[3] == item->UpdatePoints.at(1)[2]);


}
#include "catch.hpp"
#include "expconfig_helpers.h"

#include "analysis/physics/Physics.h"

#include "base/OptionsList.h"
#include "base/WrapTFile.h"
#include "base/tmpfile_t.h"

#include "TError.h"

#include <memory>

using namespace std;
using namespace ant;
using namespace ant::analysis;

void dotest();

TEST_CASE("PhysicsRegistry: Create all physics classes", "[analysis]") {
    test::EnsureSetup();
    dotest();
}

bool histogram_overwrite_detected = false;
bool duplicate_mkdir_detected = false;


void dotest() {
    // some errors only appear when some outfiles are present
    tmpfile_t tmpfile;
    auto outfile = std_ext::make_unique<WrapTFileOutput>(tmpfile.filename,
                            WrapTFileOutput::mode_t::recreate,
                            true);
    // overwrite ROOT's error handler to detect some warnings
    SetErrorHandler([] (
                    int level, Bool_t abort, const char *location,
                    const char *msg) {
        // those tests are specific enough...
        if(string(location) == "TDirectory::Append")
            histogram_overwrite_detected = true;
        if(string(location) == "TDirectoryFile::Append")
            histogram_overwrite_detected = true;
        if(string(location) == "TDirectoryFile::mkdir")
            duplicate_mkdir_detected = true;
        DefaultErrorHandler(level, abort, location, msg);
    });

    // create all available physics classes
    for(auto name : PhysicsRegistry::GetList()) {
        histogram_overwrite_detected = false;
        duplicate_mkdir_detected = false;
        INFO(name);
        try {
            PhysicsRegistry::Create(name);
            REQUIRE_FALSE(histogram_overwrite_detected);
            REQUIRE_FALSE(duplicate_mkdir_detected);
        }
        catch(PhysicsRegistry::Exception e) {
            FAIL(string("Physics Registry error: ")+e.what());
        }
        catch(WrapTFile::Exception) {
            // ignore silently if Physics classes can't load some files...
        }
        catch(...) {
            FAIL("Unexpected exception");
        }

    }
    // write the file
    REQUIRE_NOTHROW(outfile = nullptr);

}

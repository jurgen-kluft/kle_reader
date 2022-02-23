#include "xbase/x_target.h"
#include "xbase/x_runes.h"
#include "kle_reader/kle_reader.h"

#include "xunittest/xunittest.h"

using namespace xcore;

UNITTEST_SUITE_BEGIN(reader)
{
    UNITTEST_FIXTURE(read)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(test)
        {
			kle::kb_t keyboard;
			kle::read("data/test.json", keyboard);
        }
    }
}
UNITTEST_SUITE_END
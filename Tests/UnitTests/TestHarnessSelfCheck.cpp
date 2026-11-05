#include "../TestHarness.h"

// Minimal sanity check that the harness itself reports passing assertions.
TEST_CASE("unit.harness")
{
	CHECK(true);
	CHECK_EQ(1 + 1, 2);
	CHECK_EQ(std::string("abc"), std::string("abc"));

	Json::Value jActual(Json::objectValue);
	jActual["value"] = 1;
	Json::Value jExpected(Json::objectValue);
	jExpected["value"] = 1;
	CHECK_JSON_EQ(jActual, jExpected);
}

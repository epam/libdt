#include "time_zone_name_mapping_case.h"
#include <dt-private/tzmapping.h>
#include <libdt/dt_types.h>
#include <set>
#include <vector>
#include <string>
#include <algorithm>

static const char *invalidTimeZone = "Invalid time zone it misspresented in natural world";
static const char *windowsStandardTimeHawaianName = "Hawaiian Standard Time";
static const char *olsenHawaianName = "Pacific/Honolulu";
static const char *abbrTerritory = "001";
static const char *aliasesOlsenForHawaian[] = {"Pacific/Rarotonga", "Pacific/Tahiti", "Pacific/Johnston",
                                               "Pacific/Honolulu", "Etc/GMT+10"
                                              };
static const char *aliasesAbbrForHawaian[] = {"001", "CK", "PF", "UM", "US", "ZZ"};

TimeZoneNameMappingCase::TimeZoneNameMappingCase()
{
}

TEST_F(TimeZoneNameMappingCase, errorHandling)
{
    tz_aliases_t *aliases = NULL;
    tz_alias_t *alias = (tz_alias_t *)&aliases;
    tz_alias_iterator_t *iterator = (tz_alias_iterator_t *)&alias;
    EXPECT_EQ(tzmap_map(NULL, &aliases), DT_INVALID_ARGUMENT);
    EXPECT_EQ(tzmap_map(invalidTimeZone, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(tzmap_map(invalidTimeZone, &aliases), DT_TIMEZONE_NOT_FOUND);
    EXPECT_TRUE(aliases == NULL);
    EXPECT_EQ(tzmap_map(abbrTerritory, &aliases), DT_TIMEZONE_NOT_FOUND);
    EXPECT_TRUE(aliases == NULL);

    EXPECT_EQ(tzmap_free(NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(tzmap_iterate(NULL, &iterator, &alias), DT_INVALID_ARGUMENT);
    aliases = (tz_aliases_t *)&iterator;
    EXPECT_EQ(tzmap_iterate(aliases, NULL, &alias), DT_INVALID_ARGUMENT);
    EXPECT_EQ(tzmap_iterate(aliases, &iterator, NULL), DT_INVALID_ARGUMENT);


}

void testMappingWithHawaianData(const char *tzName)
{
    tz_aliases_t *aliases = NULL;
    bool visitedWindowsStandardTimeAlias = false;
    dt_status_t  status = DT_UNKNOWN_ERROR;
    tz_alias_t *alias = NULL;
    tz_alias_iterator_t *iterator = TZMAP_BEGIN;
    size_t aliasesArrayLength = sizeof(aliasesOlsenForHawaian) / sizeof(char *);
    std::set<std::string> setOlsenForHawaian(aliasesOlsenForHawaian, aliasesOlsenForHawaian + aliasesArrayLength);
    aliasesArrayLength = sizeof(aliasesAbbrForHawaian) / sizeof(char *);
    std::set<std::string> setAbbrForHawaian(aliasesAbbrForHawaian, aliasesAbbrForHawaian + aliasesArrayLength);

    EXPECT_EQ(tzmap_map(tzName, &aliases), DT_OK);
    EXPECT_TRUE(aliases != NULL);
    while ((status = tzmap_iterate(aliases, &iterator, &alias)) == DT_OK) {
        std::set<std::string>::iterator it;
        EXPECT_TRUE(alias != NULL);
        bool finded = false;
        switch (alias->kind) {
            case TZMAP_OLSEN_NAME:
                it = setOlsenForHawaian.find(alias->name);
                if (it != setOlsenForHawaian.end()) {
                    setOlsenForHawaian.erase(it);
                    finded = true;
                }
                break;
            case TZMAP_WIN_STANDARD_TIME:
                if (std::string(alias->name) == std::string(windowsStandardTimeHawaianName)) {
                    finded = true;
                    visitedWindowsStandardTimeAlias = true;
                }
                break;
            case TZMAP_ABBREVIATION:
                it = setAbbrForHawaian.find(alias->name);
                if (it != setAbbrForHawaian.end()) {
                    setAbbrForHawaian.erase(it);
                    finded = true;
                }
                break;
            case TZMAP_UNKNOWN:
            default:
                break;
        }
        EXPECT_TRUE(finded);
    }
    EXPECT_TRUE(visitedWindowsStandardTimeAlias);
    EXPECT_TRUE(setAbbrForHawaian.empty());
    EXPECT_TRUE(setOlsenForHawaian.empty());
    EXPECT_EQ(status, DT_NO_MORE_ITEMS);
    EXPECT_EQ(tzmap_free(aliases), DT_OK);
}

TEST_F(TimeZoneNameMappingCase, fromWindowsStandardTimeMapping)
{
    testMappingWithHawaianData(windowsStandardTimeHawaianName);
}

TEST_F(TimeZoneNameMappingCase, fromOlsenMapping)
{
    testMappingWithHawaianData(olsenHawaianName);
}




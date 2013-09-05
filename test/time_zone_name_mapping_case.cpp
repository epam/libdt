#include "time_zone_name_mapping_case.h"
#include <libtz/tzmapping.h>
#include "libtz/dt_types.h"
#include <set>
#include <vector>
#include <string>
#include <algorithm>

static const char* invalidTimeZone = "Invalid time zone it misspresented in natural world";
static const char* windowsStandardTimeHawaianName = "Hawaiian Standard Time";
static const char* olsenHawaianName = "Pacific/Honolulu";
static const char* abbrHawaianName = "CK";
static const char* aliasesOlsenForHawaian[] = {"Pacific/Rarotonga", "Pacific/Tahiti", "Pacific/Johnston",
 "Pacific/Honolulu", "Etc/GMT+10"};
static const char* aliasesAbbrForHawaian[] = {"CK", "PF", "UM", "US", "ZZ"};

TimeZoneNameMappingCase::TimeZoneNameMappingCase()
{
}

TEST_F(TimeZoneNameMappingCase, variousMapping)
{
    tz_aliases_t *aliases = NULL;
    dt_status_t  status = DT_UNKNOWN_ERROR;
    tz_alias_t *alias = NULL;
    tz_alias_iterator_t *iterator = NULL;
    EXPECT_EQ(tzmap_map(NULL, &aliases), DT_INVALID_ARGUMENT);
    EXPECT_EQ(tzmap_map(invalidTimeZone, NULL), DT_INVALID_ARGUMENT);

    EXPECT_EQ(tzmap_map(invalidTimeZone, &aliases), DT_TIMEZONE_NOT_FOUND);
    EXPECT_TRUE(aliases == NULL);


}

TEST_F(TimeZoneNameMappingCase, fromWindowsStandardTimeMapping)
{
    tz_aliases_t *aliases = NULL;
    dt_status_t  status = DT_UNKNOWN_ERROR;
    tz_alias_t *alias = NULL;
    tz_alias_iterator_t *iterator = NULL;
    size_t aliasesArrayLength = sizeof(aliasesOlsenForHawaian) / sizeof(char *);
    std::set<std::string> setOlsenForHawaian(aliasesOlsenForHawaian, aliasesOlsenForHawaian + aliasesArrayLength);
    aliasesArrayLength = sizeof(aliasesAbbrForHawaian) / sizeof(char *);
    std::set<std::string> setAbbrForHawaian(aliasesAbbrForHawaian, aliasesAbbrForHawaian + aliasesArrayLength);

    EXPECT_EQ(tzmap_map(windowsStandardTimeHawaianName, &aliases), DT_OK);
    EXPECT_TRUE(aliases != NULL);
    while((status = tzmap_iterate(aliases, &iterator, &alias)) == DT_OK && iterator != NULL) {
        std::set<std::string>::iterator it;
        EXPECT_TRUE(alias != NULL);
        bool finded = false;
        switch (alias->kind) {
        case TZA_OLSEN_NAME:
        it = setOlsenForHawaian.find(alias->name);
        if (it != setOlsenForHawaian.end()) {
            setOlsenForHawaian.erase(it);
            finded = true;
        }
        break;
    case TZA_WIN_STANDARD_TIME:
            if (std::string(alias->name) == std::string(windowsStandardTimeHawaianName)) {
                finded = true;
            }
        break;
    case TZA_ABBREVIATION:
        it = setAbbrForHawaian.find(alias->name);
        if (it != setAbbrForHawaian.end()) {
            setAbbrForHawaian.erase(it);
            finded = true;
        }
            break;
        case TZA_UNKNOWN:
        default:
            break;
        }
        EXPECT_TRUE(finded);
    }
    EXPECT_EQ(status, DT_NO_MORE_ITEMS);
}

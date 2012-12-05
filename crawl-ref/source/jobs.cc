#include "AppHdr.h"

#include "jobs.h"

#include "libutil.h"

static const char * Job_Abbrev_List[ NUM_JOBS ] =
    { "Fi", "Wz", "Pr",
      "Gl", "Ne",
      "As", "Be", "Hu",
      "Cj", "En", "FE", "IE", "Su", "AE", "EE", "Sk",
      "VM",
      "CK", "Tm", "He",
#if TAG_MAJOR_VERSION == 34
      "St",
#endif
      "Mo", "Wr", "Wn", "Ar", "AM",
      "DK", "AK" };

static const char * Job_Name_List[ NUM_JOBS ] =
    { M_("Fighter"), M_("Wizard"), M_("Priest"),
      M_("Gladiator"), M_("Necromancer"),
      M_("Assassin"), M_("Berserker"), M_("Hunter"), M_("Conjurer"), M_("Enchanter"),
      M_("Fire Elementalist"), M_("Ice Elementalist"), M_("Summoner"), M_("Air Elementalist"),
      M_("Earth Elementalist"), M_("Skald"),
      M_("Venom Mage"),
      M_("Chaos Knight"), M_("Transmuter"), M_("Healer"),
#if TAG_MAJOR_VERSION == 34
      M_("Stalker"),
#endif
      M_("Monk"), M_("Warper"), M_("Wanderer"), M_("Artificer"), M_("Arcane Marksman"),
      M_("Death Knight"), M_("Abyssal Knight") };

const char *get_job_abbrev(int which_job)
{
    if (which_job == JOB_UNKNOWN)
        return "Un";
    COMPILE_CHECK(ARRAYSZ(Job_Abbrev_List) == NUM_JOBS);
    ASSERT(which_job >= 0 && which_job < NUM_JOBS);

    return Job_Abbrev_List[which_job];
}

job_type get_job_by_abbrev(const char *abbrev)
{
    int i;

    for (i = 0; i < NUM_JOBS; i++)
    {
        // This assumes untranslated abbreviations.
        if (toalower(abbrev[0]) == toalower(Job_Abbrev_List[i][0])
            && toalower(abbrev[1]) == toalower(Job_Abbrev_List[i][1]))
        {
            break;
        }
    }

    return ((i < NUM_JOBS) ? static_cast<job_type>(i) : JOB_UNKNOWN);
}

const char *get_job_name(int which_job)
{
    if (which_job == JOB_UNKNOWN)
        return M_("Unemployed");
    COMPILE_CHECK(ARRAYSZ(Job_Name_List) == NUM_JOBS);
    ASSERT(which_job >= 0 && which_job < NUM_JOBS);

    return Job_Name_List[which_job];
}

job_type get_job_by_name(const char *name)
{
    int i;
    job_type cl = JOB_UNKNOWN;

    string low_name = lowercase_string(name);

    for (i = 0; i < NUM_JOBS; i++)
    {
        string low_job = lowercase_string(Job_Name_List[i]);

        size_t pos = low_job.find(low_name);
        if (pos != string::npos)
        {
            cl = static_cast<job_type>(i);
            if (!pos)  // prefix takes preference
                break;
        }
    }

    return cl;
}

bool is_valid_job(job_type job)
{
    return (job >= 0 && job < NUM_JOBS);
}

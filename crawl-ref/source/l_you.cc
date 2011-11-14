#include "AppHdr.h"

#include "cluautil.h"
#include "l_libs.h"

#include "abl-show.h"
#include "areas.h"
#include "branch.h"
#include "chardump.h"
#include "coord.h"
#include "delay.h"
#include "env.h"
#include "food.h"
#include "initfile.h"
#include "itemname.h"
#include "itemprop.h"
#include "items.h"
#include "libutil.h"
#include "newgame.h"
#include "ng-setup.h"
#include "mapmark.h"
#include "mon-util.h"
#include "mutation.h"
#include "jobs.h"
#include "ouch.h"
#include "religion.h"
#include "shopping.h"
#include "species.h"
#include "skills.h"
#include "skills2.h"
#include "spl-transloc.h"
#include "spl-util.h"
#include "stuff.h"
#include "transform.h"
#include "travel.h"

/*
--- Player-related bindings.

module "you"
*/

/////////////////////////////////////////////////////////////////////
// Bindings to get information on the player (clua).
//

static const char *transform_name()
{
    switch (you.form)
    {
    case TRAN_SPIDER:
        return "spider";
    case TRAN_BAT:
        return "bat";
    case TRAN_BLADE_HANDS:
        return "blade";
    case TRAN_STATUE:
        return "statue";
    case TRAN_ICE_BEAST:
        return "ice";
    case TRAN_DRAGON:
        return "dragon";
    case TRAN_LICH:
        return "lich";
    case TRAN_PIG:
        return "pig";
    case TRAN_APPENDAGE:
        return "appendage";
    default:
        return "";
    }
}

/*
--- Has player done something that takes time?
function turn_is_over() */
LUARET1(you_turn_is_over, boolean, you.turn_is_over)
/*
--- Get player's name
function name() */
LUARET1(you_name, string, you.your_name.c_str())
/*
--- Get name of player's race
function race() */
LUARET1(you_race, string, species_name(you.species).c_str())
/*
--- Get name of player's background
function class() */
LUARET1(you_class, string, get_job_name(you.char_class))
/*
--- Get name of player's god
function god() */
LUARET1(you_god, string, god_name(you.religion).c_str())
/*
--- Is this [player's] god good?
-- @param god defaults to you.god()
function good_god(god) */
LUARET1(you_good_god, boolean,
        lua_isstring(ls, 1) ? is_good_god(str_to_god(lua_tostring(ls, 1)))
        : is_good_god(you.religion))
/*
--- Is this [player's] god evil?
-- @param god defaults to you.god()
function evil_god(god) */
LUARET1(you_evil_god, boolean,
        lua_isstring(ls, 1) ? is_evil_god(str_to_god(lua_tostring(ls, 1)))
        : is_evil_god(you.religion))
/*
--- Does this [player's] god like fresh corpses?
-- @param god defaults to you.god()
function god_likes_fresh_corpses(god) */
LUARET1(you_god_likes_fresh_corpses, boolean,
        lua_isstring(ls, 1) ?
        god_likes_fresh_corpses(str_to_god(lua_tostring(ls, 1))) :
        god_likes_fresh_corpses(you.religion))
LUARET2(you_hp, number, you.hp, you.hp_max)
LUARET2(you_mp, number, you.magic_points, you.max_magic_points)
LUARET1(you_hunger, string, hunger_level())
LUARET2(you_strength, number, you.strength(), you.max_strength())
LUARET2(you_intelligence, number, you.intel(), you.max_intel())
LUARET2(you_dexterity, number, you.dex(), you.max_dex())
LUARET1(you_exp, number, you.experience_level)
LUARET1(you_exp_pool, number, you.exp_available)
LUARET1(you_exp_points, number, you.experience)
LUARET1(you_skill, number,
        lua_isstring(ls, 1) ? you.skills[str_to_skill(lua_tostring(ls, 1))]
                            : 0)
LUARET1(you_skill_progress, number,
        lua_isstring(ls, 1)
            ? get_skill_percentage(str_to_skill(lua_tostring(ls, 1)))
            : 0)
LUARET1(you_res_poison, number, player_res_poison(false))
LUARET1(you_res_fire, number, player_res_fire(false))
LUARET1(you_res_cold, number, player_res_cold(false))
LUARET1(you_res_draining, number, player_prot_life(false))
LUARET1(you_res_shock, number, player_res_electricity(false))
LUARET1(you_res_statdrain, number, player_sust_abil(false))
LUARET1(you_res_mutation, number, wearing_amulet(AMU_RESIST_MUTATION, false))
LUARET1(you_spirit_shield, number, player_spirit_shield())
LUARET1(you_gourmand, boolean, wearing_amulet(AMU_THE_GOURMAND, false))
LUARET1(you_like_chunks, number, player_likes_chunks(true))
LUARET1(you_saprovorous, number, player_mutation_level(MUT_SAPROVOROUS))
LUARET1(you_levitating, boolean, you.flight_mode() == FL_LEVITATE)
LUARET1(you_flying, boolean, you.flight_mode() == FL_FLY)
LUARET1(you_transform, string, transform_name())
LUARET1(you_berserk, boolean, you.berserk())
LUARET1(you_confused, boolean, you.confused())
LUARET1(you_paralysed, boolean, you.paralysed())
LUARET1(you_asleep, boolean, you.asleep())
LUARET1(you_hasted, boolean, you.duration[DUR_HASTE])
LUARET1(you_slowed, boolean, you.duration[DUR_SLOW])
LUARET1(you_exhausted, boolean, you.duration[DUR_EXHAUSTED])
LUARET1(you_teleporting, boolean, you.duration[DUR_TELEPORT])
LUARET1(you_poisoned, boolean, you.duration[DUR_POISONING])
LUARET1(you_invisible, boolean, you.duration[DUR_INVIS])
LUARET1(you_mesmerised, boolean, you.duration[DUR_MESMERISED])
LUARET1(you_rotting, boolean, you.rotting)
LUARET1(you_silenced, boolean, silenced(you.pos()))
LUARET1(you_sick, boolean, you.disease)
LUARET1(you_deaths, number, you.deaths)
LUARET1(you_lives, number, you.lives)

LUARET1(you_where, string, level_id::current().describe().c_str())
LUARET1(you_branch, string, level_id::current().describe(false, false).c_str())
LUARET1(you_subdepth, number, level_id::current().depth)
// [ds] Absolute depth is 1-based for Lua to match things like DEPTH:
// which are also 1-based. Yes, this is confusing. FIXME: eventually
// change you.absdepth0 to be 1-based as well.
LUARET1(you_absdepth, number, you.absdepth0 + 1)
LUAWRAP(you_stop_activity, interrupt_activity(AI_FORCE_INTERRUPT))
LUARET1(you_taking_stairs, boolean,
        current_delay_action() == DELAY_ASCENDING_STAIRS
        || current_delay_action() == DELAY_DESCENDING_STAIRS)
LUARET1(you_turns, number, you.num_turns)
LUARET1(you_time, number, you.elapsed_time)
LUARET1(you_can_smell, boolean, you.can_smell())
LUARET1(you_has_claws, number, you.has_claws(false))
LUARET1(you_level_type_tag, string, you.level_type_tag.c_str())

LUARET1(you_see_cell_rel, boolean,
        you.see_cell(coord_def(luaL_checkint(ls, 1), luaL_checkint(ls, 2)) + you.pos()))
LUARET1(you_see_cell_no_trans_rel, boolean,
        you.see_cell_no_trans(coord_def(luaL_checkint(ls, 1), luaL_checkint(ls, 2)) + you.pos()))
LUARET1(you_piety_rank, number, piety_rank(you.piety) - 1)
LUARET1(you_max_burden, number, carrying_capacity(BS_UNENCUMBERED))
LUARET1(you_burden, number, you.burden)

static int l_you_genus(lua_State *ls)
{
    bool plural = lua_toboolean(ls, 1);
    std::string genus = species_name(you.species, true);
    lowercase(genus);
    if (plural)
        genus = pluralise(PLU_DEFAULT, genus);
    lua_pushstring(ls, genus.c_str());
    return (1);
}

void lua_push_floor_items(lua_State *ls, int link);
static int you_floor_items(lua_State *ls)
{
    lua_push_floor_items(ls, env.igrid(you.pos()));
    return (1);
}

static int l_you_spells(lua_State *ls)
{
    lua_newtable(ls);
    int index = 0;
    for (int i = 0; i < 52; ++i)
    {
        const spell_type spell = get_spell_by_letter(index_to_letter(i));
        if (spell == SPELL_NO_SPELL)
            continue;

        lua_pushstring(ls, spell_title(spell));
        lua_rawseti(ls, -2, ++index);
    }
    return (1);
}

static int l_you_spell_letters(lua_State *ls)
{
    lua_newtable(ls);
    int index = 0;

    char buf[2];
    buf[1] = 0;

    for (int i = 0; i < 52; ++i)
    {
        buf[0] = index_to_letter(i);
        const spell_type spell = get_spell_by_letter(buf[0]);
        if (spell == SPELL_NO_SPELL)
            continue;

        lua_pushstring(ls, buf);
        lua_rawseti(ls, -2, ++index);
    }
    return (1);
}

static int l_you_abils(lua_State *ls)
{
    lua_newtable(ls);

    std::vector<const char *>abils = get_ability_names();
    for (int i = 0, size = abils.size(); i < size; ++i)
    {
        lua_pushstring(ls, abils[i]);
        lua_rawseti(ls, -2, i + 1);
    }
    return (1);
}

static int l_you_abil_letters(lua_State *ls)
{
    lua_newtable(ls);

    char buf[2];
    buf[1] = 0;

    std::vector<talent> talents = your_talents(false);
    for (int i = 0, size = talents.size(); i < size; ++i)
    {
        buf[0] = talents[i].hotkey;
        lua_pushstring(ls, buf);
        lua_rawseti(ls, -2, i + 1);
    }
    return (1);
}

static int you_can_consume_corpses(lua_State *ls)
{
    lua_pushboolean(ls,
                    can_ingest(OBJ_FOOD, FOOD_CHUNK, true, false)
                    || can_ingest(OBJ_CORPSES, CORPSE_BODY, true, false)
                  );
    return (1);
}

LUAFN(you_mutation)
{
    std::string mutname = luaL_checkstring(ls, 1);
    for (int i = 0; i < NUM_MUTATIONS; ++i)
    {
        mutation_type mut = static_cast<mutation_type>(i);
        if (!is_valid_mutation(mut))
            continue;

        const mutation_def& mdef = get_mutation_def(mut);
        if (!strcmp(mutname.c_str(), mdef.wizname))
        {
            PLUARET(integer, you.mutation[mut]);
        }
    }

    std::string err = make_stringf("No such mutation: '%s'.", mutname.c_str());
    return (luaL_argerror(ls, 1, err.c_str()));
}

static const struct luaL_reg you_clib[] =
{
    { "turn_is_over", you_turn_is_over },
    { "turns"       , you_turns },
    { "time"        , you_time },
    { "spells"      , l_you_spells },
    { "spell_letters", l_you_spell_letters },
    { "abilities"   , l_you_abils },
    { "ability_letters", l_you_abil_letters },
    { "name"        , you_name },
    { "race"        , you_race },
    { "class"       , you_class },
    { "genus"       , l_you_genus },
    { "god"         , you_god },
    { "good_god"    , you_good_god },
    { "evil_god"    , you_evil_god },
    { "hp"          , you_hp },
    { "mp"          , you_mp },
    { "hunger"      , you_hunger },
    { "strength"    , you_strength },
    { "intelligence", you_intelligence },
    { "dexterity"   , you_dexterity },
    { "skill"       , you_skill },
    { "skill_progress", you_skill_progress },
    { "xl"          , you_exp },
    { "exp_pool"    , you_exp_pool },
    { "exp"         , you_exp_points },
    { "res_poison"  , you_res_poison },
    { "res_fire"    , you_res_fire   },
    { "res_cold"    , you_res_cold   },
    { "res_draining", you_res_draining },
    { "res_shock"   , you_res_shock },
    { "res_statdrain", you_res_statdrain },
    { "res_mutation", you_res_mutation },
    { "spirit_shield", you_spirit_shield },
    { "saprovorous",  you_saprovorous },
    { "like_chunks",  you_like_chunks },
    { "gourmand",     you_gourmand },
    { "levitating",   you_levitating },
    { "flying",       you_flying },
    { "transform",    you_transform },
    { "berserk",      you_berserk },
    { "confused",     you_confused },
    { "paralysed",    you_paralysed },
    { "asleep",       you_asleep },
    { "hasted",       you_hasted },
    { "slowed",       you_slowed },
    { "exhausted",    you_exhausted },
    { "teleporting",  you_teleporting },
    { "poisoned",     you_poisoned },
    { "invisible",    you_invisible },
    { "mesmerised",   you_mesmerised },
    { "rotting",      you_rotting },
    { "silenced",     you_silenced },
    { "sick",         you_sick },
    { "deaths",       you_deaths },
    { "lives",        you_lives },
    { "piety_rank",   you_piety_rank },
    { "max_burden",   you_max_burden },
    { "burden",       you_burden },

    { "god_likes_fresh_corpses",  you_god_likes_fresh_corpses },
    { "can_consume_corpses",      you_can_consume_corpses },

    { "stop_activity", you_stop_activity },
    { "taking_stairs", you_taking_stairs },

    { "floor_items",  you_floor_items },

    { "where",        you_where },
    { "branch",       you_branch },
    { "subdepth",     you_subdepth },
    { "absdepth",     you_absdepth },

    { "can_smell",         you_can_smell },
    { "has_claws",         you_has_claws },

    { "level_type_tag",    you_level_type_tag },

    { "see_cell",          you_see_cell_rel },
    { "see_cell_no_trans", you_see_cell_no_trans_rel },

    { "mutation",          you_mutation },

    { NULL, NULL },
};

void cluaopen_you(lua_State *ls)
{
    luaL_openlib(ls, "you", you_clib, 0);
}

/////////////////////////////////////////////////////////////////////
// Player information (dlua). Grid coordinates etc.
//

LUARET1(you_can_hear_pos, boolean,
        player_can_hear(coord_def(luaL_checkint(ls,1), luaL_checkint(ls, 2))))
LUARET1(you_x_pos, number, you.pos().x)
LUARET1(you_y_pos, number, you.pos().y)
LUARET2(you_pos, number, you.pos().x, you.pos().y)

LUARET1(you_see_cell, boolean,
        you.see_cell(coord_def(luaL_checkint(ls, 1), luaL_checkint(ls, 2))))
LUARET1(you_see_cell_no_trans, boolean,
        you.see_cell_no_trans(coord_def(luaL_checkint(ls, 1), luaL_checkint(ls, 2))))

LUARET1(you_piety, number, you.piety)

LUAFN(you_stop_running)
{
    stop_running();

    return (0);
}

LUAFN(you_moveto)
{
    const coord_def place(luaL_checkint(ls, 1), luaL_checkint(ls, 2));
    ASSERT(map_bounds(place));
    you.moveto(place);
    return (0);
}

LUAFN(you_teleport_to)
{
    const coord_def place(luaL_checkint(ls, 1), luaL_checkint(ls, 2));
    bool move_monsters = false;
    if (lua_gettop(ls) == 3)
        move_monsters = lua_toboolean(ls, 3);

    lua_pushboolean(ls, you_teleport_to(place, move_monsters));

    return (1);
}

LUAFN(you_random_teleport)
{
    you_teleport_now(false, false);
    return (0);
}

static int _you_uniques(lua_State *ls)
{
    bool unique_found = false;

    if (lua_gettop(ls) >= 1 && lua_isstring(ls, 1))
        unique_found = you.unique_creatures[get_monster_by_name(lua_tostring(ls, 1))];

    lua_pushboolean(ls, unique_found);
    return (1);
}

LUARET1(you_num_runes, number, runes_in_pack())

static int _you_have_rune(lua_State *ls)
{
    int which_rune = NUM_RUNE_TYPES;
    if (lua_gettop(ls) >= 1 && lua_isnumber(ls, 1))
        which_rune = luaL_checkint(ls, 1);
    else if (lua_gettop(ls) >= 1 && lua_isstring(ls, 1))
    {
        const char *spec = lua_tostring(ls, 1);
        for (which_rune = 0; which_rune < NUM_RUNE_TYPES; which_rune++)
            if (!strcasecmp(spec, rune_type_name(which_rune)))
                break;
    }
    bool have_rune = false;
    if (which_rune >= 0 && which_rune < NUM_RUNE_TYPES)
        have_rune = you.runes[which_rune];
    lua_pushboolean(ls, have_rune);
    return (1);
}

static int _you_gold(lua_State *ls)
{
    if (lua_gettop(ls) >= 1)
    {
        const int new_gold = luaL_checkint(ls, 1);
        const int old_gold = you.gold;
        you.set_gold(std::max(new_gold, 0));
        if (new_gold > old_gold)
            you.attribute[ATTR_GOLD_FOUND] += new_gold - old_gold;
        else if (old_gold > new_gold)
            you.attribute[ATTR_MISC_SPENDING] += old_gold - new_gold;
    }
    PLUARET(number, you.gold);
}

LUAWRAP(_you_die,ouch(INSTANT_DEATH, NON_MONSTER, KILLED_BY_SOMETHING))

LUAFN(you_in_branch)
{
    const char* name = luaL_checkstring(ls, 1);

    int br = NUM_BRANCHES;

    for (int i = 0; i < NUM_BRANCHES; i++)
    {
        if (strcasecmp(name, branches[i].shortname) == 0
            || strcasecmp(name, branches[i].longname) == 0
            || strcasecmp(name, branches[i].abbrevname) == 0)
        {
            if (br != NUM_BRANCHES)
            {
                std::string err = make_stringf(
                    "'%s' matches both branch '%s' and '%s'",
                    name, branches[br].abbrevname,
                    branches[i].abbrevname);
                return (luaL_argerror(ls, 1, err.c_str()));
            }
            br = i;
        }
    }

    if (br == NUM_BRANCHES)
    {
        std::string err = make_stringf("'%s' matches no branches.", name);
        return (luaL_argerror(ls, 1, err.c_str()));
    }

    bool in_branch = (br == you.where_are_you);
    PLUARET(boolean, in_branch);
}

LUAFN(_you_shopping_list_has)
{
    const char *thing = luaL_checkstring(ls, 1);
    MAPMARKER(ls, 2, mark);

    level_pos pos(level_id::current(), mark->pos);
    bool has = shopping_list.is_on_list(thing, &pos);
    PLUARET(boolean, has);
}

LUAFN(_you_shopping_list_add)
{
    const char *thing = luaL_checkstring(ls, 1);
    const char *verb  = luaL_checkstring(ls, 2);
    const int  cost   = luaL_checkint(ls, 3);
    MAPMARKER(ls, 4, mark);

    level_pos pos(level_id::current(), mark->pos);
    bool added = shopping_list.add_thing(thing, verb, cost, &pos);
    PLUARET(boolean, added);
}

LUAFN(_you_shopping_list_del)
{
    const char *thing = luaL_checkstring(ls, 1);
    MAPMARKER(ls, 2, mark);

    level_pos pos(level_id::current(), mark->pos);
    bool deleted = shopping_list.del_thing(thing, &pos);
    PLUARET(boolean, deleted);
}

LUAFN(_you_at_branch_bottom)
{
    PLUARET(boolean, at_branch_bottom());
}

LUAWRAP(you_gain_exp, gain_exp(luaL_checkint(ls, 1)))

/*
 * Init the player class.
 *
 * @param combo a string with the species and background abbreviations to use.
 * @param weapon optional string with the weapon to give.
 * @return a string with the weapon skill name.
 */
LUAFN(you_init)
{
    const std::string combo = luaL_checkstring(ls, 1);
    newgame_def ng;
    ng.type = GAME_TYPE_NORMAL;
    ng.species = get_species_by_abbrev(combo.substr(0, 2).c_str());
    ng.job = get_job_by_abbrev(combo.substr(2, 2).c_str());
    ng.weapon = str_to_weapon(luaL_checkstring(ls, 2));
    setup_game(ng);
    you.save->unlink();
    you.save = NULL;
    PLUARET(string, skill_name(weapon_skill(OBJ_WEAPONS, ng.weapon)));
}

LUARET1(you_exp_needed, number, exp_needed(luaL_checkint(ls, 1)));
LUAWRAP(you_exercise, exercise(str_to_skill(luaL_checkstring(ls, 1)), 1));
LUARET1(you_skill_cost_level, number, you.skill_cost_level);
LUARET1(you_skill_points, number,
        you.skill_points[str_to_skill(luaL_checkstring(ls, 1))]);

static const struct luaL_reg you_dlib[] =
{
{ "hear_pos",           you_can_hear_pos },
{ "silenced",           you_silenced },
{ "x_pos",              you_x_pos },
{ "y_pos",              you_y_pos },
{ "pos",                you_pos },
{ "moveto",             you_moveto },
{ "see_cell",           you_see_cell },
{ "see_cell_no_trans",  you_see_cell_no_trans },
{ "random_teleport",    you_random_teleport },
{ "teleport_to",        you_teleport_to },
{ "gold",               _you_gold },
{ "uniques",            _you_uniques },
{ "num_runes",          you_num_runes },
{ "have_rune",          _you_have_rune },
{ "die",                _you_die },
{ "piety",              you_piety },
{ "in_branch",          you_in_branch },
{ "shopping_list_has",  _you_shopping_list_has },
{ "shopping_list_add",  _you_shopping_list_add },
{ "shopping_list_del",  _you_shopping_list_del },
{ "stop_running",       you_stop_running },
{ "at_branch_bottom",   _you_at_branch_bottom },
{ "gain_exp",           you_gain_exp },
{ "init",               you_init },
{ "exp_needed",         you_exp_needed },
{ "exercise",           you_exercise },
{ "skill_cost_level",   you_skill_cost_level },
{ "skill_points",       you_skill_points },

{ NULL, NULL }
};

void dluaopen_you(lua_State *ls)
{
    luaL_openlib(ls, "you", you_dlib, 0);
}

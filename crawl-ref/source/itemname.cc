/**
 * @file
 * @brief Misc functions.
**/

#include "AppHdr.h"

#include "itemname.h"

#include <sstream>
#include <iomanip>
#include <ctype.h>
#include <string.h>

#include "clua.h"

#include "externs.h"
#include "options.h"

#include "art-enum.h"
#include "artefact.h"
#include "colour.h"
#include "decks.h"
#include "describe.h"
#include "food.h"
#include "goditem.h"
#include "invent.h"
#include "item_use.h"
#include "itemprop.h"
#include "items.h"
#include "libutil.h"
#include "makeitem.h"
#include "mon-util.h"
#include "notes.h"
#include "player.h"
#include "religion.h"
#include "quiver.h"
#include "shopping.h"
#include "showsymb.h"
#include "skills2.h"
#include "spl-book.h"
#include "spl-summoning.h"
#include "state.h"
#include "stuff.h"
#include "throw.h"
#include "transform.h"


static bool _is_random_name_space(char let);
static bool _is_random_name_vowel(char let);

static char _random_vowel(int seed);
static char _random_cons(int seed);

static bool translate_flag;

static const char * check_gettext(const char *ptr)
{
    if(translate_flag == false || ptr[0] == 0) return ptr;
    return gettext(ptr);
}

bool is_vowel(const ucs_t chr)
{
    const char low = towlower(chr);
    return (low == 'a' || low == 'e' || low == 'i' || low == 'o' || low == 'u');
}

// quant_name is useful since it prints out a different number of items
// than the item actually contains.
string quant_name(const item_def &item, int quant,
                  description_level_type des, bool terse)
{
    // item_name now requires a "real" item, so we'll mangle a tmp
    item_def tmp = item;
    tmp.quantity = quant;

    return tmp.name(true, des, terse);
}

static const char* _interesting_origin(const item_def &item)
{
    if (origin_is_god_gift(item))
        return "god gift";
    switch (item.orig_monnum)
    {
    case MONS_SONJA:
        if (weapon_skill(item) == SK_SHORT_BLADES)
            return "Sonja";
    case MONS_PSYCHE:
        if (item.base_type == OBJ_WEAPONS && item.sub_type == WPN_DAGGER)
            return "Psyche";
    case MONS_DONALD:
        if (item.base_type == OBJ_ARMOUR && item.sub_type == ARM_SHIELD)
            return "Donald";
    default:
        return 0;
    }
}

string item_def::name(bool allow_translate, description_level_type descrip, bool terse, bool ident,
                      bool with_inscription, bool quantity_in_words,
                      iflags_t ignore_flags) const
{
    if (crawl_state.game_is_arena())
    {
        ignore_flags |= ISFLAG_KNOW_PLUSES | ISFLAG_KNOW_CURSE
                        | ISFLAG_COSMETIC_MASK;
    }

    if (descrip == DESC_NONE)
        return "";

    translate_flag = allow_translate;

    ostringstream buff;

    const string auxname = name_aux(descrip, terse, ident, with_inscription,
                                    ignore_flags);

    const bool startvowel     = is_vowel(auxname[0]);

    if (descrip == DESC_INVENTORY_EQUIP || descrip == DESC_INVENTORY)
    {
        if (in_inventory(*this)) // actually in inventory
        {
            buff << index_to_letter(link);
            if (terse)
                buff << ") ";
            else
                buff << " - ";
        }
        else
            descrip = DESC_A;
    }

    if (base_type == OBJ_BOOKS && (ident || item_type_known(*this))
        && book_has_title(*this))
    {
        if (descrip != DESC_DBNAME)
            descrip = DESC_PLAIN;
    }

    if (terse && descrip != DESC_DBNAME)
        descrip = DESC_PLAIN;

    monster_flag_type corpse_flags;

    if ((base_type == OBJ_CORPSES && is_named_corpse(*this)
         && !(((corpse_flags = props[CORPSE_NAME_TYPE_KEY].get_int64())
               & MF_NAME_SPECIES)
              && !(corpse_flags & MF_NAME_DEFINITE))
         && !(corpse_flags & MF_NAME_SUFFIX)
         && !starts_with(get_corpse_name(*this), "shaped "))
        || item_is_orb(*this) || item_is_horn_of_geryon(*this)
        || (ident || item_type_known(*this)) && is_artefact(*this)
            && special != UNRAND_OCTOPUS_KING_RING)
    {
        // Artefacts always get "the" unless we just want the plain name.
        switch (descrip)
        {
        default:
            buff << (!allow_translate ? "the " : ""); // 한글판에서는 아티팩트명 앞에 the를 붙이지 않음. 색깔로 구별이 가능하니, 혼동될일은 없을듯?
        case DESC_PLAIN:
        case DESC_DBNAME:
        case DESC_BASENAME:
        case DESC_QUALNAME:
            break;
        }
    }
    else if (quantity > 1)
    {
        switch (descrip)
        {
        case DESC_THE:        buff << check_gettext("the "); break;
        case DESC_YOUR:       buff << check_gettext("your "); break;
        case DESC_ITS:        buff << check_gettext("its "); break;
        case DESC_A:
        case DESC_INVENTORY_EQUIP:
        case DESC_INVENTORY:
        case DESC_PLAIN:
        default:
            break;
        }

        if (descrip != DESC_BASENAME && descrip != DESC_QUALNAME
            && descrip != DESC_DBNAME)
        {
            if (quantity_in_words)
                buff << number_in_words(quantity) << (!allow_translate ? " " : ""); // (130204) 재작업함. 
            else
                buff << quantity << (!allow_translate ? " " : ""); // (deceit, 110903) 공백을 삭제해야 할 부분. (110912) 이와같이 수정.
        }
    }
    else
    {
        #ifdef KR
        if(translate_flag == false) 
        {
        #endif
        switch(descrip)
        {
            case DESC_THE:        buff << check_gettext("the "); break;
            case DESC_YOUR:       buff << check_gettext("your "); break;
            case DESC_ITS:        buff << check_gettext("its "); break;
            case DESC_A:
            case DESC_INVENTORY_EQUIP:
            case DESC_INVENTORY:
                                  buff << (startvowel ? check_gettext("an ") : check_gettext("a ")); break;
            case DESC_PLAIN:
            default:
                break;
        }
        #ifdef KR
        }
        #endif
    }

    buff << auxname;

    bool equipped = false;
    if (descrip == DESC_INVENTORY_EQUIP)
    {
        equipment_type eq = item_equip_slot(*this);
        if (eq != EQ_NONE)
        {
            if (you.melded[eq])
                buff << check_gettext(M_(" (melded)"));
            else
            {
                switch (eq)
                {
                case EQ_WEAPON:
                    if (base_type == OBJ_WEAPONS || base_type == OBJ_STAVES
                        || base_type == OBJ_RODS)
                    {
                        buff << check_gettext(M_(" (weapon)"));
                    }
                    else if (you.species == SP_FELID)
                        buff << check_gettext(M_(" (in mouth)")); // (130204,DECEIT) 여기까진 예전 번역물을 쓸 수 있으니 check_gettext를 쓰고...
                    else
                        buff << ((!allow_translate) ? " (in " : "") << (!allow_translate ? you.hand_name(false) : gettext(you.hand_name(false).c_str())) << (!allow_translate ? ")" : "에 쥠");
                    break; // (130204,DECEIT) 이렇게 복잡한 부분은 걍 이렇게 처리... 
                case EQ_CLOAK:
                case EQ_HELMET:
                case EQ_GLOVES:
                case EQ_BOOTS:
                case EQ_SHIELD:
                case EQ_BODY_ARMOUR:
                    buff << check_gettext(M_(" (worn)"));
                    break;
                case EQ_LEFT_RING:
                case EQ_RIGHT_RING:
                case EQ_RING_ONE:
                case EQ_RING_TWO:
                    buff << " (";
                    buff << ((eq == EQ_LEFT_RING || eq == EQ_RING_ONE)
                             ? check_gettext(M_("left")) : check_gettext(M_("right"))); // 돌죽개발자 이살람들 왜이렇게 표현을 바꿔대는것이여ㅡㅡ
                    buff << " ";
                    buff << (!allow_translate ? you.hand_name(false) : gettext(you.hand_name(false).c_str())) ; 
                    buff << ")";
                    break;
                case EQ_AMULET:
                    if (you.species == SP_OCTOPODE && form_keeps_mutations())
                        buff << check_gettext(M_(" (around mantle)")); 
                    else
                        buff << check_gettext(M_(" (around neck)")); 
                    break;
                case EQ_RING_THREE:
                case EQ_RING_FOUR:
                case EQ_RING_FIVE:
                case EQ_RING_SIX:
                case EQ_RING_SEVEN:
                case EQ_RING_EIGHT:
                    buff << check_gettext(M_(" (on tentacle)")); 
                    break;
                default:
                    die("Item in an invalid slot");
                }
            }
        }
        else if (item_is_quivered(*this))
        {
            equipped = true;
            buff << check_gettext(M_(" (quivered)")); 
        }
        else if (item_is_active_manual(*this))
            buff << check_gettext(M_(" (studied)")); 
    }

    if (descrip != DESC_BASENAME && descrip != DESC_DBNAME && with_inscription)
    {
        const bool  tried  =  !ident && !equipped && item_type_tried(*this);
        string tried_str;

        if (tried)
        {
            item_type_id_state_type id_type = get_ident_type(*this);

            if (id_type == ID_MON_TRIED_TYPE)
                tried_str = check_gettext(M_("tried by monster")); 
            else if (id_type == ID_TRIED_ITEM_TYPE)
            {
                tried_str = check_gettext(M_("tried on item")); 
                if (base_type == OBJ_SCROLLS)
                {
                    if (sub_type == SCR_IDENTIFY
                        && you.type_id_props.exists("SCR_ID"))
                    {
                        tried_str = check_gettext(M_("tried on ")) + 
                                    you.type_id_props["SCR_ID"].get_string();
                    }
                    else if (sub_type == SCR_RECHARGING
                             && you.type_id_props.exists("SCR_RC"))
                    {
                        tried_str = check_gettext(M_("tried on ")) + 
                                    you.type_id_props["SCR_RC"].get_string();
                    }
                    else if (sub_type == SCR_ENCHANT_ARMOUR
                             && you.type_id_props.exists("SCR_EA"))
                    {
                        tried_str = check_gettext(M_("tried on ")) + 
                                    you.type_id_props["SCR_EA"].get_string();
                    }
                }
            }
            else
                tried_str = check_gettext(M_("tried")); 
        }

        vector<string> insparts;

        if (tried)
            insparts.push_back(tried_str);

        if (const char *orig = _interesting_origin(*this))
        {
            if (Options.show_god_gift == MB_TRUE
                || Options.show_god_gift == MB_MAYBE && !fully_identified(*this))
            {
                insparts.push_back(orig);
            }
        }

        if (is_artefact(*this))
        {
            string part = artefact_inscription(*this);
            if (!part.empty())
                insparts.push_back(part);
        }

        if (with_inscription && !(inscription.empty()))
            insparts.push_back(inscription);

        if (!insparts.empty())
        {
            buff << " {";

            vector<string>::iterator iter = insparts.begin();

            for (;;)
            {
                buff << *iter;
                if (++iter == insparts.end()) break;
                buff << ", ";
            }

            buff << "}";
        }
    }

    return buff.str();
}

static bool _missile_brand_is_prefix(special_missile_type brand)
{
    switch (brand)
    {
    case SPMSL_POISONED:
    case SPMSL_CURARE:
    case SPMSL_EXPLODING:
    case SPMSL_STEEL:
    case SPMSL_SILVER:
        return true;
    default:
        return false;
    }
}

static bool _missile_brand_is_postfix(special_missile_type brand)
{
    return (brand != SPMSL_NORMAL && !_missile_brand_is_prefix(brand));
}

enum mbn_type
{
    MBN_TERSE, // terse brand name
    MBN_NAME,  // brand name for item naming (adj for prefix, noun for postfix)
    MBN_BRAND, // plain brand name
};

static const char* _missile_brand_name(special_missile_type brand, mbn_type t)
{
    switch (brand)
    {
    case SPMSL_FLAME:
        return M_("flame");
    case SPMSL_FROST:
        return M_("frost");
    case SPMSL_POISONED:
        return (t == MBN_NAME ? M_("poisoned") : M_("poison"));
    case SPMSL_CURARE:
        return (t == MBN_NAME ? M_("curare-tipped") : M_("curare"));
    case SPMSL_EXPLODING:
        return (t == MBN_TERSE ? M_("explode") : M_("exploding"));
    case SPMSL_STEEL:
        return M_("steel");
    case SPMSL_SILVER:
        return M_("silver");
    case SPMSL_PARALYSIS:
        return M_("paralysis");
    case SPMSL_SLOW:
        return (t == MBN_TERSE ? M_("slow") : M_("slowing"));
    case SPMSL_SLEEP:
        return (t == MBN_TERSE ? M_("sleep") : M_("sleeping"));
    case SPMSL_CONFUSION:
        return (t == MBN_TERSE ? M_("conf") : M_("confusion"));
#if TAG_MAJOR_VERSION == 34
    case SPMSL_SICKNESS:
        return (t == MBN_TERSE ? M_("sick") : M_("sickness"));
#endif
    case SPMSL_RAGE:
        return M_("frenzy");
    case SPMSL_RETURNING:
        return (t == MBN_TERSE ? M_("return") : M_("returning"));
    case SPMSL_CHAOS:
        return M_("chaos");
    case SPMSL_PENETRATION:
        return (t == MBN_TERSE ? M_("penet") : M_("penetration"));
    case SPMSL_DISPERSAL:
        return (t == MBN_TERSE ? M_("disperse") : M_("dispersal"));
    case SPMSL_BLINDING:
        return (t == MBN_TERSE ? M_("blind") : M_("blinding"));
    case SPMSL_NORMAL:
        return "";
    default:
        return (t == MBN_TERSE ? "buggy" : "bugginess");
    }
}

const char* weapon_brand_name(const item_def& item, bool terse)
{
    switch (get_weapon_brand(item))
    {
    case SPWPN_NORMAL: return "";
    case SPWPN_FLAMING: return terse ? N_(" (flame)") : N_(" of flaming");
    case SPWPN_FREEZING: return terse ? N_(" (freeze)") : N_(" of freezing");
    case SPWPN_HOLY_WRATH: return terse ? N_(" (holy)") : N_(" of holy wrath");
    case SPWPN_ELECTROCUTION: return terse ? N_(" (elec)"):N_(" of electrocution");
    case SPWPN_ORC_SLAYING: return terse ? N_(" (slay orc)"):N_(" of orc slaying");
    case SPWPN_DRAGON_SLAYING: return terse ? N_(" (slay drac)"):N_(" of dragon slaying");
    case SPWPN_VENOM: return terse ? N_(" (venom)") : N_(" of venom");
    case SPWPN_PROTECTION: return terse ? N_(" (protect)") : N_(" of protection");
    case SPWPN_EVASION: return terse ? N_(" (evade)") : N_(" of evasion");
    case SPWPN_DRAINING: return terse ? N_(" (drain)") : N_(" of draining");
    case SPWPN_SPEED: return terse ? N_(" (speed)") : N_(" of speed");
    case SPWPN_PAIN: return terse ? N_(" (pain)") : N_(" of pain");
    case SPWPN_DISTORTION: return terse ? N_(" (distort)") : N_(" of distortion");
    case SPWPN_REACHING: return terse ? N_(" (reach)") : N_(" of reaching");
    case SPWPN_RETURNING: return terse ? N_(" (return)") : N_(" of returning");

    case SPWPN_VAMPIRICISM:
        return terse ? N_(" (vamp)") : ""; // non-terse already handled

    case SPWPN_VORPAL:
        if (is_range_weapon(item))
            return terse ? N_(" (velocity)") : N_(" of velocity");
        else
        {
            switch (get_vorpal_type(item))
            {
            case DVORP_CRUSHING: return terse ? N_(" (crush)") :N_(" of crushing");
            case DVORP_SLICING:  return terse ? N_(" (slice)") : N_(" of slicing");
            case DVORP_PIERCING: return terse ? N_(" (pierce)"):N_(" of piercing");
            case DVORP_CHOPPING: return terse ? N_(" (chop)") : N_(" of chopping");
            case DVORP_SLASHING: return terse ? N_(" (slash)") :N_(" of slashing");
            case DVORP_STABBING: return terse ? N_(" (stab)") : N_(" of stabbing");
            default:             return terse ? " (buggy vorpal)"
                                              : " of buggy destruction";
            }
        }
    case SPWPN_ANTIMAGIC: return terse ? N_(" (antimagic)") : ""; // non-terse
                                                      // handled elsewhere

    // ranged weapon brands
    case SPWPN_FLAME: return terse ? N_(" (flame)") : N_(" of flame");
    case SPWPN_FROST: return terse ? N_(" (frost)") : N_(" of frost");
    case SPWPN_PENETRATION: return terse ? N_(" (penet)") : N_(" of penetration");
    case SPWPN_REAPING: return terse ? N_(" (reap)") : N_(" of reaping");

    // both ranged and non-ranged
    case SPWPN_CHAOS: return terse ? N_(" (chaos)") : N_(" of chaos");

    // buggy brands
#if TAG_MAJOR_VERSION == 34
    case SPWPN_CONFUSE: return terse ? " (confuse)" : " of confusion";
#endif
    default: return terse ? " (buggy)" : " of bugginess";
    }
}


const char* armour_ego_name(const item_def& item, bool terse)
{
    if (!terse)
    {
        switch (get_armour_ego_type(item))
        {
        case SPARM_NORMAL:            return "";
        case SPARM_RUNNING:
            // "naga barding of running" doesn't make any sense, and yes,
            // they are possible. The terse ego name for these is {run}
            // still to avoid player confusion, it used to be {sslith}.
            if (item.sub_type == ARM_NAGA_BARDING)
                                      return M_("speedy slithering");
            else
                                      return M_("running");
        case SPARM_FIRE_RESISTANCE:   return M_("fire resistance");
        case SPARM_COLD_RESISTANCE:   return M_("cold resistance");
        case SPARM_POISON_RESISTANCE: return M_("poison resistance");
        case SPARM_SEE_INVISIBLE:     return M_("see invisible");
        case SPARM_DARKNESS:          return M_("darkness");
        case SPARM_STRENGTH:          return M_("strength");
        case SPARM_DEXTERITY:         return M_("dexterity");
        case SPARM_INTELLIGENCE:      return M_("intelligence");
        case SPARM_PONDEROUSNESS:     return M_("ponderousness");
        case SPARM_FLYING:            return M_("flying");
        case SPARM_MAGIC_RESISTANCE:  return M_("magic resistance");
        case SPARM_PROTECTION:        return M_("protection");
        case SPARM_STEALTH:           return M_("stealth");
        case SPARM_RESISTANCE:        return M_("resistance");
        case SPARM_POSITIVE_ENERGY:   return M_("positive energy");
        case SPARM_ARCHMAGI:          return M_("the Archmagi");
        case SPARM_PRESERVATION:      return M_("preservation");
        case SPARM_REFLECTION:        return M_("reflection");
        case SPARM_SPIRIT_SHIELD:     return M_("spirit shield");
        case SPARM_ARCHERY:           return M_("archery");
        default:                      return M_("bugginess");
        }
    }
    else
    {
        switch (get_armour_ego_type(item))
        {
        case SPARM_NORMAL:            return "";
        case SPARM_RUNNING:           return " {run}";
        case SPARM_FIRE_RESISTANCE:   return " {rF+}";
        case SPARM_COLD_RESISTANCE:   return " {rC+}";
        case SPARM_POISON_RESISTANCE: return " {rPois}";
        case SPARM_SEE_INVISIBLE:     return " {SInv}";
        case SPARM_DARKNESS:          return " {darkness}";
        case SPARM_STRENGTH:          return " {Str+3}";
        case SPARM_DEXTERITY:         return " {Dex+3}";
        case SPARM_INTELLIGENCE:      return " {Int+3}";
        case SPARM_PONDEROUSNESS:     return " {ponderous}";
        case SPARM_FLYING:            return " {Fly}";
        case SPARM_MAGIC_RESISTANCE:  return " {MR}";
        case SPARM_PROTECTION:        return " {AC+3}";
        case SPARM_STEALTH:           return " {Stlth+}";
        case SPARM_RESISTANCE:        return " {rC+ rF+}";
        case SPARM_POSITIVE_ENERGY:   return " {rN+}";
        case SPARM_ARCHMAGI:          return " {Archmagi}";
        case SPARM_PRESERVATION:      return " {rCorr, Cons}";
        case SPARM_REFLECTION:        return " {reflect}";
        case SPARM_SPIRIT_SHIELD:     return " {Spirit}";
        case SPARM_ARCHERY:           return " {archer}";
        default:                      return " {buggy}";
        }
    }
}

static const char* _wand_type_name(int wandtype)
{
    switch (static_cast<wand_type>(wandtype))
    {
    case WAND_FLAME:           return M_("flame");
    case WAND_FROST:           return M_("frost");
    case WAND_SLOWING:         return M_("slowing");
    case WAND_HASTING:         return M_("hasting");
    case WAND_MAGIC_DARTS:     return M_("magic darts");
    case WAND_HEAL_WOUNDS:     return M_("heal wounds");
    case WAND_PARALYSIS:       return M_("paralysis");
    case WAND_FIRE:            return ((!translate_flag) ? M_("fire") : "불");
    case WAND_COLD:            return ((!translate_flag) ? M_("cold") : "얼음");
    case WAND_CONFUSION:       return M_("confusion");
    case WAND_INVISIBILITY:    return M_("invisibility");
    case WAND_DIGGING:         return M_("digging");
    case WAND_FIREBALL:        return M_("fireball");
    case WAND_TELEPORTATION:   return M_("teleportation");
    case WAND_LIGHTNING:       return M_("lightning");
    case WAND_POLYMORPH:       return M_("polymorph");
    case WAND_ENSLAVEMENT:     return M_("enslavement");
    case WAND_DRAINING:        return M_("draining");
    case WAND_RANDOM_EFFECTS:  return M_("random effects");
    case WAND_DISINTEGRATION:  return M_("disintegration");
    default:                   return M_("bugginess");
    }
}

static const char* wand_secondary_string(int s)
{
    switch (s)
    {
    case 0:  return "";
    case 1:  return M_("jewelled ");
    case 2:  return M_("curved ");
    case 3:  return M_("long ");
    case 4:  return M_("short ");
    case 5:  return M_("twisted ");
    case 6:  return M_("crooked ");
    case 7:  return M_("forked ");
    case 8:  return M_("shiny ");
    case 9:  return M_("blackened ");
    case 10: return M_("tapered ");
    case 11: return M_("glowing ");
    case 12: return M_("worn ");
    case 13: return M_("encrusted ");
    case 14: return M_("runed ");
    case 15: return M_("sharpened ");
    default: return M_("buggily ");
    }
}

static const char* wand_primary_string(int p)
{
    switch (p)
    {
    case 0:  return M_("iron");
    case 1:  return M_("brass");
    case 2:  return M_("bone");
    case 3:  return M_("wooden");
    case 4:  return M_("copper");
    case 5:  return M_("gold");
    case 6:  return M_("silver");
    case 7:  return M_("bronze");
    case 8:  return M_("ivory");
    case 9:  return M_("glass");
    case 10: return M_("lead");
    case 11: return M_("fluorescent");
    default: return M_("buggy");
    }
}

static const char* potion_type_name(int potiontype)
{
    switch (static_cast<potion_type>(potiontype))
    {
    case POT_CURING:            return M_("curing");
    case POT_HEAL_WOUNDS:       return M_("heal wounds");
    case POT_SPEED:             return M_("speed");
    case POT_MIGHT:             return M_("might");
    case POT_AGILITY:           return M_("agility");
    case POT_BRILLIANCE:        return M_("brilliance");
#if TAG_MAJOR_VERSION == 34
    case POT_GAIN_STRENGTH:     return M_("gain strength");
    case POT_GAIN_DEXTERITY:    return M_("gain dexterity");
    case POT_GAIN_INTELLIGENCE: return M_("gain intelligence");
#endif
    case POT_FLIGHT:            return M_("flight");
    case POT_POISON:            return M_("poison");
    case POT_SLOWING:           return M_("slowing");
    case POT_PARALYSIS:         return M_("paralysis");
    case POT_CONFUSION:         return M_("confusion");
    case POT_INVISIBILITY:      return M_("invisibility");
    case POT_PORRIDGE:          return M_("porridge");
    case POT_DEGENERATION:      return M_("degeneration");
    case POT_DECAY:             return M_("decay");
    case POT_EXPERIENCE:        return M_("experience");
    case POT_MAGIC:             return M_("magic");
    case POT_RESTORE_ABILITIES: return M_("restore abilities");
    case POT_STRONG_POISON:     return M_("strong poison");
    case POT_BERSERK_RAGE:      return M_("berserk rage");
    case POT_CURE_MUTATION:     return M_("cure mutation");
    case POT_MUTATION:          return M_("mutation");
    case POT_BLOOD:             return M_("blood");
    case POT_BLOOD_COAGULATED:  return M_("coagulated blood");
    case POT_RESISTANCE:        return M_("resistance");
    case POT_FIZZING:           return M_("fizzing liquid");
    case POT_BENEFICIAL_MUTATION: return M_("beneficial mutation");
    default:                    return "bugginess";
    }
}

static const char* scroll_type_name(int scrolltype)
{
    switch (static_cast<scroll_type>(scrolltype))
    {
    case SCR_IDENTIFY:           return M_("identify");
    case SCR_TELEPORTATION:      return M_("teleportation");
    case SCR_FEAR:               return M_("fear");
    case SCR_NOISE:              return M_("noise");
    case SCR_REMOVE_CURSE:       return M_("remove curse");
    case SCR_SUMMONING:	         return M_("summoning");
    case SCR_ENCHANT_WEAPON_I:   return M_("enchant weapon I");
    case SCR_ENCHANT_ARMOUR:     return M_("enchant armour");
    case SCR_TORMENT:            return M_("torment");
    case SCR_RANDOM_USELESSNESS: return M_("random uselessness");
    case SCR_CURSE_WEAPON:       return M_("curse weapon");
    case SCR_CURSE_ARMOUR:       return M_("curse armour");
    case SCR_CURSE_JEWELLERY:    return M_("curse jewellery");
    case SCR_IMMOLATION:         return M_("immolation");
    case SCR_BLINKING:           return M_("blinking");
    case SCR_MAGIC_MAPPING:      return M_("magic mapping");
    case SCR_FOG:                return M_("fog");
    case SCR_ACQUIREMENT:        return M_("acquirement");
    case SCR_ENCHANT_WEAPON_II:  return M_("enchant weapon II");
    case SCR_VORPALISE_WEAPON:   return M_("vorpalise weapon");
    case SCR_RECHARGING:         return M_("recharging");
    case SCR_ENCHANT_WEAPON_III: return M_("enchant weapon III");
    case SCR_HOLY_WORD:          return M_("holy word");
    case SCR_VULNERABILITY:      return M_("vulnerability");
    case SCR_SILENCE:            return M_("silence");
    case SCR_AMNESIA:            return M_("amnesia");
    default:                     return M_("bugginess");
    }
}

static const char* jewellery_type_name(int jeweltype)
{
    switch (static_cast<jewellery_type>(jeweltype))
    {
    case RING_REGENERATION:          return M_("ring of regeneration");
    case RING_PROTECTION:            return M_("ring of protection");
    case RING_PROTECTION_FROM_FIRE:  return M_("ring of protection from fire");
    case RING_POISON_RESISTANCE:     return M_("ring of poison resistance");
    case RING_PROTECTION_FROM_COLD:  return M_("ring of protection from cold");
    case RING_STRENGTH:              return M_("ring of strength");
    case RING_SLAYING:               return M_("ring of slaying");
    case RING_SEE_INVISIBLE:         return M_("ring of see invisible");
    case RING_INVISIBILITY:          return M_("ring of invisibility");
    case RING_HUNGER:                return M_("ring of hunger");
    case RING_TELEPORTATION:         return M_("ring of teleportation");
    case RING_EVASION:               return M_("ring of evasion");
    case RING_SUSTAIN_ABILITIES:     return M_("ring of sustain abilities");
    case RING_SUSTENANCE:            return M_("ring of sustenance");
    case RING_DEXTERITY:             return M_("ring of dexterity");
    case RING_INTELLIGENCE:          return M_("ring of intelligence");
    case RING_WIZARDRY:              return M_("ring of wizardry");
    case RING_MAGICAL_POWER:         return M_("ring of magical power");
    case RING_FLIGHT:                return M_("ring of flight");
    case RING_LIFE_PROTECTION:       return M_("ring of life protection");
    case RING_PROTECTION_FROM_MAGIC: return M_("ring of protection from magic");
    case RING_FIRE:                  return M_("ring of fire");
    case RING_ICE:                   return M_("ring of ice");
    case RING_TELEPORT_CONTROL:      return M_("ring of teleport control");
    case AMU_RAGE:              return M_("amulet of rage");
    case AMU_CLARITY:           return M_("amulet of clarity");
    case AMU_WARDING:           return M_("amulet of warding");
    case AMU_RESIST_CORROSION:  return M_("amulet of resist corrosion");
    case AMU_THE_GOURMAND:      return M_("amulet of the gourmand");
    case AMU_CONSERVATION:      return M_("amulet of conservation");
#if TAG_MAJOR_VERSION == 34
    case AMU_CONTROLLED_FLIGHT: return M_("amulet of controlled flight");
#endif
    case AMU_INACCURACY:        return M_("amulet of inaccuracy");
    case AMU_RESIST_MUTATION:   return M_("amulet of resist mutation");
    case AMU_GUARDIAN_SPIRIT:   return M_("amulet of guardian spirit");
    case AMU_FAITH:             return M_("amulet of faith");
    case AMU_STASIS:            return M_("amulet of stasis");
    default: return M_("buggy jewellery");
    }
}

static const char* ring_secondary_string(int s)
{
    switch (s)
    {
    case 1:  return M_("encrusted ");
    case 2:  return M_("glowing ");
    case 3:  return M_("tubular ");
    case 4:  return M_("runed ");
    case 5:  return M_("blackened ");
    case 6:  return M_("scratched ");
    case 7:  return M_("small ");
    case 8:  return M_("large ");
    case 9:  return M_("twisted ");
    case 10: return M_("shiny ");
    case 11: return M_("notched ");
    case 12: return M_("knobbly ");
    default: return "";
    }
}



static const char* ring_primary_string(int p)
{
    switch (p)
    {
    case 0:  return M_("wooden");
    case 1:  return M_("silver");
    case 2:  return M_("golden");
    case 3:  return M_("iron");
    case 4:  return M_("steel");
    case 5:  return M_("tourmaline");
    case 6:  return M_("brass");
    case 7:  return M_("copper");
    case 8:  return M_("granite");
    case 9:  return M_("ivory");
    case 10: return M_("ruby");
    case 11: return M_("marble");
    case 12: return M_("jade");
    case 13: return M_("glass");
    case 14: return M_("agate");
    case 15: return M_("bone");
    case 16: return M_("diamond");
    case 17: return M_("emerald");
    case 18: return M_("peridot");
    case 19: return M_("garnet");
    case 20: return M_("opal");
    case 21: return M_("pearl");
    case 22: return M_("coral");
    case 23: return M_("sapphire");
    case 24: return M_("cabochon");
    case 25: return M_("gilded");
    case 26: return M_("onyx");
    case 27: return M_("bronze");
    case 28: return M_("moonstone");
    default: return M_("buggy");
    }
}

static const char* amulet_secondary_string(int s)
{
    switch (s)
    {
    case 0:  return M_("dented ");
    case 1:  return M_("square ");
    case 2:  return M_("thick ");
    case 3:  return M_("thin ");
    case 4:  return M_("runed ");
    case 5:  return M_("blackened ");
    case 6:  return M_("glowing ");
    case 7:  return M_("small ");
    case 8:  return M_("large ");
    case 9:  return M_("twisted ");
    case 10: return M_("tiny ");
    case 11: return M_("triangular ");
    case 12: return M_("lumpy ");
    default: return "";
    }
}

static const char* amulet_primary_string(int p)
{
    switch (p)
    {
    case 0:  return M_("zirconium");
    case 1:  return M_("sapphire");
    case 2:  return M_("golden");
    case 3:  return M_("emerald");
    case 4:  return M_("garnet");
    case 5:  return M_("bronze");
    case 6:  return M_("brass");
    case 7:  return M_("copper");
    case 8:  return M_("ruby");
    case 9:  return M_("ivory");
    case 10: return M_("bone");
    case 11: return M_("platinum");
    case 12: return M_("jade");
    case 13: return M_("fluorescent");
    case 14: return M_("crystal");
    case 15: return M_("cameo");
    case 16: return M_("pearl");
    case 17: return M_("blue");
    case 18: return M_("peridot");
    case 19: return M_("jasper");
    case 20: return M_("diamond");
    case 21: return M_("malachite");
    case 22: return M_("steel");
    case 23: return M_("cabochon");
    case 24: return M_("silver");
    case 25: return M_("soapstone");
    case 26: return M_("lapis lazuli");
    case 27: return M_("filigree");
    case 28: return M_("beryl");
    default: return M_("buggy");
    }
}

const char* rune_type_name(int p)
{
    switch (static_cast<rune_type>(p))
    {
    case RUNE_DIS:         return M_("iron");
    case RUNE_GEHENNA:     return M_("obsidian");
    case RUNE_COCYTUS:     return M_("icy");
    case RUNE_TARTARUS:    return M_("bone");
    case RUNE_SLIME_PITS:  return M_("slimy");
    case RUNE_VAULTS:      return M_("silver");
    case RUNE_SNAKE_PIT:   return M_("serpentine");
    case RUNE_ELVEN_HALLS: return M_("elven");
    case RUNE_TOMB:        return M_("golden");
    case RUNE_SWAMP:       return M_("decaying");
    case RUNE_SHOALS:      return M_("barnacled");
    case RUNE_SPIDER_NEST: return M_("gossamer");
    case RUNE_FOREST:      return M_("mossy");

    // pandemonium and abyss runes:
    case RUNE_DEMONIC:     return M_("demonic");
    case RUNE_ABYSSAL:     return M_("abyssal");

    // special pandemonium runes:
    case RUNE_MNOLEG:      return M_("glowing");
    case RUNE_LOM_LOBON:   return M_("magical");
    case RUNE_CEREBOV:     return M_("fiery");
    case RUNE_GLOORX_VLOQ: return M_("dark");
    default:               return "buggy";
    }
}

static const char* deck_rarity_name(deck_rarity_type rarity)
{
    switch (rarity)
    {
    case DECK_RARITY_COMMON:    return M_("plain");
    case DECK_RARITY_RARE:      return M_("ornate");
    case DECK_RARITY_LEGENDARY: return M_("legendary");
    default:                    return M_("buggy rarity");
    }
}

static const char* misc_type_name(int type, bool known)
{
    if (!known)
    {
        if (type >= MISC_FIRST_DECK && type <= MISC_LAST_DECK)
            return M_("deck of cards");
    }

    switch (static_cast<misc_item_type>(type))
    {
    case MISC_DECK_OF_ESCAPE:      return M_("deck of escape");
    case MISC_DECK_OF_DESTRUCTION: return M_("deck of destruction");
    case MISC_DECK_OF_DUNGEONS:    return M_("deck of dungeons");
    case MISC_DECK_OF_SUMMONING:   return M_("deck of summonings");
    case MISC_DECK_OF_WONDERS:     return M_("deck of wonders");
    case MISC_DECK_OF_PUNISHMENT:  return M_("deck of punishment");
    case MISC_DECK_OF_WAR:         return M_("deck of war");
    case MISC_DECK_OF_CHANGES:     return M_("deck of changes");
    case MISC_DECK_OF_DEFENCE:     return M_("deck of defence");

    case MISC_CRYSTAL_BALL_OF_ENERGY:    return M_("crystal ball of energy");
    case MISC_BOX_OF_BEASTS:             return M_("box of beasts");
#if TAG_MAJOR_VERSION == 34
    case MISC_EMPTY_EBONY_CASKET:        return M_("empty ebony casket");
#endif
    case MISC_AIR_ELEMENTAL_FAN:         return M_("air elemental fan");
    case MISC_LAMP_OF_FIRE:              return M_("lamp of fire");
    case MISC_LANTERN_OF_SHADOWS:        return M_("lantern of shadows");
    case MISC_HORN_OF_GERYON:            return M_("horn of Geryon");
    case MISC_DISC_OF_STORMS:            return M_("disc of storms");
    case MISC_BOTTLED_EFREET:            return M_("bottled efreet");
    case MISC_STONE_OF_EARTH_ELEMENTALS: return M_("stone of earth elementals");
    case MISC_QUAD_DAMAGE:               return M_("quad damage");

    case MISC_RUNE_OF_ZOT:
    default:
        return M_("buggy miscellaneous item");
    }
}

static const char* book_secondary_string(int s)
{
    switch (s)
    {
    case 0:  return "";
    case 1:  return M_("chunky ");
    case 2:  return M_("thick ");
    case 3:  return M_("thin ");
    case 4:  return M_("wide ");
    case 5:  return M_("glowing ");
    case 6:  return M_("dog-eared ");
    case 7:  return M_("oblong ");
    case 8:  return M_("runed ");
    case 9:  return "";
    case 10: return "";
    case 11: return "";
    default: return "buggily ";
    }
}

static const char* book_primary_string(int p)
{
    switch (p)
    {
    case 0:  return M_("paperback ");
    case 1:  return M_("hardcover ");
    case 2:  return M_("leatherbound ");
    case 3:  return M_("metal-bound ");
    case 4:  return M_("papyrus ");
    case 5:  return "";
    case 6:  return "";
    default: return "buggy ";
    }
}

static const char* _book_type_name(int booktype)
{
    switch (static_cast<book_type>(booktype))
    {
    case BOOK_MINOR_MAGIC:            return M_("Minor Magic");
    case BOOK_CONJURATIONS:           return M_("Conjurations");
    case BOOK_FLAMES:                 return M_("Flames");
    case BOOK_FROST:                  return M_("Frost");
    case BOOK_SUMMONINGS:             return M_("Summonings");
    case BOOK_FIRE:                   return M_("Fire");
    case BOOK_ICE:                    return M_("Ice");
    case BOOK_SPATIAL_TRANSLOCATIONS: return M_("Spatial Translocations");
    case BOOK_ENCHANTMENTS:           return M_("Enchantments");
    case BOOK_TEMPESTS:               return M_("the Tempests");
    case BOOK_DEATH:                  return M_("Death");
    case BOOK_HINDERANCE:             return M_("Hinderance");
    case BOOK_CHANGES:                return M_("Changes");
    case BOOK_TRANSFIGURATIONS:       return M_("Transfigurations");
    case BOOK_WAR_CHANTS:             return M_("War Chants");
    case BOOK_CLOUDS:                 return M_("Clouds");
    case BOOK_NECROMANCY:             return M_("Necromancy");
    case BOOK_CALLINGS:               return M_("Callings");
    case BOOK_MALEDICT:               return M_("Maledictions");
    case BOOK_AIR:                    return ((!translate_flag) ? M_("Air") : "바람");
    case BOOK_SKY:                    return M_("the Sky");
    case BOOK_WARP:                   return M_("the Warp");
    case BOOK_ENVENOMATIONS:          return M_("Envenomations");
    case BOOK_ANNIHILATIONS:          return M_("Annihilations");
    case BOOK_UNLIFE:                 return M_("Unlife");
    case BOOK_CONTROL:                return M_("Control");
#if TAG_MAJOR_VERSION == 34
    case BOOK_MUTATIONS:              return M_("Morphology");
#endif
    case BOOK_GEOMANCY:               return M_("Geomancy");
    case BOOK_EARTH:                  return M_("the Earth");
    case BOOK_WIZARDRY:               return M_("Wizardry");
    case BOOK_POWER:                  return M_("Power");
    case BOOK_CANTRIPS:               return M_("Cantrips");
    case BOOK_PARTY_TRICKS:           return M_("Party Tricks");
#if TAG_MAJOR_VERSION == 34
    case BOOK_STALKING:               return M_("Stalking");
#endif
    case BOOK_DEBILITATION:           return M_("Debilitation");
    case BOOK_DRAGON:                 return M_("the Dragon");
    case BOOK_BURGLARY:               return M_("Burglary");
    case BOOK_DREAMS:                 return M_("Dreams");
    case BOOK_ALCHEMY:                return M_("Alchemy");
    case BOOK_BEASTS:                 return M_("Beasts");
    case BOOK_RANDART_LEVEL:          return M_("Fixed Level");
    case BOOK_RANDART_THEME:          return M_("Fixed Theme");
    default:                          return M_("Bugginess");
    }
}

static const char* staff_secondary_string(int p)
{
    switch (p) // general descriptions
    {
    case 0:  return M_("crooked ");
    case 1:  return M_("knobbly ");
    case 2:  return M_("weird ");
    case 3:  return M_("gnarled ");
    case 4:  return M_("thin ");
    case 5:  return M_("curved ");
    case 6:  return M_("twisted ");
    case 7:  return M_("thick ");
    case 8:  return M_("long ");
    case 9:  return M_("short "); // (130204) 큰 의미는 없지만... 나중에 헷갈릴수 있으므로 M_()으로 감쌌습니다 
    default: return "buggily ";
    }
}

static const char* staff_primary_string(int p)
{
    switch (p) // special attributes
    {
    case 0:  return M_("glowing ");
    case 1:  return M_("jewelled ");
    case 2:  return M_("runed ");
    case 3:  return M_("smoking ");
    default: return "buggy ";
    }
}

static const char* staff_type_name(int stafftype)
{
    switch ((stave_type)stafftype)
    {
    case STAFF_WIZARDRY:    return M_("wizardry");
    case STAFF_POWER:       return M_("power");
    case STAFF_FIRE:        return M_("fire");
    case STAFF_COLD:        return M_("cold");
    case STAFF_POISON:      return M_("poison");
    case STAFF_ENERGY:      return M_("energy");
    case STAFF_DEATH:       return M_("death");
    case STAFF_CONJURATION: return M_("conjuration");
#if TAG_MAJOR_VERSION == 34
    case STAFF_ENCHANTMENT: return M_("enchantment");
#endif
    case STAFF_AIR:         return M_("air");
    case STAFF_EARTH:       return M_("earth");
    case STAFF_SUMMONING:   return M_("summoning");
    default:                return "bugginess";
    }
}

static const char* rod_type_name(int type)
{
    switch ((rod_type)type)
    {
    case ROD_SWARM:           return M_("the swarm");
    case ROD_WARDING:         return M_("warding");
    case ROD_LIGHTNING:       return M_("lightning");
    case ROD_STRIKING:        return M_("striking");
    case ROD_DEMONOLOGY:      return M_("demonology");
    case ROD_VENOM:           return M_("venom");
    case ROD_INACCURACY:      return M_("inaccuracy");

    case ROD_DESTRUCTION_I:
    case ROD_DESTRUCTION_II:
    case ROD_DESTRUCTION_III:
                              return M_("destruction");

    default: return "bugginess";
    }
}

const char* racial_description_string(const item_def& item, bool terse)
{
    switch (get_equip_race(item))
    {
    case ISFLAG_ORCISH:
        return (translate_flag ? "오크제 " : terse ? N_("orc ") : N_("orcish ")); 
    case ISFLAG_ELVEN:
        return (translate_flag ? "엘프제 " : terse ? N_("elf ") : N_("elven ")); 
    case ISFLAG_DWARVEN:
        return (translate_flag ? "드워프제 " : terse ? N_("dwarf ") : N_("dwarven "));
    default:
        return "";
    }
}

string base_type_string(const item_def &item, bool known)
{
    return base_type_string(item.base_type, known);
}

string base_type_string(object_class_type type, bool known)
{
    switch (type)
    {
    case OBJ_WEAPONS: return "weapon";
    case OBJ_MISSILES: return "missile";
    case OBJ_ARMOUR: return "armour";
    case OBJ_WANDS: return "wand";
    case OBJ_FOOD: return "food";
    case OBJ_SCROLLS: return "scroll";
    case OBJ_JEWELLERY: return "jewellery";
    case OBJ_POTIONS: return "potion";
    case OBJ_BOOKS: return "book";
    case OBJ_STAVES: return "staff";
    case OBJ_RODS: return "rod";
    case OBJ_ORBS: return "orb";
    case OBJ_MISCELLANY: return "miscellaneous";
    case OBJ_CORPSES: return "corpse";
    case OBJ_GOLD: return "gold";
    default: return "";
    }
}

string sub_type_string(const item_def &item, bool known)
{
    const object_class_type type = item.base_type;
    const int sub_type = item.sub_type;

    switch (type)
    {
    case OBJ_WEAPONS:  // deliberate fall through, as XXX_prop is a local
    case OBJ_MISSILES: // variable to itemprop.cc.
    case OBJ_ARMOUR:
        return item_base_name(type, sub_type);
    case OBJ_WANDS: return _wand_type_name(sub_type);
    case OBJ_FOOD: return food_type_name(sub_type);
    case OBJ_SCROLLS: return scroll_type_name(sub_type);
    case OBJ_JEWELLERY: return jewellery_type_name(sub_type);
    case OBJ_POTIONS: return potion_type_name(sub_type);
    case OBJ_BOOKS:
    {
        if (sub_type == BOOK_MANUAL)
        {	string bookname;
            if (!known)
                return check_gettext("manual");
            if (!translate_flag) { bookname = "manual of "; bookname += skill_name(static_cast<skill_type>(item.plus)); }
            else { bookname = skill_name(static_cast<skill_type>(item.plus)); bookname += "의 설명서";  }
            return bookname;
        }
        else if (sub_type == BOOK_NECRONOMICON)
            return check_gettext("Necronomicon");
        else if (sub_type == BOOK_GRAND_GRIMOIRE)
            return check_gettext("Grand Grimoire");
        else if (sub_type == BOOK_DESTRUCTION)
            return check_gettext("tome of Destruction");
        else if (sub_type == BOOK_YOUNG_POISONERS)
            return check_gettext("Young Poisoner's Handbook");

        if(!translate_flag) return string("book of ") + _book_type_name(sub_type); else return _(_book_type_name(sub_type)) + string("의 마법서");
    }
    case OBJ_STAVES: return staff_type_name(static_cast<stave_type>(sub_type));
    case OBJ_RODS:   return rod_type_name(static_cast<rod_type>(sub_type));
    case OBJ_MISCELLANY:
        if (sub_type == MISC_RUNE_OF_ZOT)
            return "rune of Zot";
        else
            return misc_type_name(sub_type, known);
    // these repeat as base_type_string
    case OBJ_ORBS: return "orb of Zot"; break;
    case OBJ_CORPSES: return "corpse"; break;
    case OBJ_GOLD: return "gold"; break;
    default: return "";
    }
}

/// lua 스크립트에 넘겨주는 놈이니 건드리지 말긔.
string ego_type_string(const item_def &item)
{
    switch (item.base_type)
    {
    case OBJ_ARMOUR:
        return armour_ego_name(item, false);
    case OBJ_WEAPONS:
        // this is specialcased out of weapon_brand_name
        // ("vampiric hand axe", etc)
        if (get_weapon_brand(item) == SPWPN_VAMPIRICISM)
            return "vampiricism";
        else if (get_weapon_brand(item) == SPWPN_ANTIMAGIC)
            return "anti-magic";
        else if (get_weapon_brand(item) != SPWPN_NORMAL)
            return string(weapon_brand_name(item, false)).substr(4);
        else
            return "";
    case OBJ_MISSILES:
        return _missile_brand_name(get_ammo_brand(item), MBN_BRAND);
    default:
        return "";
    }
}

// nets can go +0 .. -7 (-8 always destroys them)
static const char* _torn_net(int plus)
{
    if (plus >= 0)
        return "";
    else if (plus >= -2)
        return M_(" [frayed]"); 
    else if (plus >= -5)
        return M_(" [torn]"); 
    else
        return M_(" [falling apart]"); 
}

// Note that "terse" is only currently used for the "in hand" listing on
// the game screen.
string item_def::name_aux(description_level_type desc, bool terse, bool ident,
                          bool with_inscription, iflags_t ignore_flags) const
{
    // Shortcuts
    const int item_typ   = sub_type;
    const int it_plus    = plus;
    const int item_plus2 = plus2;

    const bool know_type = ident || item_type_known(*this);

    const bool dbname   = (desc == DESC_DBNAME);
    const bool basename = (desc == DESC_BASENAME || (dbname && !know_type));
    const bool qualname = (desc == DESC_QUALNAME);

    const bool know_curse =
        !basename && !qualname && !dbname
        && !testbits(ignore_flags, ISFLAG_KNOW_CURSE)
        && (ident || item_ident(*this, ISFLAG_KNOW_CURSE));

    const bool __know_pluses =
        !basename && !qualname && !dbname
        && !testbits(ignore_flags, ISFLAG_KNOW_PLUSES)
        && (ident || item_ident(*this, ISFLAG_KNOW_PLUSES));

    const bool know_brand =
        !basename && !qualname && !dbname
        && !testbits(ignore_flags, ISFLAG_KNOW_TYPE)
        && (ident || item_type_known(*this));

    const bool know_ego = know_brand;

    // Display runed/glowing/embroidered etc?
    // Only display this if brand is unknown or item is unbranded.
    const bool show_cosmetic = !__know_pluses && !terse && !basename
        && !qualname && !dbname
        && (!know_brand || !special)
        && !(ignore_flags & ISFLAG_COSMETIC_MASK);

    // So that show_cosmetic won't be affected by ignore_flags.
    const bool know_pluses = __know_pluses
        && !testbits(ignore_flags, ISFLAG_KNOW_PLUSES);

    const bool know_racial = !(ignore_flags & ISFLAG_RACIAL_MASK);

    const bool need_plural = !basename && !dbname;

    ostringstream buff;

    switch (base_type)
    {
    case OBJ_WEAPONS:
        if (know_curse && !terse)
        {
            // We don't bother printing "uncursed" if the item is identified
            // for pluses (its state should be obvious), this is so that
            // the weapon name is kept short (there isn't a lot of room
            // for the name on the main screen).  If you're going to change
            // this behaviour, *please* make it so that there is an option
            // that maintains this behaviour. -- bwr
            // Nor for artefacts. Again, the state should be obvious. --jpeg
            if (cursed())
                buff << check_gettext("cursed ");
            else if (Options.show_uncursed && !know_pluses
                     && (!know_type || !is_artefact(*this)))
                buff << check_gettext("uncursed ");
        }

        if (know_pluses)
        {
            if (is_unrandom_artefact(*this) && special == UNRAND_WOE)
                buff << (terse ? "+∞ " : "+∞,+∞ ");
            else if ((terse && it_plus == item_plus2) || sub_type == WPN_BLOWGUN)
                buff << make_stringf("%+d ", it_plus);
            else
                buff << make_stringf("%+d,%+d ", it_plus, item_plus2);
        }

        if (is_artefact(*this) && !dbname)
        {
            buff << check_gettext(get_artefact_name(*this).c_str());
            break;
        }
        else if (flags & ISFLAG_BLESSED_WEAPON && !dbname)
        {   // Since angels and daevas can get blessed base items, we
            // need a separate flag for this, so they can still have
            // their holy weapons.
            buff << check_gettext("Blessed ");
            if (weapon_skill(*this) == SK_MACES_FLAILS)
                buff << check_gettext(M_("Scourge"));
            else if (weapon_skill(*this) == SK_POLEARMS)
                buff << check_gettext(M_("Trishula"));
            else
                buff << check_gettext(M_("Blade"));
            break;
        }

        if (show_cosmetic)
        {
            switch (get_equip_desc(*this))
            {
            case ISFLAG_RUNED:
                if (!testbits(ignore_flags, ISFLAG_RUNED))
                    buff << check_gettext(M_("runed "));
                break;
            case ISFLAG_GLOWING:
                if (!testbits(ignore_flags, ISFLAG_GLOWING))
                    buff << check_gettext(M_("glowing "));
                break;
            }
        }

        if (!basename && !dbname && know_racial)
        {
            // Always give racial type (it does have game effects).
            buff << check_gettext(racial_description_string(*this, terse));
        }

        if (know_brand && !terse)
        {
            int brand = get_weapon_brand(*this);
            if (brand == SPWPN_VAMPIRICISM)
                buff << check_gettext(M_("vampiric "));
            else if (brand == SPWPN_ANTIMAGIC)
                buff << check_gettext(M_("anti-magic "));
        }

        /// 1. 무기 이름 2. 브랜드( of flaming)의 번역형 3. 저주가 걸려있으면 (curse)
		if(translate_flag)
		{
			if(terse)
			{
				buff << make_stringf("%s%s%s",
					check_gettext(item_base_name(*this).c_str()),
					know_brand ? check_gettext(weapon_brand_name(*this, terse)) : "",
					(know_curse && cursed() && terse) ? check_gettext(" (curse)") : "");
			}
			else
			{
				buff << make_stringf("%s%s%s",
					know_brand ? check_gettext(weapon_brand_name(*this, terse)) : "",
					check_gettext(item_base_name(*this).c_str()),
					(know_curse && cursed() && terse) ? check_gettext(" (curse)") : "");
			}
		}
		else
		{	
			buff << make_stringf("%s%s%s",
				check_gettext(item_base_name(*this).c_str()),
				know_brand ? check_gettext(weapon_brand_name(*this, terse)) : "",
				(know_curse && cursed() && terse) ? check_gettext(" (curse)") : "");
		}

        break;

    case OBJ_MISSILES:
    {
        special_missile_type brand  = get_ammo_brand(*this);

        if (!terse && _missile_brand_is_prefix(brand))
            buff << check_gettext(_missile_brand_name(brand, MBN_NAME)) << ' ';

		if (brand != SPMSL_NORMAL && !basename && !qualname && !dbname && translate_flag && !terse && !_missile_brand_is_prefix(brand))
			buff << _(_missile_brand_name(brand, MBN_NAME)) << " ";

        buff << check_gettext(ammo_name(static_cast<missile_type>(item_typ)));

        if (brand != SPMSL_NORMAL
            && brand != SPMSL_BLINDING
            && !basename && !qualname && !dbname)
        {
            if (terse)
				buff << " (" <<  ((!translate_flag) ? _missile_brand_name(brand, MBN_TERSE) : _(_missile_brand_name(brand, MBN_TERSE)))  << ")";
			else if (_missile_brand_is_postfix(brand) && !translate_flag)
                buff << " of " << _missile_brand_name(brand, MBN_NAME);
        }

        if (item_typ == MI_THROWING_NET && !basename && !qualname && !dbname)
            buff << check_gettext(_torn_net(it_plus));
        break;
    }
    case OBJ_ARMOUR:
        if (know_curse && !terse)
        {
            if (cursed())
                buff << check_gettext("cursed ");
            else if (Options.show_uncursed && !know_pluses)
                buff << check_gettext("uncursed ");
        }

        if (know_pluses)
            buff << make_stringf("%+d ", it_plus);

        if (item_typ == ARM_GLOVES || item_typ == ARM_BOOTS)
            buff << check_gettext("pair of ");

        // When asking for the base item name, randartism is ignored.
        if (is_artefact(*this) && !basename && !dbname)
        {
            buff << check_gettext(get_artefact_name(*this).c_str());
            break;
        }

        if (show_cosmetic)
        {
            switch (get_equip_desc(*this))
            {
            case ISFLAG_EMBROIDERED_SHINY:
                if (testbits(ignore_flags, ISFLAG_EMBROIDERED_SHINY))
                    break;
                if (item_typ == ARM_ROBE || item_typ == ARM_CLOAK
                    || item_typ == ARM_GLOVES || item_typ == ARM_BOOTS
                    || get_armour_slot(*this) == EQ_HELMET
                       && !is_hard_helmet(*this))
                {
                    buff << check_gettext(M_("embroidered "));
                }
                else if (item_typ != ARM_LEATHER_ARMOUR
                         && item_typ != ARM_ANIMAL_SKIN)
                {
                    buff << check_gettext(M_("shiny "));
                }
                else
                    buff << check_gettext(M_("dyed "));
                break;

            case ISFLAG_RUNED:
                if (!testbits(ignore_flags, ISFLAG_RUNED))
                    buff << check_gettext(M_("runed "));
                break;

            case ISFLAG_GLOWING:
                if (!testbits(ignore_flags, ISFLAG_GLOWING))
                    buff << check_gettext(M_("glowing "));
                break;
            }
        }

        if (!basename && !dbname)
        {
            // always give racial description (has game effects)
            buff << check_gettext(racial_description_string(*this, terse));
        }

        if (!basename && !dbname && is_hard_helmet(*this))
        {
            const short dhelm = get_helmet_desc(*this);

            buff <<
                   ((dhelm == THELM_DESC_PLAIN)    ? "" :
                    (dhelm == THELM_DESC_WINGED)   ? check_gettext(M_("winged "))  :
                    (dhelm == THELM_DESC_HORNED)   ? check_gettext(M_("horned "))  :
                    (dhelm == THELM_DESC_CRESTED)  ? check_gettext(M_("crested ")) :
                    (dhelm == THELM_DESC_PLUMED)   ? check_gettext(M_("plumed "))  :
                    (dhelm == THELM_DESC_SPIKED)   ? check_gettext(M_("spiked "))  :
                    (dhelm == THELM_DESC_VISORED)  ? check_gettext(M_("visored ")) :
                    (dhelm == THELM_DESC_GOLDEN)   ? check_gettext(M_("golden "))
                                                   : check_gettext(M_("buggy ")));
        }

		// 어순을 바꿔야 할 곳이라서 코드를 많이 고치게 됨. 
		// (deceit,130316) 여기 뭔가 merging이 잘못되었던듯;;; 원본코드 내려받아서 고쳤습니다.
		{
		std::string itembasename, itemegoname;
		if (!basename && item_typ == ARM_GLOVES)
		{
			const short dglov = get_gloves_desc(*this);

            itembasename = check_gettext((dglov == TGLOV_DESC_GLOVES)    ? M_("gloves") :
                           (dglov == TGLOV_DESC_GAUNTLETS) ? M_("gauntlets") :
                           (dglov == TGLOV_DESC_BRACERS)   ? M_("bracers") :
                                                             M_("bug-ridden gloves"));
		}
		else
            itembasename = check_gettext(item_base_name(*this).c_str());

		if (know_ego && !is_artefact(*this))
		{
			const special_armour_type sparm = get_armour_ego_type(*this);

			if (sparm != SPARM_NORMAL)
			{
				if (!terse)
					itemegoname = make_stringf(check_gettext(" of %s")
					, check_gettext(armour_ego_name(*this, terse)));
				else
					itemegoname = check_gettext(armour_ego_name(*this, terse));
			}
		}

#ifdef KR
            if(translate_flag == false)
                buff << itembasename << itemegoname;
            else
                buff << itemegoname << itembasename;
#else
            buff << itembasename << itemegoname;
#endif
			}

        if (know_curse && cursed() && terse)
            buff << check_gettext(" (curse)"); 
        break;

    case OBJ_WANDS:
        if (basename)
        {
            buff << check_gettext(M_("wand"));
            break;
        }

        if (know_type)
            buff << make_stringf(check_gettext("wand of %s"), check_gettext(_wand_type_name(item_typ)));
        else
        {
            buff << make_stringf(check_gettext("%s%s wand"), 
                check_gettext(wand_secondary_string(special / NDSC_WAND_PRI)),
                check_gettext(wand_primary_string(special % NDSC_WAND_PRI)));
        }

        if (know_pluses)
            buff << " (" << it_plus << ")";
        else if (!dbname && with_inscription)
        {
            if (item_plus2 == ZAPCOUNT_EMPTY)
                buff << check_gettext(" {empty}");
            else if (item_plus2 == ZAPCOUNT_RECHARGED)
                buff << check_gettext(" {recharged}");
            else if (item_plus2 > 0)
                buff << make_stringf(check_gettext(" {zapped: %d}"), item_plus2);
        }
        break;

    case OBJ_POTIONS:
        if (basename)
        {
            buff << check_gettext(M_("potion"));
            break;
        }

        if (know_type)
            /// 1. healing, heal wounds 등 포션에 붙는 말이 번역되서 들어옴. 이를테면
            /// healing = 회복, heal wounds = 상처 치료
            buff << make_stringf(check_gettext("potion of %s"), 
                check_gettext(potion_type_name(item_typ)));
        else
        {
            const int pqual   = PQUAL(plus);
            const int pcolour = PCOLOUR(plus);

            static const char *potion_qualifiers[] = {
                "",  M_("bubbling "), M_("fuming "), M_("fizzy "), M_("viscous "), M_("lumpy "),
                M_("smoky "), M_("glowing "), M_("sedimented "), M_("metallic "), M_("murky "),
                M_("gluggy "), M_("oily "), M_("slimy "), M_("emulsified ")
            };
            COMPILE_CHECK(ARRAYSZ(potion_qualifiers) == PDQ_NQUALS);

            static const char *potion_colours[] = {
#if TAG_MAJOR_VERSION == 34
                M_("clear"),
#endif
                M_("blue"), M_("black"), M_("silvery"), M_("cyan"), M_("purple"), M_("orange"),
                M_("inky"), M_("red"), M_("yellow"), M_("green"), M_("brown"), M_("pink"), M_("white")
            };
            COMPILE_CHECK(ARRAYSZ(potion_colours) == PDC_NCOLOURS);

            const char *qualifier =
                (pqual < 0 || pqual >= PDQ_NQUALS) ? "bug-filled "
                                    : check_gettext(potion_qualifiers[pqual]);

            const char *clr =  (pcolour < 0 || pcolour >= PDC_NCOLOURS) ?
                                   "bogus" : check_gettext(potion_colours[pcolour]);

            /// 1. fuming, slimy 등의 수식어가 번역되서 들어옴. eg> 연기나는, 끈적이는
            /// 2. clear, blue 등의 색깔이 번역되서 들어옴. eg> 맑은, 푸른
            buff << make_stringf(check_gettext("%s%s potion"), qualifier, clr);
        }
        break;

    case OBJ_FOOD:
        switch (item_typ)
        {
        case FOOD_MEAT_RATION: buff << check_gettext(M_("meat ration")); break;
        case FOOD_BREAD_RATION: buff << check_gettext(M_("bread ration")); break;
        case FOOD_PEAR: buff << check_gettext(M_("pear")); break;
        case FOOD_APPLE: buff << check_gettext(M_("apple")); break;
        case FOOD_CHOKO: buff << check_gettext(M_("choko")); break;
        case FOOD_HONEYCOMB: buff << check_gettext(M_("honeycomb")); break;
        case FOOD_ROYAL_JELLY: buff << check_gettext(M_("royal jelly")); break;
        case FOOD_SNOZZCUMBER: buff << check_gettext(M_("snozzcumber")); break;
        case FOOD_PIZZA: buff << check_gettext(M_("slice of pizza")); break;
        case FOOD_APRICOT: buff << check_gettext(M_("apricot")); break;
        case FOOD_ORANGE: buff << ((!translate_flag) ? "orange" : "오렌지"); break;
        case FOOD_BANANA: buff << check_gettext(M_("banana")); break;
        case FOOD_STRAWBERRY: buff << check_gettext(M_("strawberry")); break;
        case FOOD_RAMBUTAN: buff << check_gettext(M_("rambutan")); break;
        case FOOD_LEMON: buff << check_gettext(M_("lemon")); break;
        case FOOD_GRAPE: buff << check_gettext(M_("grape")); break;
        case FOOD_SULTANA: buff << check_gettext(M_("sultana")); break;
        case FOOD_LYCHEE: buff << check_gettext(M_("lychee")); break;
        case FOOD_BEEF_JERKY: buff << check_gettext(M_("beef jerky")); break;
        case FOOD_CHEESE: buff << check_gettext(M_("cheese")); break;
        case FOOD_SAUSAGE: buff << check_gettext(M_("sausage")); break;
        case FOOD_AMBROSIA: buff << check_gettext(M_("piece of ambrosia")); break;
        case FOOD_CHUNK:
            if (!basename && !dbname)
            {
                buff << make_stringf(check_gettext("%schunk of %s flesh"), 
                    (food_is_rotten(*this) && it_plus != MONS_ROTTING_HULK) ? check_gettext(M_("rotting ")) : "",
                    check_gettext(mons_type_name(static_cast<monster_type>(it_plus), DESC_PLAIN).c_str()));
            }
            else
                buff << check_gettext("chunk of flesh");
            break;
        }

        break;

    case OBJ_SCROLLS:
        if (basename)
        {
            buff << check_gettext(M_("scroll"));
            break;
        }

        if (know_type)
            buff << make_stringf(
                /// 1. 스크롤의 type이 번역되어 들어옴. eg> 공포, 소음, 보석 저주, 무기 저주, 무기 강화 I
                check_gettext("scroll of %s"), check_gettext(scroll_type_name(item_typ)));
        else
        {
            const uint32_t sseed =
                special
                + (static_cast<uint32_t>(it_plus) << 8)
                + (static_cast<uint32_t>(OBJ_SCROLLS) << 16);
            buff << make_stringf(check_gettext("scroll labeled %s"), make_name(sseed, true).c_str());
        }
        break;

    case OBJ_JEWELLERY:
    {
        if (basename)
        {
            if (jewellery_is_amulet(*this))
                buff << check_gettext(M_("amulet"));
            else
                buff << check_gettext(M_("ring"));

            break;
        }

        const bool is_randart = is_artefact(*this);

        if (know_curse)
        {
            if (cursed())
                buff << check_gettext("cursed ");
            else if (Options.show_uncursed && !terse && desc != DESC_PLAIN
                     && (!is_randart || !know_type)
                     && (!ring_has_pluses(*this) || !know_pluses)
                     // If the item is worn, its curse status is known,
                     // no need to belabour the obvious.
                     && get_equip_slot(this) == -1)
            {
                buff << check_gettext("uncursed ");
            }
        }

        if (is_randart && !dbname)
        {
            buff << check_gettext(get_artefact_name(*this).c_str());
            break;
        }

        if (know_type)
        {
            if (know_pluses && ring_has_pluses(*this))
            {
                if (ring_has_pluses(*this) == 2)
                    buff << make_stringf("%+d,%+d ", it_plus, item_plus2);
                else
                    buff << make_stringf("%+d ", it_plus);
            }

            buff << check_gettext(jewellery_type_name(item_typ));
        }
        else
        {
            if (jewellery_is_amulet(*this))
            {
                buff << make_stringf(check_gettext("%s%s amulet"), 
                    check_gettext(amulet_secondary_string(special / NDSC_JEWEL_PRI)),
                    check_gettext(amulet_primary_string(special % NDSC_JEWEL_PRI)));
            }
            else  // i.e., a ring
            {
                buff << make_stringf(check_gettext("%s%s ring"),
                    check_gettext(ring_secondary_string(special / NDSC_JEWEL_PRI)),
                    check_gettext(ring_primary_string(special % NDSC_JEWEL_PRI)));
            }
        }
        break;
    }
    case OBJ_MISCELLANY:
        if (item_typ == MISC_RUNE_OF_ZOT)
        {
            if (!dbname)
                buff << check_gettext(rune_type_name(it_plus)) << " ";
            buff << check_gettext(M_("rune of Zot"));
        }
        else
        {
            // NUM_MISCELLANY indicates unidentified deck for item_info
            if (is_deck(*this) || item_typ == NUM_MISCELLANY)
            {
                if (basename)
                {
                    buff << check_gettext(M_("deck of cards"));
                    break;
                }
                else if (bad_deck(*this))
                {
                    buff << "BUGGY deck of cards";
                    break;
                }
                if (!dbname)
                    buff << check_gettext(deck_rarity_name(deck_rarity(*this))) << ' ';
            }
            if (item_typ == NUM_MISCELLANY)
                buff << check_gettext(misc_type_name(MISC_DECK_OF_ESCAPE, false));
            else
                buff << check_gettext(misc_type_name(item_typ, know_type));
            if (is_deck(*this) && !dbname
                && (top_card_is_known(*this) || plus2 != 0))
            {
                buff << " {";
                // A marked deck!
                if (top_card_is_known(*this))
                    buff << check_gettext(card_name(top_card(*this)));

                // How many cards have been drawn, or how many are
                // left.
                if (plus2 != 0)
                {
                    if (top_card_is_known(*this))
                        buff << ", ";

                    if (plus2 > 0)
                        buff << check_gettext("drawn: ");
                    else
                        buff << check_gettext("left: ");

                    buff << abs(plus2);
                }

                buff << "}";
            }
        }
        break;

    case OBJ_BOOKS:
        if (is_random_artefact(*this) && !dbname && !basename)
        {
            buff << check_gettext(get_artefact_name(*this).c_str());
            if (!know_type)
                buff << check_gettext(M_("book"));
            break;
        }
        if (basename)
            buff << (item_typ == BOOK_MANUAL ? check_gettext(M_("manual")) : check_gettext(M_("book")));
        else if (!know_type)
        {
            buff << make_stringf(check_gettext(item_typ == BOOK_MANUAL ? N_("%s%smanual") : N_("%s%sbook")),
                check_gettext(book_secondary_string(special / NDSC_BOOK_PRI)),
                check_gettext(book_primary_string(special % NDSC_BOOK_PRI)));
        }
        else
            buff << sub_type_string(*this, !dbname);
        break;

    case OBJ_RODS:
        if (know_curse && !terse)
        {
            if (cursed())
                buff << check_gettext("cursed ");
            else if (Options.show_uncursed && desc != DESC_PLAIN
                     && !know_pluses
                     && (!know_type || !is_artefact(*this)))
            {
                buff << check_gettext("uncursed ");
            }
        }

        if (!know_type)
        {
            if (!basename)
            {
                buff << check_gettext(staff_secondary_string((rnd / NDSC_STAVE_PRI) % NDSC_STAVE_SEC))
                     << check_gettext(staff_primary_string(rnd % NDSC_STAVE_PRI));
            }

            buff << check_gettext(M_("rod")); 
        }
        else
        {
            if (know_type && know_pluses && !basename && !qualname && !dbname)
                buff << make_stringf("%+d ", special);

            if (item_typ == ROD_LIGHTNING)
                buff << check_gettext(M_("lightning rod")); 
            else
                buff << (!translate_flag ? "rod of " : "") << (!translate_flag ? rod_type_name(item_typ) : gettext(rod_type_name(item_typ))) << (!translate_flag ? "" : "의 마법 막대") ; // (130204) 으아.
        }

        if (know_curse && cursed() && terse)
            buff << check_gettext(" (curse)"); 
        break;

    case OBJ_STAVES:
        if (know_curse && !terse)
        {
            if (cursed())
                buff << check_gettext("cursed "); 
            else if (Options.show_uncursed && desc != DESC_PLAIN
                     && (!know_type || !is_artefact(*this)))
            {
                buff << check_gettext("uncursed ");
            }
        }

        if (!know_type)
        {
            if (!basename)
            {
                buff << check_gettext(staff_secondary_string(special / NDSC_STAVE_PRI))
                     << check_gettext(staff_primary_string(special % NDSC_STAVE_PRI));
            }

            buff << check_gettext(M_("staff"));
        }
        else
            buff << (!translate_flag ? "staff of " : "") << (!translate_flag ? staff_type_name(item_typ) : gettext(staff_type_name(item_typ))) << (!translate_flag ? "" : " 지팡이"); // (130204) 추가

        if (know_curse && cursed() && terse)
            buff << check_gettext(" (curse)");
        break;

    // rearranged 15 Apr 2000 {dlb}:
    case OBJ_ORBS:
        buff.str(check_gettext("Orb of Zot"));
        break;

    case OBJ_GOLD:
        buff << check_gettext("gold piece");
        break;

    case OBJ_CORPSES:
    {
        if (dbname && item_typ == CORPSE_SKELETON)
            return "decaying skeleton";

        if (food_is_rotten(*this) && !dbname && it_plus != MONS_ROTTING_HULK)
            buff << check_gettext("rotting ");

        uint64_t name_type, name_flags = 0;

        const string _name  = get_corpse_name(*this, &name_flags);
        const bool   shaped = starts_with(_name, "shaped ");
        name_type = (name_flags & MF_NAME_MASK);

        if (!_name.empty() && name_type == MF_NAME_ADJECTIVE)
            buff << _name << (translate_flag ? "" : " "); // (deceit, 110912) 수정부분 

        if ((name_flags & MF_NAME_SPECIES) && name_type == MF_NAME_REPLACE)
            buff << _name << (translate_flag ? "" : " "); // (deceit, 110912) 수정부분 
        else if (!dbname && !starts_with(_name, "the "))
        {
            const monster_type mc = static_cast<monster_type>(it_plus);
            if (!(mons_is_unique(mc) && mons_species(mc) == mc))
                buff << check_gettext(mons_type_name(mc, DESC_PLAIN).c_str()) << ' ';

            if (!_name.empty() && shaped)
                buff << _name << ' ';
        }

        if (item_typ == CORPSE_BODY)
            buff << check_gettext(M_("corpse"));
        else if (item_typ == CORPSE_SKELETON)
            buff << check_gettext(M_("skeleton"));
        else
            buff << "corpse bug";

        if (!_name.empty() && !shaped && name_type != MF_NAME_ADJECTIVE
            && !(name_flags & MF_NAME_SPECIES) && name_type != MF_NAME_SUFFIX
            && !dbname)
        {
            if(translate_flag) { buff << " (" << gettext(_name.c_str()) << ")"; } else { buff << " of " << _name;}
        }
        break;
    }

    default:
        buff << "!";
    }

    // One plural to rule them all.
    if (need_plural && quantity > 1 && !basename && !qualname)
        buff.str(pluralise(((!translate_flag) ? PLU_DEFAULT : PLU_MISC), buff.str()));

    // Disambiguation.
    if (!terse && !basename && !dbname && know_type
        && !is_artefact(*this))
    {
        switch (base_type)
        {
        case OBJ_RODS:
            switch (item_typ)
            {
            case ROD_DESTRUCTION_I:
                buff << check_gettext(M_(" [fire]"));
                break;
            case ROD_DESTRUCTION_II:
                buff << check_gettext(M_(" [ice]"));
                break;
            case ROD_DESTRUCTION_III:
                buff << check_gettext(M_(" [lightning,fireball,iron]"));
                break;
            }
            break;

        default:
            break;
        }
    }

    // Rod charges.
    if (base_type == OBJ_RODS && know_type && know_pluses
        && !basename && !qualname && !dbname)
    {
        buff << " (" << (plus / ROD_CHARGE_MULT)
             << "/"  << (plus2 / ROD_CHARGE_MULT)
             << ")";
    }

    // debugging output -- oops, I probably block it above ... dang! {dlb}
    if (buff.str().length() < 3)
    {
        buff << "bad item (cl:" << static_cast<int>(base_type)
             << ",ty:" << item_typ << ",pl:" << it_plus
             << ",pl2:" << item_plus2 << ",sp:" << special
             << ",qu:" << quantity << ")";
    }

    return buff.str();
}

bool item_type_has_ids(object_class_type base_type)
{
    COMPILE_CHECK(NUM_WEAPONS    < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_MISSILES   < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_ARMOURS    < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_WANDS      < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_FOODS      < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_SCROLLS    < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_JEWELLERY  < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_POTIONS    < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_BOOKS      < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_STAVES     < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_MISCELLANY < MAX_SUBTYPES);
    COMPILE_CHECK(NUM_RODS       < MAX_SUBTYPES);

    return base_type == OBJ_WANDS || base_type == OBJ_SCROLLS
        || base_type == OBJ_JEWELLERY || base_type == OBJ_POTIONS
        || base_type == OBJ_STAVES;
}

bool item_type_known(const item_def& item)
{
    if (item_ident(item, ISFLAG_KNOW_TYPE))
        return true;

    // Artefacts have different descriptions from other items,
    // so we can't use general item knowledge for them.
    if (is_artefact(item))
        return false;

    if (item.base_type == OBJ_MISSILES)
        return true;

    if (item.base_type == OBJ_MISCELLANY
        && (item.sub_type < MISC_FIRST_DECK || item.sub_type > MISC_LAST_DECK))
    {
        return true;
    }

    if (item.base_type == OBJ_BOOKS && item.sub_type == BOOK_DESTRUCTION)
        return true;

    if (!item_type_has_ids(item.base_type))
        return false;
    return (you.type_ids[item.base_type][item.sub_type] == ID_KNOWN_TYPE);
}

bool item_type_unknown(const item_def& item)
{
    if (item_type_known(item))
        return false;

    if (is_artefact(item))
        return true;

    return item_type_has_ids(item.base_type);
}

bool item_type_known(const object_class_type base_type, const int sub_type)
{
    if (!item_type_has_ids(base_type))
        return false;
    return (you.type_ids[base_type][sub_type] == ID_KNOWN_TYPE);
}

bool item_type_tried(const item_def &item)
{
    if (!is_artefact(item) && item_type_known(item))
        return false;

    if (fully_identified(item))
        return false;

    if (item.flags & ISFLAG_TRIED)
        return true;

    // artefacts are distinct from their base types
    if (is_artefact(item))
        return false;

    if (!item_type_has_ids(item.base_type))
        return false;
    return (you.type_ids[item.base_type][item.sub_type] != ID_UNKNOWN_TYPE);
}

void set_ident_type(item_def &item, item_type_id_state_type setting,
                    bool force)
{
    if (is_artefact(item) || crawl_state.game_is_arena())
        return;

    item_type_id_state_type old_setting = get_ident_type(item);
    set_ident_type(item.base_type, item.sub_type, setting, force);

    if (in_inventory(item))
    {
        shopping_list.cull_identical_items(item);
        if (setting == ID_KNOWN_TYPE)
            item_skills(item, you.start_train);
    }

    if (setting == ID_KNOWN_TYPE && old_setting != ID_KNOWN_TYPE
        && notes_are_active() && is_interesting_item(item)
        && !(item.flags & (ISFLAG_NOTED_ID | ISFLAG_NOTED_GET)))
    {
        // Make a note of it.
        take_note(Note(NOTE_ID_ITEM, 0, 0, item.name(true, DESC_A).c_str(),
                       origin_desc(item).c_str()));

        // Sometimes (e.g. shops) you can ID an item before you get it;
        // don't note twice in those cases.
        item.flags |= (ISFLAG_NOTED_ID | ISFLAG_NOTED_GET);
    }
}

void set_ident_type(object_class_type basetype, int subtype,
                     item_type_id_state_type setting, bool force)
{
    preserve_quiver_slots p;
    // Don't allow overwriting of known type with tried unless forced.
    if (!force
        && (setting == ID_MON_TRIED_TYPE || setting == ID_TRIED_TYPE)
        && setting <= get_ident_type(basetype, subtype))
    {
        return;
    }

    if (!item_type_has_ids(basetype))
        return;

    if (you.type_ids[basetype][subtype] != setting)
    {
        you.type_ids[basetype][subtype] = setting;
        request_autoinscribe();
    }
}

item_type_id_state_type get_ident_type(const item_def &item)
{
    if (is_artefact(item))
        return ID_UNKNOWN_TYPE;

    return get_ident_type(item.base_type, item.sub_type);
}

item_type_id_state_type get_ident_type(object_class_type basetype, int subtype)
{
    if (!item_type_has_ids(basetype))
        return ID_UNKNOWN_TYPE;
    ASSERT(subtype < MAX_SUBTYPES);
    return you.type_ids[basetype][subtype];
}

class KnownMenu : public InvMenu
{
public:
    // This loads items in the order they are put into the list (sequentially)
    menu_letter load_items_seq(const vector<const item_def*> &mitems,
                               MenuEntry *(*procfn)(MenuEntry *me) = NULL,
                               menu_letter ckey = 'a')
    {
        for (int i = 0, count = mitems.size(); i < count; ++i)
        {
            InvEntry *ie = new InvEntry(*mitems[i]);
            if (tag == "pickup")
                ie->tag = "pickup";
            // If there's no hotkey, provide one.
            if (ie->hotkeys[0] == ' ')
                ie->hotkeys[0] = ckey++;
            do_preselect(ie);

            add_entry(procfn? (*procfn)(ie) : ie);
        }

        return ckey;
    }
};

class KnownEntry : public InvEntry
{
public:
    KnownEntry(InvEntry* inv) : InvEntry(*inv->item)
    {
        hotkeys[0] = inv->hotkeys[0];
        selected_qty = inv->selected_qty;
    }

    virtual string get_text(bool need_cursor) const
    {
        need_cursor = need_cursor && show_cursor;
        int flags = item->base_type == OBJ_WANDS ? 0 : ISFLAG_KNOW_PLUSES;

        string name;


        if (item->base_type == OBJ_FOOD)
        {
            if (item->sub_type == FOOD_CHUNK)
                name = "고기";
            else
                name = "보존 식량";
        }
        else if (item->base_type == OBJ_MISCELLANY)
        {
            if (item->sub_type == MISC_RUNE_OF_ZOT)
                name = "룬";
            else
                name = "기타";
        }
        else if (item->base_type == OBJ_BOOKS || item->base_type == OBJ_RODS
                 || item->base_type == OBJ_GOLD)
        {
            name = lowercase_string(_(item_class_name(item->base_type).c_str()));
            // name = pluralise(PLU_DEFAULT, name);
        }
        else if (item->sub_type == get_max_subtype(item->base_type))
            name = "알려지지 않은 " + lowercase_string(_(item_class_name(item->base_type).c_str()));
        else
        {
            name = item->name(true, DESC_PLAIN,false,true,false,false,flags);
            // name = pluralise(PLU_DEFAULT, name); (deceit, 130316) pluralise 해제
        }

        char symbol;
        if (selected_qty == 0)
            symbol = item_needs_autopickup(*item) ? '+' : '-';
        else if (selected_qty == 1)
            symbol = '+';
        else
            symbol = '-';

        return make_stringf("%c%c%c%c%s", hotkeys[0], need_cursor ? '[' : ' ',
                                          symbol, need_cursor ? ']' : ' ',
                                          name.c_str());
    }

    virtual int highlight_colour() const
    {
        if (selected_qty >= 1)
            return WHITE;
        else
            return MENU_ITEM_STOCK_COLOUR;

    }

    virtual bool selected() const
    {
        return (selected_qty != 0 && quantity);
    }

    virtual void select(int qty)
    {
        if (qty == -2)
            selected_qty = 0;
        else if (selected_qty == 0)
            selected_qty = item_needs_autopickup(*item) ? 2 : 1;
        else
            ++selected_qty;

        if (selected_qty > 2)
            selected_qty = 1; //Set to 0 to allow triple toggle

        // Set the force_autopickup values
        const int forceval = (selected_qty == 2 ? -1 : selected_qty);
        you.force_autopickup[item->base_type][item->sub_type] = forceval;
    }
};

class UnknownEntry : public InvEntry
{
public:
    UnknownEntry(InvEntry* inv) : InvEntry(*inv->item)
    {
    }

    virtual string get_text(const bool = false) const
    {
        int flags = item->base_type == OBJ_WANDS ? 0 : ISFLAG_KNOW_PLUSES;

        return string(" ") + item->name(false, DESC_PLAIN, false, true,
                                             false, false, flags);
    }
};

static MenuEntry *known_item_mangle(MenuEntry *me)
{
    InvEntry *ie = dynamic_cast<InvEntry*>(me);
    KnownEntry *newme = new KnownEntry(ie);
    delete me;

    return newme;
}

static MenuEntry *unknown_item_mangle(MenuEntry *me)
{
    InvEntry *ie = dynamic_cast<InvEntry*>(me);
    UnknownEntry *newme = new UnknownEntry(ie);
    delete me;

    return newme;
}

static bool _identified_item_names(const item_def *it1,
                                   const item_def *it2)
{
    int flags = it1->base_type == OBJ_WANDS ? 0 : ISFLAG_KNOW_PLUSES;
    return it1->name(false, DESC_PLAIN, false, true, false, false, flags)
         < it2->name(false, DESC_PLAIN, false, true, false, false, flags);
}

void check_item_knowledge(bool unknown_items)
{
    vector<const item_def*> items;
    vector<const item_def*> items_missile; //List of missiles should go after normal items
    vector<const item_def*> items_other;    //List of other items should go after everything
    vector<SelItem> selected_items;

    bool all_items_known = true;
    for (int ii = 0; ii < NUM_OBJECT_CLASSES; ii++)
    {
        object_class_type i = (object_class_type)ii;
        if (!item_type_has_ids(i))
            continue;
        for (int j = 0; j < get_max_subtype(i); j++)
        {
            if (i == OBJ_JEWELLERY && j >= NUM_RINGS && j < AMU_FIRST_AMULET)
                continue;

            // Potions of fizzing liquid are not something that
            // need to be identified, because they never randomly
            // generate! [due]
            if (i == OBJ_POTIONS && j == POT_FIZZING)
                continue;

#if TAG_MAJOR_VERSION == 34
            // Water is never interesting either. [1KB]
            if (i == OBJ_POTIONS
                && (j == POT_WATER
                 || j == POT_GAIN_STRENGTH
                 || j == POT_GAIN_DEXTERITY
                 || j == POT_GAIN_INTELLIGENCE))
            {
                continue;
            }

            if (i == OBJ_JEWELLERY && j == AMU_CONTROLLED_FLIGHT)
                continue;

            if (i == OBJ_STAVES && j == STAFF_ENCHANTMENT)
                continue;

            if (i == OBJ_STAVES && j == STAFF_CHANNELING)
                continue;
#endif

            if (unknown_items ? you.type_ids[i][j] != ID_KNOWN_TYPE
                              : you.type_ids[i][j] == ID_KNOWN_TYPE)
            {
                item_def* ptmp = new item_def;
                if (ptmp != 0)
                {
                    ptmp->base_type = i;
                    ptmp->sub_type  = j;
                    ptmp->colour    = 1;
                    ptmp->quantity  = 1;
                    if (!unknown_items)
                        ptmp->flags |= ISFLAG_KNOW_TYPE;
                    if (i == OBJ_WANDS)
                        ptmp->plus = wand_max_charges(j);
                    items.push_back(ptmp);

                    if (you.force_autopickup[i][j] == 1)
                        selected_items.push_back(SelItem(0,1,ptmp));
                    if (you.force_autopickup[i][j] == -1)
                        selected_items.push_back(SelItem(0,2,ptmp));
                }
            }
            else
                all_items_known = false;
        }
    }

    if (unknown_items)
        all_items_known = false;

    else
    {
        // items yet to be known
        for (int ii = 0; ii < NUM_OBJECT_CLASSES; ii++)
        {
            object_class_type i = (object_class_type)ii;
            if (!item_type_has_ids(i))
                continue;
            item_def* ptmp = new item_def;
            if (ptmp != 0)
            {
                ptmp->base_type = i;
                ptmp->sub_type  = get_max_subtype(i);
                ptmp->colour    = 1;
                ptmp->quantity  = 1;
                items.push_back(ptmp);

                if (you.force_autopickup[i][ptmp->sub_type] == 1)
                    selected_items.push_back(SelItem(0,1,ptmp));
                if (you.force_autopickup[i][ptmp->sub_type ] == -1)
                    selected_items.push_back(SelItem(0,2,ptmp));
            }
        }
        // Missiles
        for (int i = 0; i < NUM_MISSILES; i++)
        {
#if TAG_MAJOR_VERSION == 34
            if (i == MI_PIE)
                continue;
#endif

            item_def* ptmp = new item_def;
            if (ptmp != 0)
            {
                ptmp->base_type = OBJ_MISSILES;
                ptmp->sub_type  = i;
                ptmp->colour    = 1;
                ptmp->quantity  = 1;
                items_missile.push_back(ptmp);

                if (you.force_autopickup[OBJ_MISSILES][i] == 1)
                    selected_items.push_back(SelItem(0,1,ptmp));
                if (you.force_autopickup[OBJ_MISSILES][i] == -1)
                    selected_items.push_back(SelItem(0,2,ptmp));
            }
        }
        // Misc.
        object_class_type misc_list[] = {OBJ_FOOD, OBJ_FOOD, OBJ_BOOKS,
                                         OBJ_RODS, OBJ_GOLD, OBJ_MISCELLANY,
                                         OBJ_MISCELLANY};
        int misc_ST_list[] = {FOOD_CHUNK, NUM_FOODS, NUM_BOOKS, NUM_RODS,
                              1, MISC_RUNE_OF_ZOT, NUM_MISCELLANY};
        for (unsigned i = 0; i < sizeof(misc_list)/sizeof(object_class_type); i++)
        {
            item_def* ptmp = new item_def;
            if (ptmp != 0)
            {
                ptmp->base_type = misc_list[i];
                ptmp->sub_type  = misc_ST_list[i];
                ptmp->colour    = 2;
                ptmp->quantity  = 18;  //show a good amount of gold

                // Make chunks fresh, non-poisonous, etc.
                if (ptmp->base_type == OBJ_FOOD
                    && ptmp->sub_type == FOOD_CHUNK)
                {
                    ptmp->special = 100;
                    ptmp->mon_type = MONS_RAT;
                }

                items_other.push_back(ptmp);

                if (you.force_autopickup[misc_list[i]][ptmp->sub_type] == 1)
                    selected_items.push_back(SelItem(0,1,ptmp));
                if (you.force_autopickup[misc_list[i]][ptmp->sub_type] == -1)
                    selected_items.push_back(SelItem(0,2,ptmp));
            }
        }
    }

    sort(items.begin(), items.end(), _identified_item_names);
    sort(items_missile.begin(), items_missile.end(), _identified_item_names);

    KnownMenu menu;
    string stitle;

    if (unknown_items)
        stitle = _("Items not yet recognised: (toggle with -)");
    else if (!all_items_known)
        stitle = _("Recognised items. (- for unrecognised, select to toggle autopickup)");
    else
        stitle = _("You recognise all items. (Select to toggle autopickup)");


    string prompt = _("(_ for help)");
    //TODO: when the menu is opened, the text is not justified properly.
    stitle = stitle + string(max(0, get_number_of_cols() - strwidth(stitle)
                                                         - strwidth(prompt)),
                             ' ') + prompt;

    menu.set_preselect(&selected_items);
    menu.set_flags( MF_QUIET_SELECT | MF_ALLOW_FORMATTING
                    | ((unknown_items) ? MF_NOSELECT
                                       : MF_MULTISELECT | MF_ALLOW_FILTER));
    menu.set_type(MT_KNOW);
    menu_letter ml;
    ml = menu.load_items(items, unknown_items ? unknown_item_mangle
                                              : known_item_mangle);

    ml = menu.load_items(items_missile, known_item_mangle, ml);
    menu.add_entry(new MenuEntry("Other Items", MEL_SUBTITLE));
    menu.load_items_seq(items_other, known_item_mangle, ml);

    menu.set_title(stitle);
    menu.show(true);

    char last_char = menu.getkey();

    vector<const item_def*>::iterator iter;
    for (iter = items.begin(); iter != items.end(); ++iter)
         delete *iter;
    for (iter = items_missile.begin(); iter != items_missile.end(); ++iter)
         delete *iter;
    for (iter = items_other.begin(); iter != items_other.end(); ++iter)
         delete *iter;

    if (!all_items_known && (last_char == '\\' || last_char == '-'))
        check_item_knowledge(!unknown_items);
}

void display_runes()
{
    vector<const item_def*> items;
    for (int i = 0; i < NUM_RUNE_TYPES; i++)
    {
        if (!you.runes[i])
            continue;

        item_def* ptmp = new item_def;
        if (ptmp != 0)
        {
            ptmp->base_type = OBJ_MISCELLANY;
            ptmp->sub_type  = MISC_RUNE_OF_ZOT;
            ptmp->quantity  = 1;
            ptmp->plus      = i;
            item_colour(*ptmp);
            items.push_back(ptmp);
        }
    }

    if (items.empty())
    {
        mpr(_("You haven't found any runes yet."));
        return;
    }

    InvMenu menu;

    menu.set_title(make_stringf(_("Runes of Zot: %d/%d"),
                                (int)items.size(),
                                you.obtainable_runes));
    menu.set_flags(MF_NOSELECT);
    menu.set_type(MT_RUNES);
    menu.load_items(items, unknown_item_mangle);
    menu.show();
    menu.getkey();
    redraw_screen();

    for (vector<const item_def*>::iterator iter = items.begin();
         iter != items.end(); ++iter)
    {
         delete *iter;
    }
}

// Used for: Pandemonium demonlords, shopkeepers, scrolls, random artefacts
string make_name(uint32_t seed, bool all_cap, int maxlen, char start)
{
    char name[ITEMNAME_SIZE];
    int  numb[17]; // contains the random seeds used for the name

    int i = 0;
    bool want_vowel = false; // Keep track of whether we want a vowel next.
    bool has_space  = false; // Keep track of whether the name contains a space.

    for (i = 0; i < ITEMNAME_SIZE; ++i)
        name[i] = '\0';

    const int var1 = (seed & 0xFF);
    const int var2 = ((seed >>  8) & 0xFF);
    const int var3 = ((seed >> 16) & 0xFF);
    const int var4 = ((seed >> 24) & 0xFF);

    numb[0]  = 373 * var1 + 409 * var2 + 281 * var3;
    numb[1]  = 163 * var4 + 277 * var2 + 317 * var3;
    numb[2]  = 257 * var1 + 179 * var4 +  83 * var3;
    numb[3]  =  61 * var1 + 229 * var2 + 241 * var4;
    numb[4]  =  79 * var1 + 263 * var2 + 149 * var3;
    numb[5]  = 233 * var4 + 383 * var2 + 311 * var3;
    numb[6]  = 199 * var1 + 211 * var4 + 103 * var3;
    numb[7]  = 139 * var1 + 109 * var2 + 349 * var4;
    numb[8]  =  43 * var1 + 389 * var2 + 359 * var3;
    numb[9]  = 367 * var4 + 101 * var2 + 251 * var3;
    numb[10] = 293 * var1 +  59 * var4 + 151 * var3;
    numb[11] = 331 * var1 + 107 * var2 + 307 * var4;
    numb[12] =  73 * var1 + 157 * var2 + 347 * var3;
    numb[13] = 379 * var4 + 353 * var2 + 227 * var3;
    numb[14] = 181 * var1 + 173 * var4 + 193 * var3;
    numb[15] = 131 * var1 + 167 * var2 +  53 * var4;
    numb[16] = 313 * var1 + 127 * var2 + 401 * var3 + 337 * var4;

    int len = 3 + numb[0] % 5 + ((numb[1] % 5 == 0) ? numb[2] % 6 : 1);

    if (all_cap)   // scrolls have longer names
        len += 6;

    if (maxlen != -1 && len > maxlen)
        len = maxlen;

    ASSERT(len > 0);
    ASSERT(len <= ITEMNAME_SIZE);

    int j = numb[3] % 17;
    const int k = numb[4] % 17;

    int count = 0;
    for (i = 0; i < len; ++i)
    {
        j = (j + 1) % 17;
        if (j == 0)
        {
            count++;
            if (count > 9)
                break;
        }

        if (i == 0 && start != 0)
        {
            // Start the name with a predefined letter.
            name[i] = start;
            want_vowel = _is_random_name_vowel(start);
        }
        else if (!has_space && i > 5 && i < len - 4
                 && (numb[(k + 10 * j) % 17] % 5) != 3) // 4/5 chance of a space
        {
            // Hand out a space.
            want_vowel = true;
            name[i] = ' ';
        }
        else if (i > 0
                 && (want_vowel
                     || (i > 1
                         && _is_random_name_vowel(name[i - 1])
                         && !_is_random_name_vowel(name[i - 2])
                         && (numb[(k + 4 * j) % 17] % 5) <= 1))) // 2/5 chance
        {
            // Place a vowel.
            want_vowel = true;
            name[i] = _random_vowel(numb[(k + 7 * j) % 17]);

            if (_is_random_name_space(name[i]))
            {
                if (i == 0) // Shouldn't happen.
                {
                    want_vowel = false;
                    name[i]    = _random_cons(numb[(k + 14 * j) % 17]);
                }
                else if (len < 7
                         || i <= 2 || i >= len - 3
                         || _is_random_name_space(name[i - 1])
                         || (i > 1 && _is_random_name_space(name[i - 2]))
                         || i > 2
                            && !_is_random_name_vowel(name[i - 1])
                            && !_is_random_name_vowel(name[i - 2]))
                {
                    // Replace the space with something else if ...
                    // * the name is really short
                    // * we're close to the begin/end of the name
                    // * we just got a space, or
                    // * the last two letters were consonants
                    i--;
                    continue;
                }
            }
            else if (i > 1
                     && name[i] == name[i - 1]
                     && (name[i] == 'y' || name[i] == 'i'
                         || (numb[(k + 12 * j) % 17] % 5) <= 1))
            {
                // Replace the vowel with something else if the previous
                // letter was the same, and it's a 'y', 'i' or with 2/5 chance.
                i--;
                continue;
            }
        }
        else // We want a consonant.
        {
            // Use one of number of predefined letter combinations.
            if ((len > 3 || i != 0)
                && (numb[(k + 13 * j) % 17] % 7) <= 1 // 2/7 chance
                && (i < len - 2
                    || i > 0 && !_is_random_name_space(name[i - 1])))
            {
                // Are we at start or end of the (sub) name?
                const bool beg = (i < 1 || _is_random_name_space(name[i - 1]));
                const bool end = (i >= len - 2);

                const int first = (beg ?  0 : (end ? 14 :  0));
                const int last  = (beg ? 27 : (end ? 56 : 67));

                const int num = last - first;

                i++;

                // Pick a random combination of consonants from the set below.
                //   begin  -> [0,27]
                //   middle -> [0,67]
                //   end    -> [14,56]

                switch (numb[(k + 11 * j) % 17] % num + first)
                {
                // start, middle
                case  0: strcat(name, "kl"); break;
                case  1: strcat(name, "gr"); break;
                case  2: strcat(name, "cl"); break;
                case  3: strcat(name, "cr"); break;
                case  4: strcat(name, "fr"); break;
                case  5: strcat(name, "pr"); break;
                case  6: strcat(name, "tr"); break;
                case  7: strcat(name, "tw"); break;
                case  8: strcat(name, "br"); break;
                case  9: strcat(name, "pl"); break;
                case 10: strcat(name, "bl"); break;
                case 11: strcat(name, "str"); i++; len++; break;
                case 12: strcat(name, "shr"); i++; len++; break;
                case 13: strcat(name, "thr"); i++; len++; break;
                // start, middle, end
                case 14: strcat(name, "sm"); break;
                case 15: strcat(name, "sh"); break;
                case 16: strcat(name, "ch"); break;
                case 17: strcat(name, "th"); break;
                case 18: strcat(name, "ph"); break;
                case 19: strcat(name, "pn"); break;
                case 20: strcat(name, "kh"); break;
                case 21: strcat(name, "gh"); break;
                case 22: strcat(name, "mn"); break;
                case 23: strcat(name, "ps"); break;
                case 24: strcat(name, "st"); break;
                case 25: strcat(name, "sk"); break;
                case 26: strcat(name, "sch"); i++; len++; break;
                // middle, end
                case 27: strcat(name, "ts"); break;
                case 28: strcat(name, "cs"); break;
                case 29: strcat(name, "xt"); break;
                case 30: strcat(name, "nt"); break;
                case 31: strcat(name, "ll"); break;
                case 32: strcat(name, "rr"); break;
                case 33: strcat(name, "ss"); break;
                case 34: strcat(name, "wk"); break;
                case 35: strcat(name, "wn"); break;
                case 36: strcat(name, "ng"); break;
                case 37: strcat(name, "cw"); break;
                case 38: strcat(name, "mp"); break;
                case 39: strcat(name, "ck"); break;
                case 40: strcat(name, "nk"); break;
                case 41: strcat(name, "dd"); break;
                case 42: strcat(name, "tt"); break;
                case 43: strcat(name, "bb"); break;
                case 44: strcat(name, "pp"); break;
                case 45: strcat(name, "nn"); break;
                case 46: strcat(name, "mm"); break;
                case 47: strcat(name, "kk"); break;
                case 48: strcat(name, "gg"); break;
                case 49: strcat(name, "ff"); break;
                case 50: strcat(name, "pt"); break;
                case 51: strcat(name, "tz"); break;
                case 52: strcat(name, "dgh"); i++; len++; break;
                case 53: strcat(name, "rgh"); i++; len++; break;
                case 54: strcat(name, "rph"); i++; len++; break;
                case 55: strcat(name, "rch"); i++; len++; break;
                // middle only
                case 56: strcat(name, "cz"); break;
                case 57: strcat(name, "xk"); break;
                case 58: strcat(name, "zx"); break;
                case 59: strcat(name, "xz"); break;
                case 60: strcat(name, "cv"); break;
                case 61: strcat(name, "vv"); break;
                case 62: strcat(name, "nl"); break;
                case 63: strcat(name, "rh"); break;
                case 64: strcat(name, "dw"); break;
                case 65: strcat(name, "nw"); break;
                case 66: strcat(name, "khl"); i++; len++; break;
                default:
                    i--;
                    break;
                }
            }
            else // Place a single letter instead.
            {
                if (i == 0)
                {
                    // Start with any letter.
                    name[i] = 'a' + (numb[(k + 8 * j) % 17] % 26);
                    want_vowel = _is_random_name_vowel(name[i]);
                }
                else
                {
                    // Pick a random consonant.
                    name[i] = _random_cons(numb[(k + 3 * j) % 17]);
                }
            }
        }

        // No letter chosen?
        if (name[i] == '\0')
        {
            i--;
            continue;
        }

        // Picked wrong type?
        if (want_vowel && !_is_random_name_vowel(name[i])
            || !want_vowel && _is_random_name_vowel(name[i]))
        {
            i--;
            continue;
        }

        if (_is_random_name_space(name[i]))
            has_space = true;

        // If we just got a vowel, we want a consonant next, and vice versa.
        want_vowel = !_is_random_name_vowel(name[i]);
    }

    // Catch break and try to give a final letter.
    if (i > 0
        && !_is_random_name_space(name[i - 1])
        && name[i - 1] != 'y'
        && _is_random_name_vowel(name[i - 1])
        && (count > 9 || (i < 8 && numb[16] % 3)))
    {
        // 2/3 chance of ending in a consonant
        name[i] = _random_cons(numb[j]);
    }

    len = strlen(name);

    if (len)
    {
        for (i = len - 1; i > 0; i--)
        {
            if (!isspace(name[i]))
                break;
            else
            {
                name[i] = '\0';
                len--;
            }
        }
    }

    // Fallback if the name was too short.
    if (len < 4)
    {
        strcpy(name, "plog");
        len = 4;
    }

    for (i = 0; i < len; i++)
        if (all_cap || i == 0 || name[i - 1] == ' ')
            name[i] = toupper(name[i]);

    return name;
}

static bool _is_random_name_space(char let)
{
    return (let == ' ');
}

// Returns true for vowels, 'y' or space.
static bool _is_random_name_vowel(char let)
{
    return (let == 'a' || let == 'e' || let == 'i' || let == 'o' || let == 'u'
            || let == 'y' || let == ' ');
}

// Returns a random vowel (a, e, i, o, u with equal probability) or space
// or 'y' with lower chances.
static char _random_vowel(int seed)
{
    static const char vowels[] = "aeiouaeiouaeiouy  ";
    return vowels[ seed % (sizeof(vowels) - 1) ];
}

// Returns a random consonant with not quite equal probability.
// Does not include 'y'.
static char _random_cons(int seed)
{
    static const char consonants[] = "bcdfghjklmnpqrstvwxzcdfghlmnrstlmnrst";
    return consonants[ seed % (sizeof(consonants) - 1) ];
}

bool is_interesting_item(const item_def& item)
{
    if (fully_identified(item) && is_artefact(item))
        return true;

    const string iname = item_prefix(item, false) + " " + item.name(false, DESC_PLAIN);
    for (unsigned i = 0; i < Options.note_items.size(); ++i)
        if (Options.note_items[i].matches(iname))
            return true;

    return false;
}

// Returns true if an item is a potential life saver in an emergency
// situation.
bool is_emergency_item(const item_def &item)
{
    if (!item_type_known(item))
        return false;

    switch (item.base_type)
    {
    case OBJ_WANDS:
        switch (item.sub_type)
        {
        case WAND_HASTING:
            if (you.religion == GOD_CHEIBRIADOS)
                return false;
        case WAND_HEAL_WOUNDS:
        case WAND_TELEPORTATION:
            return true;
        default:
            return false;
        }
    case OBJ_SCROLLS:
        switch (item.sub_type)
        {
        case SCR_TELEPORTATION:
        case SCR_BLINKING:
        case SCR_FEAR:
        case SCR_FOG:
            return true;
        default:
            return false;
        }
    case OBJ_POTIONS:
        if (you.species == SP_MUMMY)
            return false;

        switch (item.sub_type)
        {
        case POT_SPEED:
            if (you.religion == GOD_CHEIBRIADOS)
                return false;
        case POT_CURING:
        case POT_HEAL_WOUNDS:
        case POT_RESISTANCE:
        case POT_MAGIC:
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

// Returns true if an item can be considered particularly good.
bool is_good_item(const item_def &item)
{
    if (!item_type_known(item))
        return false;

    if (is_emergency_item(item))
        return true;

    switch (item.base_type)
    {
    case OBJ_SCROLLS:
        return (item.sub_type == SCR_ACQUIREMENT);
    case OBJ_POTIONS:
        switch (item.sub_type)
        {
        case POT_CURE_MUTATION:
#if TAG_MAJOR_VERSION == 34
        case POT_GAIN_STRENGTH:
        case POT_GAIN_INTELLIGENCE:
        case POT_GAIN_DEXTERITY:
#endif
        case POT_EXPERIENCE:
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

// Returns true if using an item only has harmful effects.
bool is_bad_item(const item_def &item, bool temp)
{
    if (!item_type_known(item))
        return false;

    switch (item.base_type)
    {
    case OBJ_SCROLLS:
        switch (item.sub_type)
        {
        case SCR_CURSE_ARMOUR:
        case SCR_CURSE_WEAPON:
            if (you.species == SP_FELID)
                return false;
        case SCR_CURSE_JEWELLERY:
            return (you.religion != GOD_ASHENZARI);
        default:
            return false;
        }
    case OBJ_POTIONS:
        // Can't be bad if you can't use them.
        if (you.species == SP_MUMMY)
            return false;

        switch (item.sub_type)
        {
        case POT_CONFUSION:
        case POT_SLOWING:
        case POT_DEGENERATION:
        case POT_DECAY:
        case POT_PARALYSIS:
            return true;
        case POT_POISON:
        case POT_STRONG_POISON:
            // Poison is not that bad if you're poison resistant.
            return (player_res_poison(false) <= 0
                    || !temp && you.species == SP_VAMPIRE);
        case POT_MUTATION:
            return (you.is_undead
                    && (temp || you.species != SP_VAMPIRE
                        || you.hunger_state < HS_SATIATED));
        default:
            return false;
        }
    case OBJ_JEWELLERY:
        // Potentially useful.  TODO: check the properties.
        if (is_artefact(item))
            return false;

        switch (item.sub_type)
        {
        case AMU_INACCURACY:
            return true;
        case RING_HUNGER:
            // Even Vampires can use this ring.
            return (!you.is_undead || you.is_undead == US_HUNGRY_DEAD);
        case RING_EVASION:
        case RING_PROTECTION:
        case RING_STRENGTH:
        case RING_DEXTERITY:
        case RING_INTELLIGENCE:
            return (item_ident(item, ISFLAG_KNOW_PLUSES) && item.plus <= 0);
        case RING_SLAYING:
            return (item_ident(item, ISFLAG_KNOW_PLUSES) && item.plus <= 0
                    && item.plus2 <= 0);
        default:
            return false;
        }

    default:
        return false;
    }
}

// Returns true if using an item is risky but may occasionally be
// worthwhile.
bool is_dangerous_item(const item_def &item, bool temp)
{
    if (!item_type_known(item))
        return false;

    switch (item.base_type)
    {
    case OBJ_SCROLLS:
        switch (item.sub_type)
        {
        case SCR_IMMOLATION:
        case SCR_NOISE:
            return true;
        case SCR_TORMENT:
            return (!player_mutation_level(MUT_TORMENT_RESISTANCE)
                    || !temp && you.species == SP_VAMPIRE);
        case SCR_HOLY_WORD:
            return you.undead_or_demonic();
        default:
            return false;
        }

    case OBJ_POTIONS:
        switch (item.sub_type)
        {
        case POT_MUTATION:
            // Only living characters can mutate.
            return (!you.is_undead
                    || temp && you.species == SP_VAMPIRE
                       && you.hunger_state >= HS_SATIATED);
        default:
            return false;
        }

    case OBJ_BOOKS:
        // The Tome of Destruction is certainly risky.
        return (item.sub_type == BOOK_DESTRUCTION);

    default:
        return false;
    }
}

static bool _invisibility_is_useless(const bool temp)
{
    return (temp ? you.backlit(true, true, false)
                 : you.haloed() && you.religion == GOD_SHINING_ONE);

}

bool is_useless_item(const item_def &item, bool temp)
{
    // During game startup, no item is useless.  If someone re-glyphs an item
    // based on its uselessness, the glyph-to-item cache will use the useless
    // value even if your god or species can make use of it.
    if (you.species == SP_UNKNOWN)
        return false;

    switch (item.base_type)
    {
    case OBJ_WEAPONS:
        if (you.species == SP_FELID)
            return true;

        if (!you.could_wield(item, true, !temp)
            && !is_throwable(&you, item))
        {
            // Weapon is too large (or small) to be wielded and cannot
            // be thrown either.
            return true;
        }

        if (!item_type_known(item))
            return false;

        if (you.undead_or_demonic() && is_holy_item(item))
        {
            if (!temp && you.form == TRAN_LICH
                && you.species != SP_DEMONSPAWN)
            {
                return false;
            }
            return true;
        }

        return false;

    case OBJ_MISSILES:
        if ((you.has_spell(SPELL_STICKS_TO_SNAKES)
             || !you.num_turns
                && you.char_class == JOB_TRANSMUTER)
            && item_is_snakable(item)
            || you.has_spell(SPELL_SANDBLAST)
               && (item.sub_type == MI_STONE
                || item.sub_type == MI_LARGE_ROCK))
        {
            return false;
        }

        // Save for the above spells, all missiles are useless for felids.
        if (you.species == SP_FELID)
            return true;

        // These are the same checks as in is_throwable(), except that
        // we don't take launchers into account.
        switch (item.sub_type)
        {
        case MI_LARGE_ROCK:
            return (you.body_size(PSIZE_BODY, !temp) < SIZE_LARGE
                    || !you.can_throw_large_rocks());
        case MI_JAVELIN:
        case MI_THROWING_NET:
            return (you.body_size(PSIZE_BODY, !temp) < SIZE_MEDIUM);
        }

        return false;

    case OBJ_ARMOUR:
        return !can_wear_armour(item, false, true);

    case OBJ_SCROLLS:
        if (!item_type_known(item))
            return false;

        // A bad item is always useless.
        if (is_bad_item(item, temp))
            return true;

        switch (item.sub_type)
        {
        case SCR_RANDOM_USELESSNESS:
            return true;
        case SCR_TELEPORTATION:
            return crawl_state.game_is_sprint();
        case SCR_AMNESIA:
            return (you.religion == GOD_TROG);
        case SCR_RECHARGING:
        case SCR_CURSE_WEAPON: // for non-Ashenzari, already handled
        case SCR_CURSE_ARMOUR:
        case SCR_ENCHANT_WEAPON_I:
        case SCR_ENCHANT_WEAPON_II:
        case SCR_ENCHANT_WEAPON_III:
        case SCR_ENCHANT_ARMOUR:
        case SCR_VORPALISE_WEAPON:
            return (you.species == SP_FELID);
        default:
            return false;
        }

    case OBJ_WANDS:
        if (you.species == SP_FELID)
            return true;

        if (item.sub_type == WAND_INVISIBILITY
            && item_type_known(item)
                && _invisibility_is_useless(temp))
        {
            return true;
        }

        return (item.plus2 == ZAPCOUNT_EMPTY)
                || item_ident(item, ISFLAG_KNOW_PLUSES) && !item.plus;

    case OBJ_POTIONS:
    {
        // Mummies can't use potions.
        if (you.species == SP_MUMMY)
            return true;

        if (!item_type_known(item))
            return false;

        // A bad item is always useless.
        if (is_bad_item(item, temp))
            return true;

        switch (item.sub_type)
        {
        case POT_BERSERK_RAGE:
            return (you.is_undead
                        && (you.species != SP_VAMPIRE
                            || temp && you.hunger_state <= HS_SATIATED));

        case POT_CURE_MUTATION:
#if TAG_MAJOR_VERSION == 34
        case POT_GAIN_STRENGTH:
        case POT_GAIN_INTELLIGENCE:
        case POT_GAIN_DEXTERITY:
#endif
        case POT_BENEFICIAL_MUTATION:
            return (you.is_undead
                        && (you.species != SP_VAMPIRE
                            || temp && you.hunger_state < HS_SATIATED));

        case POT_FLIGHT:
            return you.permanent_flight();

        case POT_PORRIDGE:
        case POT_BLOOD:
        case POT_BLOOD_COAGULATED:
            return !can_ingest(item, true, false);
        case POT_POISON:
        case POT_STRONG_POISON:
            // If you're poison resistant, poison is only useless.
            // Spriggans could argue, but it's too small of a gain for
            // possible player confusion.
            return (player_res_poison(false) > 0);

        case POT_INVISIBILITY:
            return _invisibility_is_useless(temp);
        }

        return false;
    }
    case OBJ_JEWELLERY:
        if (!item_type_known(item))
            return false;

        // Potentially useful.  TODO: check the properties.
        if (is_artefact(item))
            return false;

        if (is_bad_item(item, temp))
            return true;

        switch (item.sub_type)
        {
        case AMU_RAGE:
            return (you.is_undead
                        && (you.species != SP_VAMPIRE
                            || temp && you.hunger_state <= HS_SATIATED));

        case AMU_CLARITY:
            return (you.clarity(false, false));

        case AMU_RESIST_CORROSION:
            return (you.res_corr(false, false));

        case AMU_THE_GOURMAND:
            return (player_likes_chunks(true) == 3
                      && player_mutation_level(MUT_SAPROVOROUS) == 3
                      && you.species != SP_GHOUL // makes clean chunks
                                                 // contaminated
                    || player_mutation_level(MUT_GOURMAND) > 0
                    || player_mutation_level(MUT_HERBIVOROUS) == 3
                    || you.is_undead
                        && you.species != SP_GHOUL);

        case AMU_FAITH:
            return (you.species == SP_DEMIGOD && !you.religion);

        case RING_LIFE_PROTECTION:
            return (player_prot_life(false, temp, false) == 3);

        case RING_HUNGER:
        case RING_SUSTENANCE:
            return (you.species == SP_MUMMY
                    || temp && you.species == SP_VAMPIRE
                       && you.hunger_state == HS_STARVING);

        case RING_REGENERATION:
            return ((player_mutation_level(MUT_SLOW_HEALING) == 3)
                    || temp && you.species == SP_VAMPIRE
                       && you.hunger_state == HS_STARVING);

        case RING_SEE_INVISIBLE:
            return (you.can_see_invisible(false, false));

        case RING_POISON_RESISTANCE:
            return (player_res_poison(false, temp, false) > 0
                    && (temp || you.species != SP_VAMPIRE));

#if TAG_MAJOR_VERSION == 34
        case AMU_CONTROLLED_FLIGHT:
            return (player_genus(GENPC_DRACONIAN)
                    || (you.species == SP_TENGU && you.experience_level >= 5));
#endif

        case RING_WIZARDRY:
            return (you.religion == GOD_TROG);

        case RING_TELEPORT_CONTROL:
            return crawl_state.game_is_zotdef();

        case RING_TELEPORTATION:
            return crawl_state.game_is_sprint();

        case RING_INVISIBILITY:
            return _invisibility_is_useless(temp);

        case RING_FLIGHT:
            return you.permanent_flight();

        default:
            return false;
        }

    case OBJ_RODS:
        if (you.species == SP_FELID)
            return true;
        break;

    case OBJ_STAVES:
        if (you.species == SP_FELID)
            return true;
        if (you.religion == GOD_TROG)
            return true;
        if (!item_type_known(item))
            return false;
        break;

    case OBJ_FOOD:
        if (item.sub_type == NUM_FOODS)
            break;
        if (!is_inedible(item))
            return false;

        if (item.sub_type == FOOD_CHUNK
            && (you.has_spell(SPELL_SUBLIMATION_OF_BLOOD)
                || you.has_spell(SPELL_SIMULACRUM)
                || !temp && you.form == TRAN_LICH))
        {
            return false;
        }

        if (is_fruit(item) && you.religion == GOD_FEDHAS)
            return false;

        return true;

    case OBJ_CORPSES:
        if (item.sub_type != CORPSE_SKELETON)
            return false;

        if (you.has_spell(SPELL_ANIMATE_DEAD)
            || you.has_spell(SPELL_ANIMATE_SKELETON)
            || you.religion == GOD_YREDELEMNUL && !you.penance[GOD_YREDELEMNUL]
               && you.piety >= piety_breakpoint(0))
        {
            return false;
        }

        return true;

    case OBJ_MISCELLANY:
        switch (item.sub_type)
        {
#if TAG_MAJOR_VERSION == 34
        case MISC_EMPTY_EBONY_CASKET:
            return item_type_known(item);
#endif
        case MISC_LAMP_OF_FIRE:
            return !you.skill(SK_FIRE_MAGIC);
        case MISC_AIR_ELEMENTAL_FAN:
            return !you.skill(SK_AIR_MAGIC);
        case MISC_HORN_OF_GERYON:
            return item.plus2;
        default:
            return false;
        }

    case OBJ_BOOKS:
        if (item.sub_type != BOOK_MANUAL || !item_type_known(item))
            return false;
        if (you.skills[item.plus] >= 27)
            return true;
        if (is_useless_skill((skill_type)item.plus))
            return true;
        return false;

    default:
        return false;
    }
    return false;
}

string item_prefix(const item_def &item, bool temp)
{
    vector<string> prefixes;

    if (fully_identified(item))
        prefixes.push_back("identified");
    else if (item_ident(item, ISFLAG_KNOW_TYPE)
             || get_ident_type(item) == ID_KNOWN_TYPE)
    {
        prefixes.push_back("known");
    }
    else
        prefixes.push_back("unidentified");

    if (good_god_hates_item_handling(item) || god_hates_item_handling(item))
    {
        prefixes.push_back("evil_item");
        prefixes.push_back("forbidden");
    }

    if (is_emergency_item(item))
        prefixes.push_back("emergency_item");
    if (is_good_item(item))
        prefixes.push_back("good_item");
    if (is_dangerous_item(item, temp))
        prefixes.push_back("dangerous_item");
    if (is_bad_item(item, temp))
        prefixes.push_back("bad_item");
    if (is_useless_item(item, temp))
        prefixes.push_back("useless_item");

    switch (item.base_type)
    {
    case OBJ_CORPSES:
        // Skeletons cannot be eaten.
        if (item.sub_type == CORPSE_SKELETON)
        {
            prefixes.push_back("inedible");
            break;
        }
        // intentional fall-through
    case OBJ_FOOD:
        if (item.sub_type == NUM_FOODS)
            break;
        if (is_forbidden_food(item))
        {
            prefixes.push_back("evil_eating"); // compat with old configs
            prefixes.push_back("forbidden");
        }

        if (is_inedible(item))
            prefixes.push_back("inedible");
        else if (is_preferred_food(item))
            prefixes.push_back("preferred");

        if (is_poisonous(item))
            prefixes.push_back("poisonous"), prefixes.push_back("inedible");
        else if (is_mutagenic(item))
            prefixes.push_back("mutagenic");
        else if (causes_rot(item))
            prefixes.push_back("rot-inducing"), prefixes.push_back("inedible");
        else if (is_contaminated(item))
            prefixes.push_back("contaminated");
        break;

    case OBJ_POTIONS:
        if (is_good_god(you.religion) && item_type_known(item)
            && is_blood_potion(item))
        {
            prefixes.push_back("evil_eating");
            prefixes.push_back("forbidden");
        }
        if (is_preferred_food(item))
            prefixes.push_back("preferred");
        break;

    case OBJ_WEAPONS:
    case OBJ_ARMOUR:
    case OBJ_JEWELLERY:
        if (is_artefact(item))
            prefixes.push_back("artefact");
        // fall through

    case OBJ_STAVES:
    case OBJ_RODS:
    case OBJ_MISSILES:
        if (item_is_equipped(item, true))
            prefixes.push_back("equipped");
        break;

    case OBJ_BOOKS:
        if (item_is_active_manual(item))
            prefixes.push_back("equipped");
        break;

    default:
        break;
    }

    prefixes.push_back(item_class_name(item.base_type, true));

    string result = comma_separated_line(prefixes.begin(), prefixes.end(),
                                         " ", " ");

    return result;
}

string get_menu_colour_prefix_tags(const item_def &item,
                                   description_level_type desc)
{
    string cprf       = item_prefix(item);
    string colour     = "";
    string colour_off = "";
    string item_name  = item.name(true, desc);
    int col = menu_colour(item.name(false, desc), cprf, "pickup");

    if (col != -1)
        colour = colour_to_str(col);

    if (!colour.empty())
    {
        // Order is important here.
        colour_off  = "</" + colour + ">";
        colour      = "<" + colour + ">";
        item_name = colour + item_name + colour_off;
    }

    return item_name;
}

typedef map<string, item_kind> item_names_map;
static item_names_map item_names_cache;

typedef map<unsigned, vector<string> > item_names_by_glyph_map;
static item_names_by_glyph_map item_names_by_glyph_cache;

void init_item_name_cache()
{
    item_names_cache.clear();
    item_names_by_glyph_cache.clear();

    for (int i = 0; i < NUM_OBJECT_CLASSES; i++)
    {
        object_class_type base_type = static_cast<object_class_type>(i);
        const int num_sub_types = get_max_subtype(base_type);

        for (int sub_type = 0; sub_type < num_sub_types; sub_type++)
        {
            if (base_type == OBJ_BOOKS)
            {
                if (sub_type == BOOK_RANDART_LEVEL
                    || sub_type == BOOK_RANDART_THEME)
                {
                    // These are randart only and have no fixed names.
                    continue;
                }
            }

            int npluses = 0;
            if (base_type == OBJ_BOOKS && sub_type == BOOK_MANUAL)
                npluses = NUM_SKILLS;
            else if (base_type == OBJ_MISCELLANY && sub_type == MISC_RUNE_OF_ZOT)
                npluses = NUM_RUNE_TYPES;

            item_def item;
            item.base_type = base_type;
            item.sub_type = sub_type;
            for (int plus = 0; plus <= npluses; plus++)
            {
                if (plus > 0)
                    item.plus = max(0, plus - 1);
                if (is_deck(item))
                {
                    item.plus = 1;
                    item.special = DECK_RARITY_COMMON;
                    init_deck(item);
                }
                string name = item.name(false, plus ? DESC_PLAIN : DESC_DBNAME,
                                        true, true);
                lowercase(name);
                cglyph_t g = get_item_glyph(&item);

                if (base_type == OBJ_JEWELLERY && sub_type >= NUM_RINGS
                    && sub_type < AMU_FIRST_AMULET)
                {
                    continue;
                }
                else if (name.find("buggy") != string::npos)
                {
                    crawl_state.add_startup_error("Bad name for item name "
                                                  " cache: " + name);
                    continue;
                }

                if (item_names_cache.find(name) == item_names_cache.end())
                {
                    item_kind kind = {base_type, (uint8_t)sub_type,
                                      (int8_t)item.plus, 0};
                    item_names_cache[name] = kind;
                    if (g.ch)
                        item_names_by_glyph_cache[g.ch].push_back(name);
                }
            }
        }
    }

    ASSERT(!item_names_cache.empty());
}

item_kind item_kind_by_name(string name)
{
    lowercase(name);

    item_names_map::iterator i = item_names_cache.find(name);

    if (i != item_names_cache.end())
        return i->second;

    item_kind err = {OBJ_UNASSIGNED, 0, 0, 0};

    return err;
}

vector<string> item_name_list_for_glyph(unsigned glyph)
{
    item_names_by_glyph_map::iterator i;
    i = item_names_by_glyph_cache.find(glyph);

    if (i != item_names_by_glyph_cache.end())
        return i->second;

    vector<string> empty;
    return empty;
}

bool is_named_corpse(const item_def &corpse)
{
    ASSERT(corpse.base_type == OBJ_CORPSES);

    return corpse.props.exists(CORPSE_NAME_KEY);
}

string get_corpse_name(const item_def &corpse, uint64_t *name_type)
{
    ASSERT(corpse.base_type == OBJ_CORPSES);

    if (!corpse.props.exists(CORPSE_NAME_KEY))
        return "";

    if (name_type != NULL)
        *name_type = corpse.props[CORPSE_NAME_TYPE_KEY].get_int64();

    return corpse.props[CORPSE_NAME_KEY].get_string();
}

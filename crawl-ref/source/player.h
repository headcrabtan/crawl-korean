/*
 *  File:       player.cc
 *  Summary:    Player related functions.
 *  Written by: Linley Henzell
 */


#ifndef PLAYER_H
#define PLAYER_H

#include "actor.h"
#include "itemprop.h"

#ifdef USE_TILE
// This used to be in tiles.h. (jpeg)
#include "tiledef-main.h"
#include "tiledef-dngn.h"
#include "tiledef-player.h"

struct dolls_data
{
    dolls_data() { memset(parts, 0, sizeof(parts)); }

    int parts[TILEP_PART_MAX];
};
#endif

class player : public actor
{
public:
  bool turn_is_over; // flag signaling that player has performed a timed action

  // If true, player is headed to the Abyss.
  bool banished;
  std::string banished_by;

  std::vector<int> mesmerised_by; // monsters mesmerising player

  int  friendly_pickup;       // pickup setting for allies

  unsigned short prev_targ;
  coord_def      prev_grd_targ;
  char your_name[kNameLen];

  species_type species;
  job_type char_class;

  // Coordinates of last travel target; note that this is never used by
  // travel itself, only by the level-map to remember the last travel target.
  short travel_x, travel_y;

  runrest running;            // Nonzero if running/traveling.

  unsigned short unrand_reacts;

  double elapsed_time;        // total amount of elapsed time in the game

  unsigned char synch_time;   // amount to wait before calling handle_time()

  unsigned char disease;

  char max_level;

  coord_def youpos;

  coord_def prev_move;

  int hunger;
  FixedVector<char, NUM_EQUIP> equip;

  int hp;
  int hp_max;
  int base_hp;                // temporary max HP loss (rotting)
  int base_hp2;               // base HPs from levels (and permanent loss)

  int magic_points;
  int max_magic_points;
  int base_magic_points;      // temporary max MP loss? (currently unused)
  int base_magic_points2;     // base MPs from levels and potions of magic

  char strength;
  char intel;
  char dex;
  char max_strength;
  char max_intel;
  char max_dex;
  stat_type last_chosen;

  char hunger_state;

  bool wield_change;          // redraw weapon
  bool redraw_quiver;         // redraw quiver
  bool received_weapon_warning;

  unsigned long redraw_status_flags;

  // PC's symbol (usually @) and colour.
  int symbol;
  int colour;

  bool redraw_hit_points;
  bool redraw_magic_points;
  bool redraw_strength;
  bool redraw_intelligence;
  bool redraw_dexterity;
  bool redraw_experience;
  bool redraw_armour_class;
  bool redraw_evasion;

  unsigned char flash_colour;

  unsigned char hit_points_regeneration;
  unsigned char magic_points_regeneration;

  unsigned long experience;
  int experience_level;
  int gold;
  char class_name[30];
  int time_taken;

  int shield_blocks;         // number of shield blocks since last action

  FixedVector< item_def, ENDOFPACK > inv;

  int burden;
  burden_state_type burden_state;
  FixedVector<spell_type, 25> spells;
  char spell_no;
  game_direction_type char_direction;
  bool opened_zot;
  bool royal_jelly_dead;
  bool transform_uncancellable;

  unsigned short pet_target;

  int your_level; // offset by one (-1 == 0, 0 == 1, etc.) for display

  // durational things
  FixedVector<int, NUM_DURATIONS> duration;

  int rotting;

  int berserk_penalty;                // penalty for moving while berserk

  FixedVector<unsigned long, NUM_ATTRIBUTES> attribute;
  FixedVector<unsigned char, NUM_QUIVER> quiver; // default items for quiver
  FixedVector<long, NUM_OBJECT_CLASSES> sacrifice_value;

  undead_state_type is_undead;

  delay_queue_type delay_queue;       // pending actions

  FixedVector<unsigned char, 50>  skills;
  FixedVector<bool, 50>  practise_skill;
  FixedVector<unsigned int, 50>   skill_points;
  FixedVector<unsigned char, 50>  skill_order;

  skill_type sage_bonus_skill;  // If Sage is in effect, which skill it affects.
  int sage_bonus_degree;        // How much bonus XP to give in that skill.

  int  skill_cost_level;
  int  total_skill_points;
  int  exp_available;

  FixedArray<unsigned char, 6, 50> item_description;
  FixedVector<unique_item_status_type, MAX_UNRANDARTS> unique_items;
  FixedVector<bool, NUM_MONSTERS> unique_creatures;

  // NOTE: The kills member is a pointer to a KillMaster object,
  // rather than the object itself, so that we can get away with
  // just a foward declare of the KillMaster class, rather than
  // having to #include kills.h and thus make every single .cc file
  // dependant on kills.h.  Having a pointer means that we have
  // to do our own implementations of copying the player object,
  // since the default implementations will lead to the kills member
  // pointing to freed memory, or worse yet lead to the same piece of
  // memory being freed twice.
  KillMaster* kills;

  level_area_type level_type;

  // Human-readable name for portal vault. Will be set to level_type_tag
  // if not explicitly set by the entry portal.
  std::string level_type_name;

  // Three-letter extension for portal vault bones files. Will be set
  // to first three letters of level_type_tag if not explicitly set by
  // the entry portal.
  std::string level_type_ext;

  // Abbreviation of portal vault name, for use in notes.  If not
  // explicitly set by the portal vault, will be set from level_type_name
  // or level_type_tag if either is short enough, or the shorter of the
  // two will be truncated if neither is short enough.
  std::string level_type_name_abbrev;

  // Item origin string for items from portal vaults, so that dumps
  // can have origins like "You found it in on level 2 of a ziggurat".
  // Will be set relative to level_type_name if not explicitly set.
  std::string level_type_origin;

  // .des file tag for portal vault
  std::string level_type_tag;

  entry_cause_type entry_cause;
  god_type         entry_cause_god;

  branch_type where_are_you;

  FixedVector<unsigned char, 30> branch_stairs;

  god_type religion;
  std::string second_god_name; // Random second name of Jiyva
  unsigned char piety;
  unsigned char piety_hysteresis;       // amount of stored-up docking
  unsigned char gift_timeout;
  FixedVector<unsigned char, MAX_NUM_GODS>  penance;
  FixedVector<unsigned char, MAX_NUM_GODS>  worshipped;
  FixedVector<short,         MAX_NUM_GODS>  num_gifts;


  FixedVector<unsigned char, NUM_MUTATIONS> mutation;
  FixedVector<unsigned char, NUM_MUTATIONS> demon_pow;
  unsigned char magic_contamination;

  FixedVector<bool, NUM_FIXED_BOOKS> had_book;
  FixedVector<bool, NUM_SPELLS>      seen_spell;

  unsigned char normal_vision;        // how far the species gets to see
  unsigned char current_vision;       // current sight radius (cells)

  unsigned char hell_exit;            // which level plyr goes to on hell exit.

  // This field is here even in non-WIZARD compiles, since the
  // player might have been playing previously under wiz mode.
  bool          wizard;               // true if player has entered wiz mode.
  time_t        birth_time;           // start time of game

  time_t        start_time;           // start time of session
  long          real_time;            // real time played (in seconds)
  long          num_turns;            // number of turns taken

  long          last_view_update;     // what turn was the view last updated?

  int           old_hunger;  // used for hunger delta-meter (see output.cc)

  // Set when the character is going to a new level, to guard against levgen
  // failures
  dungeon_feature_type transit_stair;
  bool entering_level;
  int lava_in_sight;       // Is there lava in LoS?
  int water_in_sight;      // Is there deep water in LoS?
#ifdef USE_TILE
  coord_def last_clicked_grid; // The map position the player last clicked on.
  int last_clicked_item; // The inventory cell the player last clicked on.
#endif

  // Warning: these two are quite different.
  //
  // The spell table is an index to a specific spell slot (you.spells).
  // The ability table lists the ability (ABIL_*) which prefers that letter.
  //
  // In other words, the spell table contains hard links and the ability
  // table contains soft links.
  FixedVector<int, 52>  spell_letter_table;   // ref to spell by slot
  FixedVector<ability_type, 52>  ability_letter_table; // ref to abil by enum

  std::set<std::string> uniq_map_tags;
  std::set<std::string> uniq_map_names;

  PlaceInfo global_info;
  player_quiver* m_quiver;

  int         escaped_death_cause;
  std::string escaped_death_aux;

protected:
  FixedVector<PlaceInfo, NUM_BRANCHES>             branch_info;
  FixedVector<PlaceInfo, NUM_LEVEL_AREA_TYPES - 1> non_branch_info;

public:
    player();
    player(const player &other);
    ~player();

    void copy_from(const player &other);

    void init();

    // Low-level move the player. Use this instead of changing pos directly.
    void moveto(const coord_def &c);
    // Move the player during an abyss shift.
    void shiftto(const coord_def &c);

    void reset_prev_move();

    bool in_water() const;
    bool can_swim() const;
    bool is_levitating() const;
    bool cannot_speak() const;
    bool invisible() const;
    bool can_see_invisible() const;
    bool can_see_invisible(bool unid) const;
    bool visible_to(const actor *looker) const;
    bool see_cell(const coord_def &c) const;
    bool can_see(const actor *target) const;
    bool is_icy() const;
    bool is_fiery() const;

    bool light_flight() const;
    bool travelling_light() const;

    kill_category kill_alignment() const;

    bool has_spell(spell_type spell) const;

    size_type transform_size(int psize = PSIZE_TORSO) const;
    std::string shout_verb() const;

    item_def *slot_item(equipment_type eq);

    // actor
    int id() const;
    int mindex() const;
    int       get_experience_level() const;
    actor_type atype() const { return ACT_PLAYER; }

    god_type  deity() const;
    bool      alive() const;
    bool      is_summoned(int* duration = NULL, int* summon_type = NULL) const;

    bool      swimming() const;
    bool      submerged() const;
    bool      floundering() const;
    bool      extra_balanced() const;
    bool      can_pass_through_feat(dungeon_feature_type grid) const;
    bool      is_habitable_feat(dungeon_feature_type actual_grid) const;
    size_type body_size(int psize = PSIZE_TORSO, bool base = false) const;
    int       body_weight() const;
    int       total_weight() const;
    int       damage_brand(int which_attack = -1);
    int       damage_type(int which_attack = -1);
    int       has_claws(bool allow_tran = true) const;
    bool      has_usable_claws(bool allow_tran = true) const;
    item_def *weapon(int which_attack = -1);
    item_def *shield();

    bool      can_wield(const item_def &item,
                        bool ignore_curse = false,
                        bool ignore_brand = false,
                        bool ignore_shield = false,
                        bool ignore_transform = false) const;
    bool      could_wield(const item_def &item,
                          bool ignore_brand = false,
                          bool ignore_transform = false) const;

    std::string name(description_level_type type,
                     bool force_visible = false) const;
    std::string pronoun(pronoun_type pro, bool force_visible = false) const;
    std::string conj_verb(const std::string &verb) const;
    std::string hand_name(bool plural, bool *can_plural = NULL) const;
    std::string foot_name(bool plural, bool *can_plural = NULL) const;
    std::string arm_name(bool plural, bool *can_plural = NULL) const;

    bool fumbles_attack(bool verbose = true);
    bool cannot_fight() const;

    void attacking(actor *other);
    bool can_go_berserk() const;
    bool can_go_berserk(bool verbose) const;
    void go_berserk(bool intentional);
    bool can_mutate() const;
    bool can_safely_mutate() const;
    bool can_bleed() const;
    bool mutate();
    void backlight();
    void banish(const std::string &who = "");
    void blink(bool allow_partial_control = true);
    void teleport(bool right_now = false, bool abyss_shift = false);
    void drain_stat(int stat, int amount, actor* attacker);

    void expose_to_element(beam_type element, int strength = 0);
    void god_conduct(conduct_type thing_done, int level);

    int hunger_level() const { return hunger_state; }
    void make_hungry(int nutrition, bool silent = true);
    void poison(actor *agent, int amount = 1);
    bool sicken(int amount);
    void paralyse(actor *, int str);
    void petrify(actor *, int str);
    void slow_down(actor *, int str);
    void confuse(actor *, int strength);
    void heal(int amount, bool max_too = false);
    bool drain_exp(actor *, bool quiet = false, int pow = 3);
    bool rot(actor *, int amount, int immediate = 0, bool quiet = false);
    int hurt(const actor *attacker, int amount,
             beam_type flavour = BEAM_MISSILE,
             bool cleanup_dead = true);

    int warding() const;

    int mons_species() const;

    mon_holy_type holiness() const;
    int res_fire() const;
    int res_steam() const;
    int res_cold() const;
    int res_elec() const;
    int res_poison() const;
    int res_rotting() const;
    int res_asphyx() const;
    int res_sticky_flame() const;
    int res_holy_energy(const actor *) const;
    int res_negative_energy() const;
    int res_torment() const;
    bool confusable() const;
    bool slowable() const;

    flight_type flight_mode() const;
    bool permanent_levitation() const;
    bool permanent_flight() const;

    bool paralysed() const;
    bool cannot_move() const;
    bool cannot_act() const;
    bool confused() const;
    bool caught() const;
    bool backlit(bool check_haloed = true) const;
    bool haloed() const;

    bool asleep() const;
    void put_to_sleep(int power = 0);
    void awake();
    void check_awaken(int disturbance);

    bool can_throw_large_rocks() const;

    int armour_class() const;
    int melee_evasion(const actor *attacker) const;

    int stat_hp() const     { return hp; }
    int stat_maxhp() const  { return hp_max; }

    int shield_bonus() const;
    int shield_block_penalty() const;
    int shield_bypass_ability(int tohit) const;

    void shield_block_succeeded();

    bool wearing_light_armour(bool with_skill = false) const;
    void exercise(skill_type skill, int qty);
    int  skill(skill_type skill, bool skill_bump = false) const;

    PlaceInfo& get_place_info() const ; // Current place info
    PlaceInfo& get_place_info(branch_type branch,
                              level_area_type level_type2) const;
    PlaceInfo& get_place_info(branch_type branch) const;
    PlaceInfo& get_place_info(level_area_type level_type2) const;

    void set_place_info(PlaceInfo info);
    // Returns copies of the PlaceInfo; modifying the vector won't
    // modify the player object.
    std::vector<PlaceInfo> get_all_place_info(bool visited_only = false,
                                              bool dungeon_only = false) const;

    bool do_shaft();

    bool did_escape_death() const;
    void reset_escaped_death();
protected:
    void base_moveto(const coord_def &c);
};

extern player you;

struct player_save_info
{
    std::string name;
    unsigned long experience;
    int experience_level;
    bool wizard;
    species_type species;
    std::string class_name;
    god_type religion;
    std::string second_god_name;
#ifdef USE_TILE
    dolls_data doll;
    bool held_in_net;
#endif

    player_save_info operator=(const player& rhs);
    bool operator<(const player_save_info& rhs) const;
    std::string short_desc() const;
};

class monsters;
struct item_def;

enum genus_type
{
    GENPC_DRACONIAN,                   //    0
    GENPC_ELVEN,                       //    1
    GENPC_DWARVEN,                     //    2
    GENPC_OGRE
};

bool move_player_to_grid( const coord_def& p, bool stepped, bool allow_shift,
                          bool force, bool swapping = false );

bool player_in_mappable_area(void);
bool player_in_branch(int branch);
bool player_in_hell(void);

int get_player_wielded_weapon(void);
bool berserk_check_wielded_weapon(void);
int player_equip( equipment_type slot, int sub_type, bool calc_unid = true );
int player_equip_ego_type( int slot, int sub_type );
bool player_equip_unrand( int unrand_index );
int player_damage_brand(void);
int player_damage_type(void);
bool player_can_hit_monster(const monsters *mon);

bool player_is_shapechanged(void);

/* ***********************************************************************
 * called from: player - item_use
 * *********************************************************************** */
bool is_light_armour( const item_def &item );

/* ***********************************************************************
 * called from: beam - fight - misc - newgame
 * *********************************************************************** */
bool player_light_armour(bool with_skill = false);


/* ***********************************************************************
 * called from: acr.cc - fight.cc - misc.cc - player.cc
 * *********************************************************************** */
bool player_in_water(void);
bool player_is_swimming(void);
bool player_is_airborne(void);

/* ***********************************************************************
 * called from: ability - chardump - fight - religion - spell - spells -
 *              spells0 - spells2
 * *********************************************************************** */
bool player_under_penance(void);

int player_wielded_item();

/* ***********************************************************************
 * called from: ability - acr - fight - food - it_use2 - item_use - items -
 *              misc - mutation - ouch
 * *********************************************************************** */

bool extrinsic_amulet_effect(jewellery_type amulet);
bool wearing_amulet(jewellery_type which_am, bool calc_unid = true);


/* ***********************************************************************
 * called from: acr - chardump - describe - newgame - view
 * *********************************************************************** */
std::string species_name( species_type speci, int level, bool genus = false,
                          bool adj = false );
int str_to_species(const std::string &species);

/* ***********************************************************************
 * called from: beam
 * *********************************************************************** */
bool you_resist_magic(int power);


/* ***********************************************************************
 * called from: acr - decks - effects - it_use2 - it_use3 - item_use -
 *              items - output - shopping - spells1 - spells3
 * *********************************************************************** */
int burden_change(void);


/* ***********************************************************************
 * called from: items - misc
 * *********************************************************************** */
int carrying_capacity(burden_state_type bs = BS_OVERLOADED);


/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
int check_stealth(void);


/* ***********************************************************************
 * called from: bang - beam - chardump - fight - files - it_use2 -
 *              item_use - misc - output - spells - spells1
 * *********************************************************************** */
int player_AC(void);


/* ***********************************************************************
 * called from: spell
 * *********************************************************************** */
int player_energy(void);


/* ***********************************************************************
 * called from: beam - chardump - fight - files - misc - output
 * *********************************************************************** */
int player_evasion(void);


/* ***********************************************************************
 * called from: acr - spells1
 * *********************************************************************** */
int player_movement_speed(void);


/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
int player_hunger_rate(void);

int calc_hunger(int food_cost);

/* ***********************************************************************
 * called from: debug - it_use3 - spells0
 * *********************************************************************** */
int player_mag_abil(bool is_weighted);
int player_magical_power(void);

/* ***********************************************************************
 * called from: fight - misc - ouch - spells
 * *********************************************************************** */
int player_prot_life(bool calc_unid = true, bool temp = true,
                     bool items = true);


/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
int player_regen(void);


/* ***********************************************************************
 * called from: fight - files - it_use2 - misc - ouch - spells - spells2
 * *********************************************************************** */
int player_res_cold(bool calc_unid = true, bool temp = true,
                    bool items = true);
int player_res_acid(bool calc_unid = true, bool items = true);
int player_acid_resist_factor();

int player_res_torment(bool calc_unid = true);

bool player_item_conserve(bool calc_unid = true);
int player_mental_clarity(bool calc_unid = true, bool items = true);
int player_spirit_shield(bool calc_unid = true);

bool player_can_smell();
bool player_likes_chunks(bool permanently = false);
bool player_can_swim();
bool player_likes_water(bool permanently = false);

bool player_is_unholy();
int player_mutation_level(mutation_type mut);

/* ***********************************************************************
 * called from: fight - files - ouch
 * *********************************************************************** */
int player_res_electricity(bool calc_unid = true, bool temp = true,
                           bool items = true);


/* ***********************************************************************
 * called from: acr - fight - misc - ouch - spells
 * *********************************************************************** */
int player_res_fire(bool calc_unid = true, bool temp = true,
                    bool items = true);
int player_res_sticky_flame(bool calc_unid = true, bool temp = true,
                            bool items = true);
int player_res_steam(bool calc_unid = true, bool temp = true,
                     bool items = true);


/* ***********************************************************************
 * called from: beam - decks - fight - fod - it_use2 - misc - ouch -
 *              spells - spells2
 * *********************************************************************** */
int player_res_poison(bool calc_unid = true, bool temp = true,
                      bool items = true);
int player_res_rotting();

bool player_control_teleport(bool calc_unid = true, bool temp = true,
                             bool items = true);

int player_res_magic(void);

int player_res_asphyx();

/* ***********************************************************************
 * called from: beam - chardump - fight - misc - output
 * *********************************************************************** */
int player_shield_class(void);


int player_spec_air(void);
int player_spec_cold(void);
int player_spec_conj(void);
int player_spec_death(void);
int player_spec_earth(void);
int player_spec_ench(void);
int player_spec_fire(void);
int player_spec_holy(void);
int player_spec_poison(void);
int player_spec_summ(void);


/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
int player_speed(void);


/* ***********************************************************************
 * called from: chardump - spells
 * *********************************************************************** */
int player_spell_levels(void);

/* ***********************************************************************
 * called from: libgui - item_use
 * *********************************************************************** */
bool player_knows_spell(int spell);

// last updated 18may2000 {dlb}
/* ***********************************************************************
 * called from: effects
 * *********************************************************************** */
int player_sust_abil(bool calc_unid = true);
void jiyva_stat_action();


/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
int player_teleport(bool calc_unid = true);


/* ***********************************************************************
 * called from: ability - acr - items - misc - spells1 - spells3
 * *********************************************************************** */
bool items_give_ability(const int slot, artefact_prop_type abil);
int scan_artefacts(artefact_prop_type which_property, bool calc_unid = true);


/* ***********************************************************************
 * called from: fight - item_use
 * *********************************************************************** */
int slaying_bonus(char which_affected);


bool player_mesmerised_by(const monsters *mon);
void update_beholders(const monsters *mon, bool force = false);
void check_beholders();

/* ***********************************************************************
 * called from: acr - decks - it_use2 - ouch
 * *********************************************************************** */
unsigned long exp_needed(int lev);


/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
int get_expiration_threshold(duration_type dur);
bool dur_expiring(duration_type dur);
void display_char_status(void);


/* ***********************************************************************
 * called from: item_use - items - misc - spells - spells3
 * *********************************************************************** */
void forget_map(unsigned char chance_forgotten = 100, bool force = false);


// last updated 19may2000 {dlb}
/* ***********************************************************************
 * called from: acr - fight
 * *********************************************************************** */
void gain_exp(unsigned int exp_gained, unsigned int* actual_gain = NULL,
              unsigned int* actual_avail_gain = NULL);

// last updated 17dec2000 {gdl}
/* ***********************************************************************
 * called from: acr - it_use2 - item_use - mutation - transfor - player -
 *              misc - stuff
 * *********************************************************************** */
void modify_stat(stat_type which_stat, char amount, bool suppress_msg,
                 const std::string& cause, bool see_source = true);
void modify_stat(stat_type which_stat, char amount, bool suppress_msg,
                 const char* cause, bool see_source = true);
void modify_stat(stat_type which_stat, char amount, bool suppress_msg,
                 const monsters* cause);
void modify_stat(stat_type which_stat, char amount, bool suppress_msg,
                 const item_def &cause, bool removed = false);

bool player_in_bat_form();
bool player_can_open_doors();

// last updated 19may2000 {dlb}
/* ***********************************************************************
 * called from: decks - it_use2 - player
 * *********************************************************************** */
void level_change(bool skip_attribute_increase = false);


/* ***********************************************************************
 * called from: ability - fight - item_use - mutation - newgame - spells0 -
 *              transfor
 * *********************************************************************** */
bool player_genus( genus_type which_genus,
                   species_type species = SP_UNKNOWN );
bool is_player_same_species( const int mon, bool = false );

bool you_can_wear( int eq, bool special_armour = false );
bool player_has_feet(void);
bool player_wearing_slot( int eq );
bool you_tran_can_wear(const item_def &item);
bool you_tran_can_wear( int eq, bool check_mutation = false );

bool enough_hp(int minimum, bool suppress_msg);
bool enough_mp(int minimum, bool suppress_msg, bool include_items = true);

void dec_hp(int hp_loss, bool fatal, const char *aux = NULL);
void dec_mp(int mp_loss);

void inc_mp(int mp_gain, bool max_too);
void inc_hp(int hp_gain, bool max_too);

void rot_hp(int hp_loss);
void unrot_hp(int hp_recovered);
int player_rotted();
void rot_mp(int mp_loss);

void inc_max_hp( int hp_gain );
void dec_max_hp( int hp_loss );

void inc_max_mp( int mp_gain );
void dec_max_mp( int mp_loss );

void deflate_hp(int new_level, bool floor);
void set_hp(int new_amount, bool max_too);

int get_real_hp(bool trans, bool rotted = false);
int get_real_mp(bool include_items);

int get_contamination_level();

void set_mp(int new_amount, bool max_too);

void contaminate_player(int change, bool controlled = false,
                        bool status_only = false);

bool confuse_player(int amount, bool resistable = true);

bool curare_hits_player(int death_source, int amount);
bool poison_player(int amount, bool force = false);
void dec_poison_player();
void reduce_poison_player(int amount);
bool miasma_player();

bool napalm_player(int amount);
void dec_napalm_player();

bool slow_player(int amount);
void dec_slow_player();

void haste_player(int amount);
void dec_haste_player();

bool disease_player(int amount);
void dec_disease_player();

bool rot_player(int amount);

bool player_has_spell(spell_type spell);
size_type player_size(int psize = PSIZE_TORSO, bool base = false);

item_def *player_slot_item(equipment_type eq);

bool player_weapon_wielded();
item_def *player_weapon();
item_def *player_shield();

// Determines if the given grid is dangerous for the player to enter.
bool is_feat_dangerous(dungeon_feature_type feat);

void run_macro(const char *macroname = NULL);

int count_worn_ego(int which_ego);
int stat_modifier(stat_type stat);

#endif

#ifndef COMBAT_H
#define COMBAT_H

class MOB;

// ============================================================================
// Combat System
// ============================================================================

// Combat statistics that can be modified by items/skills/buffs
struct CombatStats {
    int base_damage;        // Base damage before modifiers
    int armor;              // Damage reduction
    double hit_chance;      // Chance to hit (0.0 to 1.0)
    double crit_chance;     // Chance for critical hit
    double crit_multiplier; // Critical damage multiplier

    CombatStats()
        : base_damage(10), armor(0), hit_chance(0.9),
          crit_chance(0.1), crit_multiplier(2.0) {}
};

// Result of a combat action
struct CombatResult {
    bool hit;               // Did the attack hit?
    bool critical;          // Was it a critical hit?
    int damage_dealt;       // Total damage dealt
    bool target_killed;     // Did the attack kill the target?

    CombatResult()
        : hit(false), critical(false), damage_dealt(0), target_killed(false) {}
};

class Combat {
public:
    // Main combat resolution
    static CombatResult resolve_attack(MOB* attacker, MOB* defender);

private:
    // Combat phases (extensible for future features)
    static CombatStats get_attack_stats(MOB* attacker);
    static CombatStats get_defense_stats(MOB* defender);
    static bool roll_to_hit(double hit_chance);
    static bool roll_critical(double crit_chance);
    static int calculate_damage(const CombatStats& attack_stats,
                                 const CombatStats& defense_stats,
                                 bool is_critical);
    static void apply_damage(MOB* defender, int damage);
};

#endif // COMBAT_H

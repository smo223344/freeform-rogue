#ifndef MOB_H
#define MOB_H

#include "entity.h"

// Forward declarations
struct CombatStats;
struct CombatResult;

// ============================================================================
// MOB System - Mobile entities with HP and action points
// ============================================================================

class MOB {
public:
    Entity* entity;              // Associated entity
    int hp;                      // Current hit points
    int max_hp;                  // Maximum hit points
    double ap_rate;              // Action points accumulated per turn
    double action_points;        // Current action points

    MOB(Entity* ent, int max_health, double ap_accumulation);
    virtual ~MOB() = default;

    // Action point management
    void accumulate_ap();
    bool can_act(double cost) const;
    void spend_ap(double cost);

    // Combat
    virtual void take_damage(int damage);
    bool is_alive() const;

    // Combat hooks - can be overridden for items/skills/buffs
    // These allow extensibility for equipment, passive abilities, etc.
    virtual void modify_attack_stats(CombatStats& stats) const;
    virtual void modify_defense_stats(CombatStats& stats) const;
    virtual void on_attack(MOB* target, CombatResult& result);
    virtual void on_defend(MOB* attacker, const CombatResult& result);
    virtual void on_kill(MOB* target);
    virtual void on_death(MOB* killer);

    // Actions - to be overridden by subclasses
    virtual void take_turn() = 0;  // Pure virtual - must be implemented

    // Action costs
    static constexpr double MOVE_COST = 100.0;
    static constexpr double ATTACK_COST = 100.0;

protected:
    // Protected members for subclasses
};

// ============================================================================
// Player - Human-controlled MOB
// ============================================================================

class Player : public MOB {
public:
    Player(Entity* ent);

    void take_turn() override;
};

// ============================================================================
// Enemy - AI-controlled hostile MOB
// ============================================================================

class Enemy : public MOB {
public:
    Enemy(Entity* ent, int max_health, double ap_accumulation);

    void take_turn() override;
};

#endif // MOB_H

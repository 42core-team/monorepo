export interface Pos {
    x: number;
    y: number;
}

export enum ObjType {
    CORE = 0,
    UNIT = 1,
    RESOURCE = 2,
    WALL = 3,
    MONEY = 4,
    BOMB = 5
}

export enum UnitType {
    WARRIOR = 0,
    MINER = 1,
    CARRIER = 2,
    TANK = 3
}

export enum BuildType {
    NONE = 0,
    WALL = 1,
    BOMB = 2
}

export enum ObjState {
    ALIVE = "alive",
    DEAD = "dead"
}

export interface UnitConfig {
    name: string;
    unit_type: UnitType;
    cost: number;
    hp: number;
    baseActionCooldown: number;
    maxActionCooldown: number;
    balancePerCooldownStep: number;
    dmg_core: number;
    dmg_unit: number;
    dmg_deposit: number;
    dmg_wall: number;
    dmg_bomb: number;
    build_type: BuildType;
}

export interface Config {
    gridSize: number;
    idle_income: number;
    idle_income_timeout: number;
    deposit_hp: number;
    deposit_income: number;
    gem_pile_income: number;
    core_hp: number;
    core_spawn_cooldown: number;
    initial_balance: number;
    wall_hp: number;
    wall_build_cost: number;
    bomb_countdown: number;
    bomb_throw_cost: number;
    bomb_reach: number;
    bomb_damage_core: number;
    bomb_damage_unit: number;
    bomb_damage_deposit: number;
    units: UnitConfig[];
}

export interface Obj {
    type: ObjType;
    id: number;
    pos: Pos;
    hp: number;
    state: ObjState;
    s_core: {
        team_id: number;
        gems: number;
        spawn_cooldown: number;
        balance: number;
    };
    s_unit: {
        unit_type: UnitType;
        team_id: number;
        gems: number;
        action_cooldown: number;
        balance: number;
    };
    s_deposit_gems_pile: {
        gems: number;
    };
    s_bomb: {
        countdown: number;
    };
}

export type ObjUnit = Obj & { type: ObjType.UNIT };
export type ObjCore = Obj & { type: ObjType.CORE };

export interface Game {
    elapsed_ticks: number;
    config: Config;
    my_team_id: number;
    objects: Obj[];
}

export type GameState = Game;

export enum ActionType {
    CREATE = "create",
    MOVE = "move",
    ATTACK = "attack",
    TRANSFER = "transfer_gems",
    BUILD = "build"
}

export type Action =
    | { type: ActionType.CREATE, unit_type: UnitType }
    | { type: ActionType.MOVE, unit_id: number, x: number, y: number }
    | { type: ActionType.ATTACK, unit_id: number, target_id: number }
    | { type: ActionType.TRANSFER, source_id: number, x: number, y: number, amount: number }
    | { type: ActionType.BUILD, unit_id: number, x: number, y: number };

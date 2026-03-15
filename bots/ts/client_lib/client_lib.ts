import * as net from 'node:net';
import type {GameState, Action, Obj, Pos} from './types';
import {ActionType, UnitType, ObjType, ObjState} from './types';

let game: GameState | null = null;
let isGameInitialized: boolean = false;

// Internal module state replacing the class fields
let socket: net.Socket | null = null;
let actions: Action[] = [];
let debugData: any[] = [];
let buffer = '';
let host = process.env.SERVER_IP || '127.0.0.1';
let port = process.env.SERVER_PORT ? Number.parseInt(process.env.SERVER_PORT) : 4444;
let teamId: number = process.argv.length >= 3 ? Number(process.argv[2]) : 0;
let teamNameInternal = '';
let debug = false;
let tickCallback: (() => void) | null = null;

export function getGame(): GameState {
    if (!isGameInitialized || !game)
        throw new Error('Game is not initialized. Call initGame() before accessing game state.');
    return game;
}

function validateTeamId(): void {
    if (Number.isNaN(teamId)) {
        console.error('Invalid team id. Usage: ./bot <team_id>');
        process.exit(1);
    }
}

async function connectInternal(): Promise<void> {
    return new Promise((resolve, reject) => {
        socket = new net.Socket();

        socket.connect(port, host, () => {
            if (debug) console.log(`Connected to ${host}:${port}`);
            sendLogin();
        });

        socket.on('data', (data: Buffer) => {
            handleData(data);
            if (game && resolve) {
                resolve();
                (resolve as any) = null;
            }
        });

        socket.on('error', (err) => {
            console.error('Socket error:', err);
            reject(err);
        });

        socket.on('close', () => {
            if (debug) console.log('Connection closed');
            process.exit(0);
        });
    });
}

function sendLogin(): void {
    const loginMsg = {
        password: '42',
        id: teamId,
        name: teamNameInternal
    };
    sendJson(loginMsg);
}

function sendJson(obj: any): void {
    if (socket) {
        const str = JSON.stringify(obj) + '\n';
        socket.write(str);
        if (debug) console.log('Sent:', str);
    }
}

function handleData(data: Buffer): void {
    buffer += data.toString();
    let newlineIndex;
    while ((newlineIndex = buffer.indexOf('\n')) !== -1) {
        const line = buffer.slice(0, newlineIndex);
        buffer = buffer.slice(newlineIndex + 1);
        if (line.trim()) {
            handleLine(line);
        }
    }
}

function handleLine(line: string): void {
    try {
        const parsed = JSON.parse(line);
        if (debug) console.log('Received:', line);

        if (!game) {
            // Initial config
            game = {
                elapsed_ticks: 0,
                config: parsed,
                my_team_id: teamId,
                objects: []
            };
            isGameInitialized = true;
            if (debug) console.log('Config received');
            // Send first packet of actions right after receiving config to start game loop
            sendActions();
            return;
        }

        // Update game state from server message
        updateGameState(parsed);

        // Print errors from last tick if there were any
        printServerErrors(parsed?.errors);

        // Decrement cooldowns manually as C lib does
        decrementCooldowns();

        // After state update, we should have a tick
        if (tickCallback)
            try {
                tickCallback();
            } catch (e) {
                console.error('Error executing tick callback:', e);
            }

        // ALWAYS send actions back to the server to prevent timeouts
        sendActions();
    } catch (e) {
        console.error('Error parsing JSON:', e, 'Line:', line);
    }
}

function updateGameState(parsed: any): void {
    if (!parsed) return;
    const game = getGame();
    if (parsed.tick !== undefined) {
        game.elapsed_ticks = parsed.tick;
    }
    if (parsed.objects && Array.isArray(parsed.objects)) {
        for (const diff of parsed.objects) {
            applyDiff(diff);
        }
    }
}

function printServerErrors(errors: any[]): void {
    if (!errors || !Array.isArray(errors)) return;
    for (const error of errors) {
        console.error(`\x1b[31m${error}\x1b[0m`);
    }
}

function decrementCooldowns(): void {
    const game = getGame();
    for (const obj of game.objects) {
        if (obj.type === ObjType.UNIT) {
            if (obj.s_unit.action_cooldown > 0) obj.s_unit.action_cooldown--;
        } else if (obj.type === ObjType.CORE) {
            if (obj.s_core.spawn_cooldown > 0) obj.s_core.spawn_cooldown--;
        }
    }
}

function applyDiff(diff: any): void {
    const id = diff.id;
    if (id === undefined) return;

    const game = getGame();
    if (diff.state === 'dead') {
        game.objects = game.objects.filter(o => o.id !== id);
        return;
    }

    const obj = findOrInitializeObject(id);
    updateObjectProperties(obj, diff);
    updateTypeSpecificProperties(obj, diff);
}

function findOrInitializeObject(id: number): Obj {
    let obj = getGame().objects.find(o => o.id === id);
    if (!obj) {
        obj = {
            id,
            state: ObjState.ALIVE,
            pos: {x: 0, y: 0},
            s_core: {team_id: 0, gems: 0, spawn_cooldown: 0, balance: 0},
            s_unit: {unit_type: 0, team_id: 0, gems: 0, action_cooldown: 0, balance: 0},
            s_deposit_gems_pile: {gems: 0},
            s_bomb: {countdown: 0}
        } as Obj;
        getGame().objects.push(obj);
    }
    return obj;
}

function updateObjectProperties(obj: Obj, diff: any): void {
    if (diff.type !== undefined) obj.type = diff.type;
    if (diff.x !== undefined || diff.y !== undefined) {
        if (diff.x !== undefined) obj.pos.x = diff.x;
        if (diff.y !== undefined) obj.pos.y = diff.y;
    }
    if (diff.hp !== undefined) obj.hp = diff.hp;
}

function updateTypeSpecificProperties(obj: Obj, diff: any): void {
    if (diff.teamId !== undefined) {
        obj.s_core.team_id = diff.teamId;
        obj.s_unit.team_id = diff.teamId;
    }
    if (diff.gems !== undefined) {
        obj.s_core.gems = diff.gems;
        obj.s_core.balance = diff.gems;
        obj.s_unit.gems = diff.gems;
        obj.s_unit.balance = diff.gems;
        obj.s_deposit_gems_pile.gems = diff.gems;
    }
    if (diff.ActionCooldown !== undefined) obj.s_unit.action_cooldown = diff.ActionCooldown;
    if (diff.SpawnCooldown !== undefined) obj.s_core.spawn_cooldown = diff.SpawnCooldown;
    if (diff.unit_type !== undefined) obj.s_unit.unit_type = diff.unit_type;
    if (diff.countdown !== undefined) obj.s_bomb.countdown = diff.countdown;
}

function sendActions(): void {
    const packet = {
        actions: actions,
        debug_data: debugData
    };
    sendJson(packet);
    actions = [];
    debugData = [];
}

// Exported API: previously public methods of the class
export function coreCreateUnit(unitType: UnitType): void {
    actions.push({
        type: ActionType.CREATE,
        unit_type: unitType
    });
}

export function coreActionMove(unit: Obj, pos: Pos): void {
    actions.push({
        type: ActionType.MOVE,
        unit_id: unit.id,
        x: pos.x,
        y: pos.y
    });
}

export function coreActionAttack(attacker: Obj, target: Obj): void {
    actions.push({
        type: ActionType.ATTACK,
        unit_id: attacker.id,
        target_id: target.id
    });
}

export function coreActionTransferGems(source: Obj, targetPos: Pos, amount: number): void {
    actions.push({
        type: ActionType.TRANSFER,
        source_id: source.id,
        x: targetPos.x,
        y: targetPos.y,
        amount: amount
    });
}

export function coreGetObjFromPos(pos: Pos): Obj | null {
    return getGame().objects.find(o => o.pos.x === pos.x && o.pos.y === pos.y) || null;
}

export function coreInternalIsPosValid(pos: Pos): boolean {
    const game = getGame();
    return pos.x >= 0 && pos.y >= 0 && pos.x < game.config.gridSize && pos.y < game.config.gridSize;
}

function coreStaticIsFriendlyObj(o: Obj | null): boolean {
    if (!o) return false;
    const game = getGame();
    if (o.type === ObjType.UNIT) return o.s_unit.team_id === game.my_team_id;
    if (o.type === ObjType.CORE) return o.s_core.team_id === game.my_team_id;
    return false;
}

export function coreActionPathfind(unit: Obj, pos: Pos): void {
    if (!unit || unit.type !== ObjType.UNIT) return;
    if (unit.pos.x === pos.x && unit.pos.y === pos.y) return;
    if (unit.s_unit.action_cooldown !== 0) return;

    const posOptionY = (pos.y === unit.pos.y)
        ? unit.pos
        : {x: unit.pos.x, y: unit.pos.y + (pos.y > unit.pos.y ? 1 : -1)} as Pos;
    const posOptionX = (pos.x === unit.pos.x)
        ? unit.pos
        : {x: unit.pos.x + (pos.x > unit.pos.x ? 1 : -1), y: unit.pos.y} as Pos;

    let posOptionXPriority = 0;
    let posOptionYPriority = 0;

    // 1. + 2. check: out-of-bounds & one axis done
    if (!coreInternalIsPosValid(posOptionY) || posOptionY.y === unit.pos.y) posOptionYPriority += 500;
    if (!coreInternalIsPosValid(posOptionX) || posOptionX.x === unit.pos.x) posOptionXPriority += 500;

    // 2. check: prioritize larger axis
    if (Math.abs(unit.pos.x - pos.x) > Math.abs(unit.pos.y - pos.y))
        posOptionYPriority++;
    else
        posOptionXPriority++;

    // 3. check: pos emptiness
    const posOptionXObj = coreGetObjFromPos(posOptionX);
    const posOptionYObj = coreGetObjFromPos(posOptionY);

    if (posOptionXObj) posOptionXPriority += 50;
    if (posOptionYObj) posOptionYPriority += 50;

    // 4. check: obj friendliness check
    if (coreStaticIsFriendlyObj(posOptionXObj)) posOptionXPriority += 100;
    if (coreStaticIsFriendlyObj(posOptionYObj)) posOptionYPriority += 100;

    // -----

    if (posOptionXPriority < 250 && posOptionXPriority < posOptionYPriority) {
        if (posOptionXObj && !coreStaticIsFriendlyObj(posOptionXObj))
            coreActionAttack(unit, posOptionXObj);
        else if (!posOptionXObj)
            coreActionMove(unit, posOptionX);
        return;
    }
    if (posOptionYPriority < 250) {
        if (posOptionYObj && !coreStaticIsFriendlyObj(posOptionYObj))
            coreActionAttack(unit, posOptionYObj);
        else if (!posOptionYObj)
            coreActionMove(unit, posOptionY);
    }
}

export function coreActionBuild(builder: Obj, pos: Pos): void {
    actions.push({
        type: ActionType.BUILD,
        unit_id: builder.id,
        x: pos.x,
        y: pos.y
    });
}

export function coreDebugAddObjectInfo(obj: Obj, info: string): void {
    let entry = debugData.find(d => d.object_id === obj.id);
    if (!entry) {
        entry = {
            object_id: obj.id,
            object_info: '',
            object_path: []
        };
        debugData.push(entry);
    }
    entry.object_info += info;
}

export function coreDebugAddObjectPathStep(unit: Obj, pos: Pos): void {
    let entry = debugData.find(d => d.object_id === unit.id);
    if (!entry) {
        entry = {
            object_id: unit.id,
            object_info: '',
            object_path: []
        };
        debugData.push(entry);
    }
    entry.object_path.push({x: pos.x, y: pos.y});
}

// New entry point replacing the class: set callback and connect
export async function startGame(teamName: string, onTick: () => void): Promise<void> {
    teamNameInternal = teamName;
    tickCallback = onTick;

    // Re-evaluate env/argv on each start, in case caller changed them
    host = process.env.SERVER_IP || '127.0.0.1';
    port = process.env.SERVER_PORT ? Number.parseInt(process.env.SERVER_PORT) : 4444;
    teamId = process.argv.length >= 3 ? Number(process.argv[2]) : 0;
    validateTeamId();

    // Enable verbose logs via env var if desired
    debug = process.env.CLIENT_LIB_DEBUG === '1' || process.env.CLIENT_LIB_DEBUG === 'true';

    await connectInternal();
}
